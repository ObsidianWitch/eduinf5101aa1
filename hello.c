
/*
  Sample task which prints its TID... 
  try to spawn several of them to see the result
*/

#include <stdio.h>
#include "pvm3.h"
#include <unistd.h>

main()
{
  char hostname [256];
  
  gethostname ( hostname, 255 );
  printf("Hello world, I am task t%X on machine %s\n", pvm_mytid(), hostname);
  pvm_exit();
  exit(0);
}


