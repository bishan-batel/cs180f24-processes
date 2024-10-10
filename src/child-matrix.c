#include <errno.h>
#include <stdio.h>   /* fopen, fscanf, fclose */
#include <stdlib.h>  /* exit, malloc          */
#include <sys/shm.h>


int main(int argc, char **argv)
{
   return child_matrix(argc, argv);
}