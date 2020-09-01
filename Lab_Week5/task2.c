#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
int main(int argc, char* argv[])
{
    int my_rank;
    int p;
    MPI_Init(&argc, &argv);  // Initialise MPI and get the rank/size values
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    int val = -1;
    do{
        if(my_rank == 0){
            printf("Enter a round number (> 0 ): ");  // Ask for the number from the user
            fflush(stdout);
            scanf("%d", &val);  
        }
        MPI_Bcast(&val, 1, MPI_INT, 0, MPI_COMM_WORLD); // This function has to be visible to all processes.
        printf("Processors: %d. Received Value: %d\n", my_rank, val);
        fflush(stdout);
    } while(val > 0);
    MPI_Finalize();  // Finish up with MPI and stop parallelism
    return 0;
}