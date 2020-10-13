/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 9 - Task 3 
 * mpicc task3.c -o task3_out 
 * mpirun -np 4 task3_out
 */

/* Gets the neighbors in a cartesian communicator
* Orginally written by Mary Thomas
* - Updated Mar, 2015
* Link: https://edoras.sdsu.edu/~mthomas/sp17.605/lectures/MPICart-Comms-and-Topos.pdf
* Minor modifications to fix bugs and to revise print output
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
int master_io(MPI_Comm master_comm, MPI_Comm comm);
int slave_io(MPI_Comm master_comm, MPI_Comm comm);
int main(int argc, char **argv)
{
 int rank;
 MPI_Comm new_comm;
  MPI_Init(&argc, &argv);
 MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 MPI_Comm_split( MPI_COMM_WORLD,rank == 0, 0, &new_comm);
 if (rank == 0)
master_io( MPI_COMM_WORLD, new_comm );
 else
slave_io( MPI_COMM_WORLD, new_comm );
 MPI_Finalize();
 return 0;
}
/* This is the master */
int master_io(MPI_Comm master_comm, MPI_Comm comm)
{
 int i,j, size;
 char buf[256];
 MPI_Status status;
 MPI_Comm_size( master_comm, &size );
 for (j=1; j<=2; j++) {
for (i=1; i<size; i++) {
 MPI_Recv( buf, 256, MPI_CHAR, i, 0, master_comm, &status
);
 fputs( buf, stdout );
}
 }
 return 0;
}
/* This is the slave */
int slave_io(MPI_Comm master_comm, MPI_Comm comm)
{
 char buf[256];
 int rank;

 MPI_Comm_rank(comm, &rank);
 sprintf(buf, "Hello from slave %d\n", rank);
 MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, 0, master_comm );

 sprintf( buf, "Goodbye from slave %d\n", rank );
 MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, 0, master_comm );
 return 0;
}