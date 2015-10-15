#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "pvm3.h"

#define GRPNAME "gaussp"

/**
 * Reads the N*N matrix stored in the specified file.
 * Stores N / nTasks lines in the (partial) matrix parameter for T0.
 * Sends N / nTasks lines to each task. Each task can then receive and store the
 * lines in its partial matrix.
 * @Note This function must be executed by the task having the id 0 (T0) in the 
 * previously created pvm group.
 */
void matrix_load(char filename[], double *matrix, int N, int nTasks) {	
	FILE *f = fopen(filename, "r");
    if (f == NULL) { perror("matrix_load : fopen "); } 
    
    double *lineBuffer = malloc(N * sizeof(double));
    
    int counter = 0; // number of lines saved in the matrix for the task 0
    for(size_t i = 0 ; i < N ; i++) {
        for(size_t j = 0 ; j < N ; j++) {
            fscanf(f, "%lf", &lineBuffer[j]);
        }
		
		int taskGrpId = i % nTasks;
        if (taskGrpId == 0) {
            memcpy(&matrix[counter * N], lineBuffer, N * sizeof(double));
            counter++;
        }
        else {
            int dest = pvm_gettid(GRPNAME, taskGrpId);
            pvm_initsend(PvmDataDefault);
            pvm_pkdouble(lineBuffer, N, 1);
            pvm_send(dest, 0);
        }
    }
    
    free(lineBuffer);
    fclose(f);
}

/**
 * Receives a partial matrix from T0 and stores it in the matrix parameter.
 */
void partial_matrix_recv(double* matrix, int N, int nTasks) {
    int src = pvm_gettid(GRPNAME, 0);
    for (size_t i = 0 ; i < N / nTasks ; i++) {
        pvm_recv(src, -1);
        pvm_upkdouble(&matrix[i*N], N, 1);
    }
}

/**
 * Sends a partial matrix to T0.
 */
void partial_matrix_send(double* matrix, int N, int myGrpId, int nTasks) {
	int dest = pvm_gettid(GRPNAME, 0);
	for (size_t i = 0 ; i < N / nTasks ; i++) {
        pvm_initsend(PvmDataDefault);
        pvm_pkdouble(&matrix[i*N], N, 1);
        pvm_send(dest, 0);
	}
}

/**
 * Saves a complete matrix in the specified file.
 * The matrix parameter is T0's partial matrix. This function will retrieve
 * the whole matrix by receiving missing lines from the other tasks.
 * @note This function must be executed by T0.
 */
void matrix_save(
	char filename[], double *matrix, int N, int myGrpId, int nTasks
) {    
    FILE *f = fopen(filename, "w");
    if(f == NULL) { perror("matrix_save : fopen "); }
    
	double *lineBuffer = malloc(N * sizeof(double)); 
    
    int counter = 0; // number of lines saved in the file for the task 0
    for(size_t i = 0 ; i < N ; i++) {
        if (i % nTasks == 0) { // T0 saves its line
            for(size_t j = 0 ; j < N ; j++) {
            	fprintf(f, "%8.2f ", matrix[counter*N + j]);
            }
            fprintf(f, "\n");
            counter++;
        }
        else { // T0 receives a line from another task
            int src = pvm_gettid(GRPNAME, i % nTasks);
            pvm_recv(src, -1);
            pvm_upkdouble(lineBuffer, N, 1);
            for(size_t j = 0 ; j < N ; j++) {
                fprintf(f, "%8.2f ", lineBuffer[j]);
            }
            fprintf(f, "\n");
        }
    }
    
    free(lineBuffer);
    fclose(f);
}

void matrix_display(double *matrix, int myGrpId, int N, int nTasks) {
    printf("myGrpId: %d\n", myGrpId);
    for (size_t i = 0 ; i < N/nTasks ; i++) {
	    for (size_t j = 0 ; j < N ; j++) {
        	printf("%f ", matrix[i*N + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void gauss(double* matrix, int N) {
    double pivot;

    for(size_t k = 0 ; k < N-1 ; k++) { // mise a 0 de la col. k
        // printf(". ");
        if (fabs(*(matrix+k+k*N)) <= 1.0e-11) {
            printf("ATTENTION: pivot %zu presque nul: %g\n", k, *(matrix+k+k*N));
            exit(EXIT_FAILURE);
        }
        for(size_t i = k+1 ; i < N ; i++) { // update lines(k+1) to(n-1)
            pivot = - *(matrix+k+i*N) / *(matrix+k+k*N);
            for(size_t j = k ; j < N ; j++) { // update elts(k) -(N-1) of line i
                *(matrix+j+i*N) = *(matrix+j+i*N) + pivot * *(matrix+j+k*N);
            }
            // *(matrix+k+i*N) = 0.0;
        }
    }
    printf("\n");
}

void dowork(char filename[], int myGrpId, int N, int nTasks) {
    double *partialMatrix = malloc((N/nTasks) * N * sizeof(double));
    if (partialMatrix == NULL) { 
	    printf("Malloc failed\n");
	    exit(EXIT_FAILURE);
    }
    
    if (myGrpId == 0) {
        matrix_load(filename, partialMatrix, N, nTasks);
    }
    else {
        partial_matrix_recv(partialMatrix, N, nTasks);
    }
    
    matrix_display(partialMatrix, myGrpId, N, nTasks);
    
    // TODO compute gauss
	
	if (myGrpId == 0) {
	    sprintf(filename+strlen(filename), ".result");
    	matrix_save(filename, partialMatrix, N, myGrpId, nTasks);
    }
    else {
    	partial_matrix_send(partialMatrix, N, myGrpId, nTasks);
    }
    
    free(partialMatrix);
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        printf("Usage: %s <matrix size> <matrix name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]); // matrix size
    
    char filename[255];
    strcpy(filename, argv[2]);
    
    // enroll in pvm
    pvm_mytid();

    // determine the number of tasks
    int *tids;
    int nTasks = pvm_siblings(&tids);
	
	// join group & get current task's index in the group
    int myGrpId = pvm_joingroup(GRPNAME);
    pvm_barrier(GRPNAME, nTasks);
    pvm_freezegroup(GRPNAME, nTasks);

    dowork(filename, myGrpId, N, nTasks);

    pvm_lvgroup(GRPNAME);
    pvm_exit();
    
    return EXIT_SUCCESS;
}
