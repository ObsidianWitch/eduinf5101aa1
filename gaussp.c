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

void matrix_load(char nom[], double *tab, int N, int proc, int nproc) {
    FILE *f;
    int i,j;
    int counter = 0;

    if((f = fopen(nom, "r")) == NULL) { perror("matrix_load : fopen "); } 
    for(i=0; i<N; i++) {
        double *tmp = malloc(N * sizeof(double));
        for(j=0; j<N; j++) {
            fscanf(f, "%lf",(tmp + j));
        }

        if (i % nproc == 0) {
            memcpy(tab+counter*N, tmp, N * sizeof(double));
            counter++;
            free(tmp);
        }
        else {
            int dest= pvm_gettid(GRPNAME, i % nproc);
            pvm_initsend(PvmDataDefault);
            pvm_pkdouble(tmp, N, 1);
            pvm_send(dest, i%nproc);
            free(tmp);
        }
    }
    fclose(f);
}

void matrix_recv(double* tab, int N, int nproc) {
    int i;
    for (i = 0; i < N/nproc; i++){
        double *tmp = malloc(N * sizeof(double));
        int src = pvm_gettid (GRPNAME, 0 );
        pvm_recv(src, -1);
        pvm_upkdouble(tmp, 1, 1);
        memcpy(tab+i*N, tmp, N*sizeof(tmp));
        free(tmp);
    }
}

void matrix_save(char nom[], double *tab, int N, int proc, int nproc) {
    FILE *f;
    int i,j;

    if (proc == 0) {
        if((f = fopen(nom, "w")) == NULL) { perror("matrix_save : fopen "); } 
        for(i=0; i<N; i++) {
            if (i%nproc == 0) { // P0 save its line
                for(j=0; j<N; j++) {
                    fprintf(f, "%8.2f ", *(tab+i*N+j));
                }
                fprintf(f, "\n");
            }
            else { // P0 receives line from other procs
                double *tmp = malloc(N * sizeof(double));
                int src = pvm_gettid (GRPNAME, 0 );
                pvm_recv(src, -1);
                pvm_upkdouble(tmp, 1, 1);
                for(j=0; j<N; j++) {
                    fprintf(f, "%8.2f ", *(tmp+j));
                }
                fprintf(f, "\n");
                free(tmp);
            }
        }
        fclose(f);
    }
    else { // Send line to P0
        for ( i = 0; i < N/nproc; i++){
            int dest= pvm_gettid(GRPNAME, i % nproc);
            pvm_initsend(PvmDataDefault);
            pvm_pkdouble(tab+i*N, N, 1);
            pvm_send(dest, i);
        }
    }
}

void matrix_display(double *tab,int  N) {
    int i,j;

    for(i=0; i<N; i++) {
        for(j=0; j<N; j++) {
            printf("%8.2f ", *(tab+i*N+j));
        }
        printf("\n");
    }

}

void gauss(double * tab, int N) {
    int i,j,k;
    double pivot;

    for(k=0; k<N-1; k++){ /* mise a 0 de la col. k */
        /* printf(". "); */
        if(fabs(*(tab+k+k*N)) <= 1.0e-11) {
            printf("ATTENTION: pivot %d presque nul: %g\n", k, *(tab+k+k*N));
            exit(-1);
        }
        for(i=k+1; i<N; i++){ /* update lines(k+1) to(n-1) */
            pivot = - *(tab+k+i*N) / *(tab+k+k*N);
            for(j=k; j<N; j++){ /* update elts(k) -(N-1) of line i */
                *(tab+j+i*N) = *(tab+j+i*N) + pivot * *(tab+j+k*N);
            }
            /* *(tab+k+i*N) = 0.0; */
        }
    }
    printf("\n");
}

int main(int argc, char ** argv) {
    if(argc != 3){
        printf("Usage: %s <matrix size> <matrix name>\n", argv[0]);
        exit(-1);
    }

    int NPROC = 4; /* default nb of proc */
    int mytid;     /* my task id */
    int *tids;     /* array of task id */
    int me;        /* my process number */
    int i;
    char nom[255];
    int N = atoi(argv[1]); // matrix size
    strcpy(nom, argv[2]);
    

    /* enroll in pvm */
    mytid = pvm_mytid();

    /* determine the size of my sibling list */
    NPROC = pvm_siblings(&tids); 
    /* WARNING: tids are in order of spawning, which is different from
       the task index JOINING the group */

    me = pvm_joingroup(GRPNAME); /* me: task index in the group */
    pvm_barrier(GRPNAME, NPROC);
    pvm_freezegroup(GRPNAME, NPROC);
    for(i = 0; i < NPROC; i++) {
        tids[i] = pvm_gettid(GRPNAME, i);
    }

    // All the tasks are equivalent at that point
    dowork(nom, me, tids, N, NPROC);
    

    pvm_lvgroup(GRPNAME);
    pvm_exit();
    
    return EXIT_SUCCESS;
}

/* Simple example passes a token around a ring */

dowork(char nom[], int me, int tids[], int N, int nproc) {
    int i, j, k;
    double *tab, pivot;
    FILE *f;
    struct timeval tv1, tv2; // for timing
    int duree;
    if((tab=malloc((N*nproc)*sizeof(double))) == NULL) {
        printf("Cant malloc %d bytes\n", (N*nproc)*sizeof(double));
        exit(-1);
    }
    gettimeofday(&tv1,(struct timezone*)0);
    if(me == 0){
        matrix_load(nom , tab, N, me, nproc);
    }
    else {
        matrix_recv(tab, N, nproc);
    }

    sprintf(nom+strlen(nom), ".result");
    matrix_save(nom, tab, N, me, nproc);

    /*
    gettimeofday(&tv2,(struct timezone*)0);
    duree =(tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
    printf("loading time: %10.8f sec.\n", duree/1000000.0);
    gettimeofday(&tv1,(struct timezone*)0);

    gauss(tab, N);

    gettimeofday(&tv2,(struct timezone*)0);
    duree =(tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
    printf("computation time: %10.8f sec.\n", duree/1000000.0);
    //*/
}
