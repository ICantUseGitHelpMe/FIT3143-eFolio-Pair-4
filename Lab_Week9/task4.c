/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 9 - Task 3 
 * mpicc task3.c -o task3_out 
 * mpirun -np 4 task3_out
 */

/* Gets the neighbors in a cartesian communicator
* Orginally written by Mary Thomas
* - Updated Mar, 2015
* Link: https://edoras.sdsu.edu/~mthomas/sp17.605/lectures/MPICart-Comms-and-Topos.pdf
* Minor modifications to fix bugs and to revise print output
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
// Define the specific codes for different kinds of messages
#define ORDERED 10
#define UNORDERED 1
#define EXIT 2
int master_io(MPI_Comm master_comm, MPI_Comm comm);
int slave_io(MPI_Comm master_comm, MPI_Comm comm);
int main(int argc, char **argv)
{
    int rank;
    MPI_Comm new_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_split(MPI_COMM_WORLD, rank == 0, 0, &new_comm);
    if (rank == 0)
        master_io(MPI_COMM_WORLD, new_comm);
    else
        slave_io(MPI_COMM_WORLD, new_comm);
    MPI_Finalize();
    return 0;
}
/* This is the master 
*************************************
*/
int master_io(MPI_Comm master_comm, MPI_Comm comm)
{
    int i, j, size;
    int is_finished[3];
    is_finished[0] = is_finished[1] = is_finished[2] = 0;
    char buf[256];
    MPI_Status status;
    MPI_Comm_size(master_comm, &size);
    int stop = 0;
    while (stop == 0)
    {
        int ordered[size - 1]; // Shows who wants an ordered output
        for (int node = 0; node < size - 1; node++)
        {
            ordered[node] = 0; // default to 0
        }
        for (int k = 1; k < size; k++)
        {
            int code;

            if (stop == 1) return 0;
            MPI_Recv(&code, 1, MPI_INT, k, 0, master_comm, &status);
            if (code == UNORDERED)
            {
                // Unordered will always go before ordered BUT
                // if multiple ordered are in the buffer at the same time, we decided it will default to lowest rank first
                // Recieve and print right now:
                MPI_Recv(buf, 256, MPI_CHAR, k, 0, master_comm, &status);
                fputs(buf, stdout);

                char test_buf[256];
                sprintf(test_buf, "Goodbye"); // If it's a goodbye message, end the process
                int matched = 1;
                for (int x = 0; x < 7; x++)
                {
                    if (test_buf[x] != buf[x])
                    {

                        matched = 0;
                    }
                }
                if (matched == 1)
                {
                    is_finished[k - 1] = 1;
                }
            }
            else if (code == ORDERED)
            {
                ordered[k - 1] = 1;
            }
        }

        for (j = 1; j <= 2; j++)
        {
            for (i = 1; i < size; i++)
            {
                if (ordered[i - 1] == 1)
                { // Print what was ordered
                    if (is_finished[i-1] == 0){
                    MPI_Recv(buf, 256, MPI_CHAR, i, 0, master_comm, &status);
                    fputs(buf, stdout);
                    char test_buf[256];
                    sprintf(test_buf, "Goodbye"); // If it's a goodbye message, end the process

                    int matched = 1;
                    for (int x = 0; x < 7; x++)
                    {

                        if (test_buf[x] != buf[x])
                        {

                            matched = 0;
                        }
                    }
                    if (matched == 1)
                    {
                        is_finished[i - 1] = 1;
                    }
                }}
            }
        }

        stop = 1;
        for (int loop = 0; loop < size - 1; loop ++)
        {
            if (is_finished[loop] == 0)
            {
                stop = 0; // Keep going if there is at least one left
            }

        }
    }
    return 0;
}
/* This is the slave 
*************************************
*/
int slave_io(MPI_Comm master_comm, MPI_Comm comm)
{
    char buf[256];
    int rank;

    MPI_Comm_rank(comm, &rank);

    int code = UNORDERED; // Sending an ordered communication

    MPI_Send(&code, 1, MPI_INT, 0, 0, master_comm);

    sprintf(buf, "Hello from slave %d\n", rank);
    MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 0, 0, master_comm);

    code = ORDERED;


    MPI_Send(&code, 1, MPI_INT, 0, 0, master_comm);

    sprintf(buf, "Goodbye from slave %d\n", rank);
    MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 0, 0, master_comm);
    return 0;
}