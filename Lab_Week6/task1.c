/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 6 - Task 1
 *      Write an MPI program that takes data from 
 * process zero and sends it to all other processes
 * by sending it in a ring. That is, process i should
 * receive the data and send it to process i+1, until 
 * the last process is reached. The last process then 
 * sends the data back to process zero.
 *      Each MPI process prints out the received data. 
 * Use a loop to repeat the cycle until a sentinel value
 * is specified to exit the program. 
 *
 * mpicc task1.c -o task1_out -lm
 * mpirun -np 4 task1_out
 */
#include <stdio.h>
#include <mpi.h>
int main(int argc, char **argv){
    int rank, s_value, r_value, size;
    MPI_Status status;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    do {
        if (rank == 0) {
            printf("Enter a round number: ");
            fflush(stdout);
            scanf( "%d", &s_value );
            // Add your code here
            MPI_Send(&s_value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);                        // MPI_Send to one rank above current rank
            MPI_Recv(&r_value, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);     // MPI_Recv from the end
            printf( "Process %d got %d from Process %d\n", rank, r_value, size - 1);
            fflush(stdout);
            // printf( "Process %d got %d from Process %d\n", rank, r_value, size - 1);
            // fflush(stdout);

        }
        else {
            // Add your code here
            MPI_Recv(&r_value, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);     // MPI_Recv from previous rank

            MPI_Send(&r_value, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);                 // MPI_Send to next thread, and if rank + 1 meets 
                                                                                                // the last processor, revert to 0.

            printf( "Process %d got %d from Process %d\n",
            rank, r_value, rank - 1);
            fflush(stdout);
        }
    } while (r_value >= 0);
    MPI_Finalize( );
    return 0;
}
