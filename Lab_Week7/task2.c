/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 7 - Task 2
 *      Your task is to replace the MPI Send and MPI 
 * Recv calls in your solution for Question 1 with two 
 * calls to MPI Sendrecv. The first call should shift 
 * data up; that is, it should send data to the processor 
 * above and receive data from the processor below. The 
 * second call to MPI Sendrecv should reverse this; it
 * should send data to the processor below and receive 
 * from the processor above.
 *
 * mpicc task2.c -o task2_out
 * mpirun -np 4 task2_out
 */
#include <stdio.h>
#include <mpi.h>
/* This example handles a 12 x 12 mesh, on 4 processors only. */
#define maxn 12
int main( argc, argv )
int argc;
char **argv;
{
    int rank, value, size, errcnt, toterr, i, j;
    int up_nbr, down_nbr;
    MPI_Status status;
    double x[12][12];
    double xlocal[(12/4)+2][12];
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
    /* Send up and receive from below (shift up)
    */
    /* Note the use of xlocal[i] for &xlocal[i][0]
    */
    /* Note that we use MPI_PROC_NULL to remove the if statements that would be needed without MPI_PROC_NULL */
    up_nbr = rank + 1;
    if (up_nbr >= size) up_nbr = MPI_PROC_NULL;
    down_nbr = rank -1;
    if (down_nbr < 0) down_nbr = MPI_PROC_NULL; // Use sendrecv to send and recv at the same time
    MPI_Sendrecv( xlocal[maxn/size], maxn, MPI_DOUBLE, up_nbr, 0, xlocal[0], maxn, MPI_DOUBLE, down_nbr, 0, MPI_COMM_WORLD, &status );
    /* Send down and receive from above (shift down) */
    MPI_Sendrecv( xlocal[1], maxn, MPI_DOUBLE, down_nbr, 1, xlocal[maxn/size+1], maxn, MPI_DOUBLE, up_nbr, 1, MPI_COMM_WORLD, &status );
    /* Check that we have the correct results */
    errcnt = 0;
    for (i=1; i<=maxn/size; i++)
    for (j=0; j<maxn; j++)
    if (xlocal[i][j] != rank) errcnt++;
    for (j=0; j<maxn; j++) {
        if (xlocal[0][j] != rank - 1) errcnt++;
        if (rank < size - 1 && xlocal[maxn/size+1][j] != rank + 1) errcnt++;
    }
    MPI_Reduce( &errcnt, &toterr, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    if (rank == 0) {
        if (toterr)
            printf( "! found %d errors\n", toterr );
        else
            printf( "No errors\n" );
    }
    MPI_Finalize( );
    return 0;
}
