/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 5 - Task 4
 *      In this task, you will modify your argument 
 * broadcast routine to communicate different data types
 * by using MPI_Pack and MPI_Unpack.
 *      Have your program read an integer and a 
 * double-precision value from standard input (from process
 * 0, as before), and communicate this to all of the other
 * processes with an MPI_Bcast call. Use MPI_Pack to pack 
 * the data into a buffer (for simplicity, you can use char 
 * packbuf[100]; but consider how to use MPI_Pack_size 
 * instead).
 *      Note that MPI_Bcast, unlike MPI_Send/MPI_Recv 
 * operations, requires that exactly the same amount of data
 * be sent and received. Thus, you will need to make sure 
 * that all processes have the same value for the count 
 * argument to MPI_Bcast.
 *      Have all processes exit when a negative integer is 
 * read. You may want to use these MPI routines in your solution:
 * MPI_Pack MPI_Unpack MPI_Bcast
 */
#include <stdio.h>
#include <mpi.h>

// Create new struct
struct valuestruct {
    int a;
    double b;
};

int main(int argc, char** argv){
    // Initialize environment variables
    struct valuestruct values;                      // Declare struct for storing int and double
    int myrank, pos, psize;                         // Declare integer to store rank, position, and packsize
    char packbuf[100];                              // Declare buffer for packing and broadcast
    
    // Define new MPI datatype for struct
    MPI_Datatype Valuetype;                         
    MPI_Datatype type[2] = { MPI_INT, MPI_DOUBLE };
    int blocklen[2] = { 1, 1 };                     
    MPI_Aint disp[2];

    // Initialise MPI and get the relevant thread details
    MPI_Init(&argc, &argv);  
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Get_address(&values.a, &disp[0]);
    MPI_Get_address(&values.b, &disp[1]);
    
    // Create MPI struct
    disp[1]=disp[1]-disp[0];
    disp[0]=0;    
    MPI_Type_create_struct(2, blocklen, disp, type, &Valuetype);
    MPI_Type_commit(&Valuetype);

    do{
        if (myrank == 0){                                                               // Main thread code
            printf("\nEnter an round number (>0) & a real number: ");                   // Request for integer and double input
            fflush(stdout);
            scanf("%d%lf", &values.a, &values.b);
            psize = 0;                                                                  // Initialize MPI_Pack size integer
            MPI_Pack(&values, 1, Valuetype, packbuf, 100, &psize, MPI_COMM_WORLD);      // Pack struct into buffer
        }

        MPI_Bcast(&psize, 1, MPI_INT, 0, MPI_COMM_WORLD);                               // Broadcast pack size variable
        MPI_Bcast(packbuf, 100, MPI_PACKED, 0, MPI_COMM_WORLD);                         // Broadcast buffer
        
        if (myrank !=0){                                                                // Code for other threads
            pos = 0;                                                                    // Initialize position integer
            MPI_Unpack(packbuf, psize, &pos, &values, 1, Valuetype, MPI_COMM_WORLD);    // Unpack broadcasted buffer using broadcaster pack size
        }

        printf("Rank: %d. value.a = %d. value.b = %lf\n", myrank, values.a, values.b);  // Print rank, value.a, and value.b
    } while(values.a > 0);                                                              // Loop as long as the input integer is greater than 0

    MPI_Type_free(&Valuetype);                                                          // Clean up MPI datatype
    MPI_Finalize();                                                                     // Finish up with MPI and close it
    return 0;
}