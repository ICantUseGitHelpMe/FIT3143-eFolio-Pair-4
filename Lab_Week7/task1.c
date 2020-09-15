#include <stdio.h>
#include <mpi.h>

/* This example handles a 12 x 12 mesh, on 4 processors only. */
#define maxn 12

int main( argc, argv )
int argc;
char **argv;
{
    int rank, value, size, errcnt, toterr, i, j, up_nbr, down_nbr;
    MPI_Status status;
    double x[12][12];
    double xlocal[(12/4)+2][12];
    MPI_Request request[size * 2];
    MPI_Status status[size * 2];
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

    /*
      CALL MPI_COMM_RANK(comm, rank, ierr)
    IF(rank.EQ.0) THEN
        CALL MPI_ISEND(a(1), 10, MPI_REAL, 1, tag, comm, request, ierr)
        **** do some computation ****
        CALL MPI_WAIT(request, status, ierr)
    ELSE
        CALL MPI_IRECV(a(1), 15, MPI_REAL, 0, tag, comm, request, ierr)
        **** do some computation ****
        CALL MPI_WAIT(request, status, ierr)
    END IF
    */
   
    up_nbr = rank + 1;  // This prevents crashing from occurring if the current process is 0 or size - 1 (max)
    down_nbr = rank - 1;
    if (up_nbr >= size){
        up_nbr = MPI_PROC_NULL;
    }
    if (down_nbr < 0){
        down_nbr = MPI_PROC_NULL;
    }
    
    // Send above / Send below
    MPI_ISend( xlocal[maxn/size], maxn, MPI_DOUBLE, up_nbr, 0, MPI_COMM_WORLD, &request[rank*2] );
    MPI_ISend( xlocal[1], maxn, MPI_DOUBLE, down_nbr 1, MPI_COMM_WORLD, &request[rank*2 + 1] );

    MPI_Waitall(size*2, request);
    // receive above / receive below
    MPI_Recv( xlocal[maxn/size+1], maxn, MPI_DOUBLE, up_nbr, 1, MPI_COMM_WORLD, &status );
    MPI_Recv( xlocal[0], maxn, MPI_DOUBLE, down_nbr, 0, MPI_COMM_WORLD, &status );


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
            errcnt++
        };
    }

    MPI_Reduce( &errcnt, &toterr, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
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