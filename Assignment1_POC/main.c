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
#include <stdbool.h>
int set_memory(int index, int value);
int get_memory(int index, int rank, int size);

int main( argc, argv )
int argc;
char **argv;
{
    int rank, size;
    int share_buff[100];  // The array for our "Distributed Shared Memory"

    int goal = 100;  // Target we are summing to

    // shared memory stuff here

    MPI_Init( &argc, &argv );  // Begin MPI
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );  // This processor's rank
    MPI_Comm_size( MPI_COMM_WORLD, &size );  // The number of processors

    if (rank == 0){
        share_buff[0] = 27;
        share_buff[1] = 1;
        bool exit[size];
        for (int j; j < size; j++){
            exit[j] = false;
        }
        while (true)
        {   
            int buf[3];  // buf[0] has the code, index, and reply rank
            MPI_Recv(buf, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (buf[0] == 20){  // Done
                // Update the relevant process to complete
                exit[buf[2]] = true;
            }
            if (buf[0] == 10){ // Information
                MPI_Send(&share_buff[buf[1]], 1, MPI_INT, buf[2], 0, MPI_COMM_WORLD);    // MPI_Send to the main thread

            }
            
        }
        
    }   
    else{ 
        for (int i = ((float) rank / size) * goal; i < ((float) (rank + 1) )/ size * goal; i++){
            // int current = share_buff[0];
            int current = get_memory(0, rank, size);
            printf("Process: %d,Val: %d\n", rank, current);
            // int new_val = current + i;
            // share_buff[0] = new_val;
            // set_memory(0, new_val);  // Send the memory
        }
    }
    // individual process stuff here

    // printf("Rank %d, Values: %d\n", rank, share_buff[0]);

    MPI_Finalize( );
    return 0;
}

int get_memory(int index, int rank, int size){
    int buff[3];
    buff[0] = 10;
    buff[1] = index;
    buff[2] = rank;
    MPI_Send(buff, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);    // MPI_Send to the main thread
    int result;
    MPI_Recv(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


    return result;
}

int set_memory(int index, int value){

    return 0;
}