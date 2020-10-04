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
 * mpicc main.c -o main_out -lm
 * mpirun -np 4 main_out
 */

#include <stdio.h>
#include <mpi.h>
#include <stdbool.h>
#include <math.h>
int set_memory(int index, int value);
int get_memory(int index, int rank, int size);
void finish(int rank);

int main( argc, argv )
int argc;
char **argv;
{
    int rank, size;
    int share_buff[100];  // The array for our "Distributed Shared Memory"

    int goal = 10;  // Target we are summing to

    // shared memory stuff here

    MPI_Init( &argc, &argv );  // Begin MPI
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );  // This processor's rank
    MPI_Comm_size( MPI_COMM_WORLD, &size );  // The number of processors

    if (rank == 0){
        share_buff[0] = 0;
        bool exit[size];
        for (int j; j < size; j++){
            exit[j] = false;
        }
        bool processing = true;
        while (processing)
        {   
            int buf[3];  // buf[0] has the code, index, and reply rank
            MPI_Recv(buf, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (buf[0] == 20){  // Done
                // Update the relevant process to complete
                exit[buf[2]] = true;
                // printf("Proc %d FInished!\n", buf[2]);
            }
            else if (buf[0] == 10){ // Information
                // printf("Value now: %d, sent val %d\n", share_buff[0],  share_buff[buf[1]]);
                MPI_Send(&share_buff[buf[1]], 1, MPI_INT, buf[2], 0, MPI_COMM_WORLD);    // MPI_Send to the main thread
                
                int val[2];  // val[0] is the index, val[1] is the value
                MPI_Recv(val, 2, MPI_INT, buf[2], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                share_buff[val[0]] = val[1];
                // printf("Value now: %d, RECV val %d\n", share_buff[0], val[1]);
                
            }

            // Check if we are ready to end by checking all processors
            bool stop = true;
            for (int check = 1; check < size; check ++){
                stop = stop && exit[check]; // Only returns true if everything is false
            }

            processing = !stop;
            
        }

        printf("Final result: %d\n", share_buff[0]);
        
    }   
    else{ 

        // Calculate the range for each processor
        float relative_rank = rank - 1; // Account for rank 0 being the memory controller
        float next_rel_rank = relative_rank + 1;
        int relative_size = size - 1;
        int cumulate = 0;

        for (int i =  floor(relative_rank / relative_size * goal) + 1; i < ceil(next_rel_rank  / relative_size * goal); i++){
            int current = get_memory(0, rank, size);
            int new_val = current + i;
            set_memory(0, new_val);  // Send the memory
        }

        if (rank == size - 1){  // The final processor can add the last number which would otherwise be an outlier
            int current = get_memory(0, rank, size);
            int new_val = current + goal;
            set_memory(0, new_val);  // Send the memory
        }

        finish(rank);  // Send the finsihed signal
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
    int buf[2];
    buf[0] = index;
    buf[1] = value;
    MPI_Send(buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);    // MPI_Send to the main thread
    return 1;
}

void finish(int rank){
    int buf[3];
    buf[0] = 20;  // End code
    buf[1] = -1; // Irrelevant
    buf[2] = rank;  // The rank to end
    MPI_Send(buf, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);    // MPI_Send to the main thread
    return;
}