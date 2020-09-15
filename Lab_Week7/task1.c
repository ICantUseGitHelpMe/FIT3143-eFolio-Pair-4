/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 7 - Task 1
 *      This task implements a simple parallel data 
 * structure. This structure is a two-dimension regular 
 * mesh of points, divided into slabs, with each slab
 * allocated to a different MPI process. In the simplest 
 * C form, the full data structure is:
 *          double x[maxn][maxn];
 * and we want to arrange it so that each process has a 
 * local piece:
 *          double xlocal[maxn/size][maxn];
 * where size is the size of the communicator (e.g., the 
 * number of MPI processes).
 *      If that was all that there was to it, there wouldn't 
 * be anything to do.
 *      However, for the computation that we're going to 
 * perform on this data structure, we'll need the adjacent 
 * values. That is, to compute a new x[i][j], we will need
 * x[i][j+1] x[i][j-1] x[i+1][j] x[i-1][j]. The last two of 
 * these could be a problem if they are not in xlocal but
 * are instead on the adjacent processes. To handle this 
 * difficulty, we define ghost points that we will contain the 
 * values of these adjacent points.
 *      Write a parallel code using MPI to copy divide the array 
 * x into equalsized strips and to copy the adjacent edges to the 
 * neighboring processes. Assume that x is maxn by maxn, and that 
 * maxn is evenly divided by the number of processes. For 
 * simplicity, you may assume a fixed size array and a fixed (or 
 * minimum) number of processors.
 *      To test the routine, have each process fill its section 
 * with the rank of the process, and the ghost points with -1. After 
 * the exchange takes place, test to make sure that the ghost points 
 * have the proper value. Assume that the domain is not periodic; 
 * that is, the top process (rank = size - 1) only sends and receives 
 * data from the one under it (rank = size - 2)and the bottom process 
 * (rank = 0) only sends and receives data from the one above it (rank 
 * = 1). Consider a maxn of 12 and use 4 processors to start with.
 *      In this exercise (i.e., Task 1), use non-blocking MPI 
 * routines instead of the blocking routines you have learned 
 * before. Use MPI Isend and MPI Irecv and use MPI Wait or MPI 
 * Waitall to test for completion of the nonblocking operations.
 *      You may want to use these MPI routines in your solution: 
 * MPI Isend, MPI Irecv, MPI Waitall.
 *      
 *      NOTE THAT the example in (https://www.mcs.anl.gov/research/
 *      projects/mpi/tutorial/mpiexmpl/src/exchange/C/main.html) was
 *      referred when replacing MPI_Send, MPI_Recv with MPI_Isend,
 *      MPI_Irecv, MPI_Wait
 *
 * mpicc task1.c -o task1_out
 * mpirun -np 4 task1_out
 */

#include <stdio.h>
#include <mpi.h>

/* This example handles a 12 x 12 mesh, on 4 processors only. */
#define maxn 12

int main( argc, argv )
int argc;
char **argv;
{
        
    int rank, value, size, errcnt, toterr, i, j, up_nbr, down_nbr;
    double x[12][12];
    double xlocal[(12/4)+2][12];

    MPI_Request request;
    // MPI_Status status;
    MPI_Status status;
    MPI_Init( &argc, &argv );  // Begin MPI
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    if (size != 4) MPI_Abort( MPI_COMM_WORLD, 1 );
    /* xlocal[][0] is lower ghostpoints, xlocal[][maxn+2] is upper */
    /* Fill the data as specified */
    for (i=1; i<=maxn/size; i++) 
	for (j=0; j<maxn; j++) 
	    xlocal[i][j] = rank;
    for (j=0; j<maxn; j++) {
	xlocal[0][j] = -1;
	xlocal[maxn/size+1][j] = -1;
    }

   
    up_nbr = rank + 1;  // This prevents crashing from occurring if the current process is 0 or size - 1 (max)
    down_nbr = rank - 1;
    if (up_nbr >= size){
        up_nbr = MPI_PROC_NULL;
    }
    if (down_nbr < 0){
        down_nbr = MPI_PROC_NULL;
    }

    // Send above / Send below.  Be sure to wait afterwards before continuing.
    MPI_Isend( xlocal[maxn/size], maxn, MPI_DOUBLE, up_nbr, 0, MPI_COMM_WORLD, &request );
    MPI_Wait(&request, &status);

    MPI_Isend( xlocal[1], maxn, MPI_DOUBLE, down_nbr, 1, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

    // receive above / receive below.  Be sure to wait afterwards before continuing.
    MPI_Irecv( xlocal[maxn/size+1], maxn, MPI_DOUBLE, up_nbr, 1, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

    MPI_Irecv( xlocal[0], maxn, MPI_DOUBLE, down_nbr, 0, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

     

    errcnt = 0;
    for (i=1; i<=maxn/size; i++) 
	for (j=0; j<maxn; j++){
	    if (xlocal[i][j] != rank){
            errcnt++;
        }
    }
    for (j=0; j<maxn; j++) {
	    if (xlocal[0][j] != rank - 1){
            errcnt++;
        }
        if (rank < size-1 && xlocal[maxn/size+1][j] != rank + 1){
            errcnt++;
        };
    }

    MPI_Reduce( &errcnt, &toterr, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );  // Normal code reducing the problem (see tut doc)
    if (rank == 0) {
        if (toterr){
            printf( "! found %d errors\n", toterr );
        }
        else{
            printf( "No errors\n" );
        }
    }

    MPI_Finalize();
    return 0;
} 