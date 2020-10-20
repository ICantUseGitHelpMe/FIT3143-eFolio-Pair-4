/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 10 - Task 4 
 * mpicc task4.c -o task4_out -lm
 * mpirun -np 4 task4_out
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
int main()
{
FILE *pInfile;
float a_coeff, b_coeff, c_coeff, x4, x2, disc;
float x4r, x4i, x2r, x2i;
int fileElementCount = 0, constNeg = -4;;
int my_rank;
int p;
MPI_Status status;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
MPI_Comm_size(MPI_COMM_WORLD, &p);
// WRITE PART(a) HERE
switch (my_rank){
case 0:{
// CONTINUE WITH PART (a) HERE
break;
}
case 4:{
// WRITE PART (b) HERE
break;
}
case 2:{
// WRITE PART (c) HERE
break;
}
}
MPI_Finalize();
return 0;
}
