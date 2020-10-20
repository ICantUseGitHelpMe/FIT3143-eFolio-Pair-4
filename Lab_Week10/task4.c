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
#include <memory.h>
#define CONTINUE 0.0
#define END 1.0
void file_append(char *out);
void file_float_append(float out);
int main(int argc, char **argv)
{
    int fileSize = 0;
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
            FILE *inputFile;
            int counter = 0;
            double *memBuff = NULL;
            float a, b, c;
            float d;
            int fileSize;
            inputFile = fopen("quad.txt", "r");
            fscanf(inputFile, "%d", &fileSize);
            memBuff = (double *)malloc(fileSize * sizeof(double));
            memset(memBuff, 0, fileSize * sizeof(double));
            // Send the counter to the last process
            // Read each element from the file
            char discard[50];
            fscanf(inputFile, "%s%s%s", discard, discard, discard); // Ignore the a, b, c
            counter++;
            while (counter < fileSize)
            {
                fscanf(inputFile, "%f", &a);
                // printf("%f ", a);
                fscanf(inputFile, "%f", &b);
                // printf("%f ", b);
                fscanf(inputFile, "%f", &c);
                // printf("%f\n", c);

                float send_buf[4]; // Send D, then A, then B, then the sentinal
                send_buf[0] = pow(b, 2.0) - 4 * a * c;
                send_buf[1] = a;
                send_buf[2] = b;
                counter++;

                if (counter == fileSize)
                {
                    send_buf[3] = END; // Send the stop code if counter is at the end, otherwise continue.
                }
                else
                {
                    send_buf[3] = CONTINUE; // Send the stop code if counter is at the end, otherwise continue.
                }
                MPI_Send(send_buf, 4, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
            }
            // Close up the file
            printf("ENDED 0:\n");

            fclose(inputFile);
            inputFile = NULL;
        }
        case 1:
        {
            int stop = 0;
            while (stop == 0) // Continue until told to stop
            {
                // WRITE PART (b) HERE
                float input_buf[4]; // d, a, b, and a sentinal
                MPI_Recv(input_buf, 4, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                float root1, root2, x1_real, x1_img, x2_real, x2_img = 0;
                float d = input_buf[0];
                float a = input_buf[1];
                float b = input_buf[2];
                if (d == 0)
                {
                    root1 = root1 = -b / (2 * a);
                }
                else if (d > 0)
                {
                    root1 = (-b + sqrt(d)) / (2 * a);
                    root2 = (-b - sqrt(d)) / (2 * a);
                }
                else
                { // d is negative, so we have imaginary roots
                    x1_real = -b / (2 * a);
                    x1_img = sqrt(abs(d)) / (2 * a);
                    x2_real = -b / (2 * a);
                    x2_img = sqrt(abs(d)) / (2 * a);
                }

                stop = input_buf[3]; // 0 if the previous node wants to continue, 1 if it wants to stop

                // prepare the information for sending
                float output_buf[7];
                output_buf[0] = root1;
                output_buf[1] = root2;
                output_buf[2] = x1_real;
                output_buf[3] = x1_img;
                output_buf[4] = x2_real;
                output_buf[5] = x2_img;
                output_buf[6] = stop; // Send whether we want to stop

                MPI_Send(output_buf, 6, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD);
            }
            printf("ENDED 1:\n");

            break;
        }
        case 2:
        {
            // x1 and x_2's real and img will all be 0 if they are not used - this is impossible to happen so it is a clear indication the numbers are real
            // Input is: root1, root2 (roots if not imaginary), x1 real, x1 img, zx2 real, x2 img (if imaginary)
            // WRITE PART (c) HERE
            file_append("x1,x2,x1_real,x1_img x2_real,x2_img\n");

            int stop = 0;
            while (stop == 0)
            {
                float output_buf[7];
                MPI_Recv(output_buf, 7, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if ((output_buf[2] == 0) && (output_buf[3] == 0) && (output_buf[4] == 0) && (output_buf[5] == 0))
                {
                    float root1, root2;
                    root1 = output_buf[0];
                    root2 = output_buf[1];
                    char store_buf[50];
                    sprintf(store_buf, "%f", root1); // Convert to string
                    file_append(store_buf);
                    file_append(",");
                    sprintf(store_buf, "%f", root2);
                    file_append(store_buf);
                    file_append(", , , , \n");
                }
                else
                {
                    float x1_real, x1_img, x2_real, x2_img;
                    char store_buf[50];
                    x1_real = output_buf[2];
                    x1_img = output_buf[3];
                    x2_real = output_buf[4];
                    x2_img = output_buf[5];
                    file_append(" , ,");
                    sprintf(store_buf, "%f", x1_real);
                    file_append(store_buf);
                    file_append(",");
                    sprintf(store_buf, "%f", x1_img);

                    file_append(store_buf);
                    file_append(",");
                    sprintf(store_buf, "%f", x2_real);

                    file_append(store_buf);
                    file_append(",");
                    sprintf(store_buf, "%f", x2_img);

                    file_append(store_buf);
                }
                file_append("\n");
                // If the input from node 1 says stop
                stop = output_buf[6];
            }
            printf("ENDED 2:\n");

            break;
        }
        default:
        {
            printf("END N:\n");
            break;
        }
    }
                        

    MPI_Finalize();
    printf("ENDED ALL:\n");
    return 0;
}
//File output:
void file_append(char *out)
{
    FILE *output;

    char *outputFileBuffer;                        // Declare buffer for name of output file
    outputFileBuffer = malloc(sizeof(char) * 256); // Allocate space for declared buffer
    strcpy(outputFileBuffer, "");
    snprintf(outputFileBuffer, sizeof(char) * 256, "roots.txt");
    output = fopen(outputFileBuffer, "a");

    if (output == NULL)
    {
        printf("ERROR");
        exit(1);
    }

    fprintf(output, "%s", out);
    fclose(output);
}

void file_float_append(float out)
{
    FILE *output;

    char *outputFileBuffer;                        // Declare buffer for name of output file
    outputFileBuffer = malloc(sizeof(char) * 256); // Allocate space for declared buffer
    strcpy(outputFileBuffer, "");
    snprintf(outputFileBuffer, sizeof(char) * 256, "roots.txt");
    output = fopen(outputFileBuffer, "a");

    if (output == NULL)
    {
        printf("ERROR");
        exit(1);
    }

    fprintf(output, "%f", out);
    fclose(output);
}