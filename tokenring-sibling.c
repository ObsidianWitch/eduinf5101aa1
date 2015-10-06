
/*  tokenring example using PVM 3.4 
    - uses sibling() to determine the nb of spawned tasks (xpvm and pvm> ok)
    - uses group for token ring communication
*/

#include <stdio.h>
#include <sys/types.h>
#include "pvm3.h"

#define GRPNAME "tokenring"

main(int argc, char ** argv) {
    int NPROC = 8;		/* default nb of proc */
    int mytid;                  /* my task id */
    int *tids;                  /* array of task id */
    int me;                     /* my process number */
    int i;

    /* enroll in pvm */
    mytid = pvm_mytid();

    /* determine the size of my sibling list */
    NPROC = pvm_siblings(&tids); 
    /* WARNING: tids are in order of spawning, which is different from
       the task index JOINING the group */

    me = pvm_joingroup( GRPNAME ); /* me: task index in the group */
    pvm_barrier( GRPNAME, NPROC );
    pvm_freezegroup ( GRPNAME, NPROC );
    for ( i = 0; i < NPROC; i++) tids[i] = pvm_gettid ( GRPNAME, i); 

/*--------------------------------------------------------------------------*/
/*           all the tasks are equivalent at that point                     */

     dowork( me, tids, NPROC );

     pvm_lvgroup( GRPNAME );
     pvm_exit();
}

/* Simple example passes a token around a ring */

dowork( int me, int tids[], int nproc ) {
     int token;
     int src, dest;
     int count  = 1;
     int stride = 1;
     int msgtag = 4;

     /* Determine neighbors in the ring */
     src = pvm_gettid (GRPNAME, (me - 1 + nproc) % nproc );
     dest= pvm_gettid (GRPNAME, (me + 1) % nproc );
     if( me == 0 ) { 
        token = dest;
        pvm_initsend( PvmDataDefault );
        pvm_pkint( &token, count, stride );
        pvm_send( dest, msgtag );
        pvm_recv( src, msgtag );
     }
     else {
        pvm_recv( src, msgtag );
        pvm_upkint( &token, count, stride );
        pvm_initsend( PvmDataDefault );
        pvm_pkint( &token, count, stride );
        pvm_send( dest, msgtag );
     }
     printf("P%d (%x) token ring done\n", me, pvm_mytid());
}
