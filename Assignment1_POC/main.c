/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Assignment 1 - POC
 *      This POC is for the shared memory message-passing
 * system proposed in Sharing Memory Robustly in 
 * Message-Pasing Systems, Attiya et al. The system works 
 * in the following fashion:
 *      - message passing occurs using shared memory, to 
 *        simulate this, a buffer array will be created
 *      - all process that will access shared memory will
 *        need to access the buffer to receive messages 
 *        before sending a new message
 *      - messages being sent to other processes will need
 *        write to the shared buffer array
 *      - all processes will poll from the shared array 
 *        when reading / writing
 *
 * mpicc main.c -o main_out
 * mpirun -np 4 main_out
 */

#include <stdio.h>
#include <mpi.h>

int main( argc, argv )
int argc;
char **argv;
{
    int rank, size;
    char share_buff[100];  // The array for our "Distributed Shared Memory"
    int goal = 100;  // Target we are summing to

    // shared memory stuff here

    MPI_Init( &argc, &argv );  // Begin MPI
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );  // This processor's rank
    MPI_Comm_size( MPI_COMM_WORLD, &size );  // The number of processors
    
    for (int i = ((float) rank / size) * goal; i < ((float) (rank + 1) )/ size * goal; i++){
        printf("Processor: %d, i: %d\n", rank, i);
    }

    // individual process stuff here

    MPI_Finalize( );
    return 0;
}
