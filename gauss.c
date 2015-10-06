
#include <stdlib.h>
#include <stdio.h>
	 
#include <sys/time.h>
#include <time.h>

#include <errno.h>

void matrix_load ( char nom[], double *tab, int N ) {
  FILE *f;
  int i,j;

  if ((f = fopen (nom, "r")) == NULL) { perror ("matrix_load : fopen "); } 
  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      fscanf (f, "%lf", (tab+i*N+j) );
    }
  }
  fclose (f);
}

void matrix_save ( char nom[], double *tab, int N ) {
  FILE *f;
  int i,j;

  if ((f = fopen (nom, "w")) == NULL) { perror ("matrix_save : fopen "); } 
  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      fprintf (f, "%8.2f ", *(tab+i*N+j) );
    }
    fprintf (f, "\n");
  }
  fclose (f);
}

void matrix_display ( double *tab,int  N ) {
  int i,j;

  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      printf ("%8.2f ", *(tab+i*N+j) );
    }
    printf ("\n");
  }

}

void gauss ( double * tab, int N ) {
  int i,j,k;
  double pivot;

  for ( k=0; k<N-1; k++ ){ /* mise a 0 de la col. k */
    /* printf (". "); */
    if ( fabs(*(tab+k+k*N)) <= 1.0e-11 ) {
      printf ("ATTENTION: pivot %d presque nul: %g\n", k, *(tab+k+k*N) );
      exit (-1);
    }
    for ( i=k+1; i<N; i++ ){ /* update lines (k+1) to (n-1) */
      pivot = - *(tab+k+i*N) / *(tab+k+k*N);
      for ( j=k; j<N; j++ ){ /* update elts (k) - (N-1) of line i */
	*(tab+j+i*N) = *(tab+j+i*N) + pivot * *(tab+j+k*N);
      }
      /* *(tab+k+i*N) = 0.0; */
    }
  }
  printf ("\n");
}

main (int argc, char **argv) {
  int N, i, j, k;
  double *tab, pivot;
  char nom[255];
  FILE *f;
  struct timeval tv1, tv2;	/* for timing */
  int duree;
  
  if (argc != 3){
    printf ("Usage: %s <matrix size> <matrix name>\n", argv[0]);
    exit (-1);
  } 
  N = atoi ( argv[1] );
  strcpy(nom, argv[2]);
  if ( (tab=malloc(N*N*sizeof(double))) == NULL ) {
    printf ("Cant malloc %d bytes\n", N*N*sizeof(double));
    exit (-1);
  }
  gettimeofday( &tv1, (struct timezone*)0 );
      matrix_load ( nom , tab, N );
  gettimeofday( &tv2, (struct timezone*)0 );
  duree = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
  printf ("loading time: %10.8f sec.\n", duree/1000000.0 );

  gettimeofday( &tv1, (struct timezone*)0 );
      gauss ( tab, N );
  gettimeofday( &tv2, (struct timezone*)0 );
  duree = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
  printf ("computation time: %10.8f sec.\n", duree/1000000.0 );

  sprintf ( nom+strlen(nom), ".result" );
  matrix_save ( nom, tab, N );
}
