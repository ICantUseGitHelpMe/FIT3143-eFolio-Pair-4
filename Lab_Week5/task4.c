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
#include <stdlib.h>
#include <mpi.h>

struct valuestruct {
    int a;
    double b;
};
int main(int argc, char** argv){
    struct valuestruct values;  // Prepare the struct with MPI data types
    
    int myrank;
    MPI_Datatype Valuetype;
    MPI_Datatype type[2] = { MPI_INT, MPI_DOUBLE };
    int blocklen[2] = { 1, 1};
    MPI_Aint disp[2];


    MPI_Init(&argc, &argv);  // Initialise MPI and get the relevant thread details
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Get_address(&values.a, &disp[0]);
    MPI_Get_address(&values.b, &disp[1]);

    char packbuf[100];  // Initialise the buffer for sending
    char outbuf[100];  // Initialise the buffer for sending
    int pos = 0;
    int output_va;
    double output_vb;

    int checker = 0;
    //Make relative
    disp[1]=disp[1]-disp[0];
    disp[0]=0;
    // Create MPI struct
    MPI_Type_create_struct(2, blocklen, disp, type, &Valuetype);
    MPI_Type_commit(&Valuetype);
    do{

        if (myrank == 0){  // Main thread code
            printf("Enter an round number (>0) & a real number: ");
            fflush(stdout);
            scanf("%d%lf", &values.a, &values.b);

            MPI_Pack(&values, 1, Valuetype, outbuf, 100, &pos, MPI_COMM_WORLD);  // Pack up the data and store it in the buffer
        }


        MPI_Bcast(outbuf, 2, Valuetype, 0, MPI_COMM_WORLD);  // Send the data and output it



        if (myrank == 0){
            printf("Rank: %d. values.a = %d. values.b = %lf\n", myrank, values.a, values.b);
            checker = values.a;
        }
        
        MPI_Unpack(outbuf, 100, &pos, &values, 1, Valuetype, MPI_COMM_WORLD);
        

        if (myrank != 0){
        
            printf("Rank: %d. values.a = %d. values.b = %lf\n", myrank, values.a, values.b);
        }
        fflush(stdout);

        
    } while(checker > 0);

    /* Clean up the type */
    MPI_Type_free(&Valuetype);  
    MPI_Finalize(); // Finish up with MPI and close it
    return 0;
}