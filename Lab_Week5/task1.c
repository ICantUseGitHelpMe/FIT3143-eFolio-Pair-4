#include <stdio.h>
#include <mpi.h>
int main( argc, argv )
int argc;
char **argv;

{
    int rank, size;  // Initialise the variables for use
    MPI_Init( &argc, &argv );   // Initialise MPI
    MPI_Comm_size( MPI_COMM_WORLD, &size ); // Get the communicator size and rank for the current process
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    printf( "Hello world from process %d of %d\n", rank, size ); // Print the information on this thread
    MPI_Finalize();  // Stop MPI sending messages.
    return 0;
}

/* Run with:
mpicc task1.c -o task1_out
mpirun -np 4 ./task1_out 
*/
