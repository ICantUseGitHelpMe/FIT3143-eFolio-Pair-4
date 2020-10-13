/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 9 - Task 2 
 * mpicc task2.c -o task2_out -lm
 * mpirun -np 4 task2_out
 */

/* Gets the neighbors in a cartesian communicator
* Orginally written by Mary Thomas
* - Updated Mar, 2015
* Link: https://edoras.sdsu.edu/~mthomas/sp17.605/lectures/MPICart-Comms-and-Topos.pdf
* Minor modifications to fix bugs and to revise print output
*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <time.h>
#define SHIFT_ROW 0
#define SHIFT_COL 1
#define DISP 1
#define TARGET 1000 // The number of iterations in the prime check
void file_append(int out, int thread);
void file_clear(int thread);
int is_prime(int input_integer);
int rand_prime(int my_rank);
int main(int argc, char *argv[]) {
    int ndims=2, size, my_rank, reorder, my_cart_rank, ierr;
    int nrows, ncols;
    int nbr_i_lo, nbr_i_hi;
    int nbr_j_lo, nbr_j_hi;
    MPI_Comm comm2D;
    int dims[ndims],coord[ndims];
    int wrap_around[ndims];    
    /* Seed generator */
    srand((unsigned) time(NULL));
    /* start up initial MPI environment */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* process command line arguments*/
    if (argc == 3) {
        nrows = atoi (argv[1]);
        ncols = atoi (argv[2]);
        dims[0] = nrows; /* number of rows */
        dims[1] = ncols; /* number of columns */
        if( (nrows*ncols) != size) {
            if( my_rank ==0) printf("ERROR: nrows*ncols)=%d * %d = %d != %d\n", nrows, ncols, nrows*ncols,size);
            MPI_Finalize();
            return 0;
        }
    } else {
        nrows=ncols=(int)sqrt(size);
        dims[0]=dims[1]=0;
    }
    /************************************************************
    */
    /* create cartesian topology for processes */
    /************************************************************
    */
    MPI_Dims_create(size, ndims, dims);
    if(my_rank==0)
        printf("Root Rank: %d. Comm Size: %d: Grid Dimension = [%d x %d] \n",my_rank,size,dims[0],dims[1]);
    /* create cartesian mapping */
    wrap_around[0] = wrap_around[1] = 0; /* periodic shift is .false. */
    reorder = 1;
    ierr =0;
    ierr = MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, wrap_around, reorder, &comm2D);
    if(ierr != 0) printf("ERROR[%d] creating CART\n",ierr);
    /* find my coordinates in the cartesian communicator group */
    MPI_Cart_coords(comm2D, my_rank, ndims, coord);
    /* use my cartesian coordinates to find my rank in cartesian group*/
    MPI_Cart_rank(comm2D, coord, &my_cart_rank);
    /* get my neighbors; axis is coordinate dimension of shift */
    /* axis=0 ==> shift along the rows: P[my_row-1]: P[me] :
    P[my_row+1] */
    /* axis=1 ==> shift along the columns P[my_col-1]: P[me] :
    P[my_col+1] */
    MPI_Cart_shift( comm2D, SHIFT_ROW, DISP, &nbr_i_lo, &nbr_i_hi);
    MPI_Cart_shift( comm2D, SHIFT_COL, DISP, &nbr_j_lo, &nbr_j_hi);
    printf("Global rank: %d. Cart rank: %d. Coord: (%d, %d). Left: %d. Right: %d. Top: %d. Bottom: %d\n", my_rank, my_cart_rank, coord[0], coord[1], nbr_j_lo, nbr_j_hi, nbr_i_lo, nbr_i_hi);
    fflush(stdout);
    file_clear(my_rank);
    for (int looper = 0; looper < TARGET; looper++)
    {
        // array = [-1, -1, 252341, 998989831]
        int myprime =   rand_prime(my_rank);
        
        int primes[4];  // Will either contain the prime, or -1 if there is no associated cartesian neighbour

        if (nbr_i_lo >= 0){
            MPI_Request send_request;
            MPI_Request recv_request;
            int their_prime;
            
            MPI_Isend(&myprime, 1, MPI_INT, nbr_i_lo, 1, MPI_COMM_WORLD, &send_request);
            MPI_Irecv( &their_prime, 1, MPI_INT, nbr_i_lo, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
            MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
            primes[0] = their_prime; // Update the prime to the index

            printf("Rank %d got %d from %d\n", my_rank, their_prime, nbr_i_lo);
        }
        else{
            primes[0] = -1;
        }
        if (nbr_j_lo >= 0){
            MPI_Request send_request;
            MPI_Request recv_request;
            int their_prime;

            MPI_Isend(&myprime, 1, MPI_INT, nbr_j_lo, 1, MPI_COMM_WORLD, &send_request);
            MPI_Irecv( &their_prime, 1, MPI_INT, nbr_j_lo, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);

            // MPI_Wait(&send_request, MPI_STATUS_IGNORE);
            MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
            primes[1] = their_prime;

            printf("Rank %d got %d from %d\n", my_rank, their_prime, nbr_j_lo);
        }
        else{
            primes[1] = -1;  // Show it is unresponsive

        }
        if (nbr_i_hi >= 0){
            MPI_Request send_request;
            MPI_Request recv_request;
            int their_prime;
            MPI_Isend(&myprime, 1, MPI_INT, nbr_i_hi, 1, MPI_COMM_WORLD, &send_request);
            MPI_Irecv( &their_prime, 1, MPI_INT, nbr_i_hi, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
            // MPI_Wait(&send_request, MPI_STATUS_IGNORE);
            MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
            primes[2] = their_prime;  // Get their prime

            printf("Rank %d got %d from %d\n", my_rank, their_prime, nbr_i_hi);
        }
        else{
            primes[2] = -1;  // Show it is unresponsive            
        }
        if (nbr_j_hi >= 0){
            MPI_Request send_request;
            MPI_Request recv_request;
            int their_prime;

            MPI_Isend(&myprime, 1, MPI_INT, nbr_j_hi, 1, MPI_COMM_WORLD, &send_request);
            MPI_Irecv( &their_prime, 1, MPI_INT, nbr_j_hi, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
            // MPI_Wait(&send_request, MPI_STATUS_IGNORE);
            MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
            primes[3] = their_prime;  // Show it is unresponsive
            printf("Rank %d got %d from %d\n", my_rank, their_prime, nbr_j_hi);
        }
        else{
            primes[3] = -1;
        }
        int to_append = 1;
        for (int index = 0; index < 4; index++) {
            if (myprime != primes[index] && primes[index] != -1){
                to_append = 0;
            }
        }
        if (to_append == 1){
            file_append(myprime, my_rank);
            to_append = 0;
        }
    }
    MPI_Comm_free( &comm2D );
    MPI_Finalize();
    return 0;
}

int rand_prime(int my_rank){
    int iterative_integer;
    int n = 30, returned = 0;
    while (returned == 0){
        iterative_integer = rand() % 10 + my_rank;
        for (iterative_integer; iterative_integer < n; iterative_integer++) {
            int prime_boolean = is_prime(iterative_integer);
            if (prime_boolean == 1){
                returned = 1;
                return(iterative_integer);
            }
        }
    }
}

//File output:
void file_append (int out, int thread){
	FILE *output;

    char * outputFileBuffer;                                                            // Declare buffer for name of output file
    outputFileBuffer = malloc(sizeof(char) * 256);                                      // Allocate space for declared buffer
    strcpy(outputFileBuffer, "");                                                       
    snprintf(outputFileBuffer, sizeof(char) * 256, "task_2_output_%d.txt", thread);
    output = fopen(outputFileBuffer, "a");

	if(output == NULL){
		printf("ERROR");
		exit(1);
	}

	fprintf(output, "%d\n", out);
	fclose(output);

}

//File clear:
void file_clear (int thread){
	FILE *output;
    
    char * outputFileBuffer;
    outputFileBuffer = malloc(sizeof(char) * 256);
    strcpy(outputFileBuffer, "");
    snprintf(outputFileBuffer, sizeof(char) * 256, "task_2_output_%d.txt", thread);
    output = fopen(outputFileBuffer, "w");

	if(output == NULL){
		printf("ERROR");
		exit(1);
	}

	fclose(output);

}

// Function for determining whether or not a number is prime
int is_prime(int input_integer){
    // Let input_integer be the integer to be tested for prime property
    if (input_integer <= 1){
        return(0);
    }
    int input_root;
    // Find square root of input_integer
    input_root = floor(sqrt(input_integer));
    // Iterate through each integer, i, from 2 to square root of input_integer
    for (int iterative_integer = 2; iterative_integer <= input_root; iterative_integer++) {
        // If the remainder of input_integer divded by i is 0, return 0
        int remainder = input_integer % iterative_integer;
        if (remainder == 0){
            return(0);
        }
    }
    // If the remainder of input_integer divided by all i values is not 0, return 1
    return(1);
}