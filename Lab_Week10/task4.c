/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 10 - Task 4 
 * mpicc task4.c -o task4_out -lm
 * mpirun -np 4 task4_out
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#define CONTINUE 0
#define END 1
int main(int argc, char **argv)
{
    FILE *pInfile;
    float a_coeff, b_coeff, c_coeff, x1, x2, disc;
    float x1r, x1i, x2r, x2i;
    int fileElementCount = 0, constNeg = -1;
    int my_rank;
    int p;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    // WRITE PART(a) HERE
    switch (my_rank)
    {
    case 0:
    {
        // CONTINUE WITH PART (a) HERE
        pInfile = fopen("quad.txt","r");
        char *read, *line;
        size_t len;
        while ((read =  getline(&line, &len, pInfile)) != -1) {
            printf("Retrieved line of length %zu:\n", read);
            printf("%s", line);
            char * element = strtok (line," ");
            while (element != NULL){
                printf( " %s\n", element ); //printing each token
                element = strtok(NULL, " ");
            }
        }
        break;
    }
    case 1:
    {
        int stop = 0;
        while (stop == 0)  // Continue until told to stop
        {
            // WRITE PART (b) HERE
            float input_buf[4];  // d, a, b, and a sentinal
            MPI_Recv(input_buf, 4, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            float root1, root2, x1_real, x1_img, x2_real, x2_img = 0;
            float d = input_buf[0];
            float a = input_buf[1];
            float b = input_buf[2];
            if (d == 0) {
                root1 = root1 = -b / (2 * a);
            }
            else if (d > 0){
                root1 = (-b + sqrt(d)) / (2 * a);
                root2 = (-b - sqrt(d)) / (2 * a);
            }
            else{  // d is negative, so we have imaginary roots
                x1_real = -b / (2*a);
                x1_img = sqrt(abs(d)) / (2*a);
                x2_real = -b / (2*a);
                x2_img = sqrt(abs(d)) / (2*a);
            }

            stop = input_buf[3];  // 0 if the previous node wants to continue, 1 if it wants to stop

            // prepare the information for sending
            float output_buf[7];
            output_buf[0] = root1;
            output_buf[1] = root2;
            output_buf[2] = x1_real;
            output_buf[3] = x1_img;
            output_buf[4] = x2_real;
            output_buf[5] = x2_img;
            output_buf[6] = stop;  // Send whether we want to stop
            
            MPI_Send(output_buf, 6, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD);
        }
        break;
    }
    case 2:
    {
        // x1 and x_2's real and img will all be 0 if they are not used - this is impossible to happen so it is a clear indication the numbers are real
        // Input is: root1, root2 (roots if not imaginary), x1 real, x1 img, zx2 real, x2 img (if imaginary)
        // WRITE PART (c) HERE
        break;
    }
    }
    MPI_Finalize();
    return 0;
}