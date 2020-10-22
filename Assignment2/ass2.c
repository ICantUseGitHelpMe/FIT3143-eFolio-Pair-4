// Assignment 2
// mpicc ass2.c -o ass2_out -lm
// mpirun -np 4 ass2_out

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#define ITERATIONS 1000       // The number of loops in the server and satellite
#define INTERVAL 50 * 1000    // How frequently the main processor will check for updates from nodes (in microseconds)
#define SPLITTER 157          // Divide random numbers by this to make them floats.  This number is a prime
#define THRESHOLD 80.0f       // The temperature that evokes a positive response (degrees)
#define MOD_DENOMINATOR 60.0f // The number the generated temperature is modded by.  Determines the upper range of the generated temperature
#define MIN_TEMP 25.0f        // The baseline temperature.
#define SATELLITE_CACHE 5     // The number of elements the satellite will keep in its memory
#define ROWS 2                // The number of rows of nodes
#define COLUMNS 2             // The number of columns of nodes
#define SATELLITE_REQUEST 2   // The Tag used for requests to the satellite
#define BASE_REQUEST 1        // The Tag used for the requests to the base
#define SERVER_ID 0           // The rank of the server node
// Server headers
int server_control();
void satellite();
void server();

// Node headers

// General headers
float generate_temp(); // Generate a random temperature

// Main loop
int main(int argc, char **argv)
{
    // First, split between the server and the nodes
    // Standard boilerplate
    int rank, size;
    MPI_Comm new_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_split(MPI_COMM_WORLD, rank == size - 1, 0, &new_comm);
    srand((unsigned)time(NULL) + rank); // Seed the random, uniquely for each thread

    //Server Controls:
    if (rank == SERVER_ID)
    {
        server_control();
    }
    // Node controls
    else
    {
        // Philip, start your work here, you can use task 1 and 2 of Lab 10 as inspiration
    }

    // Rejoin everything together again
    MPI_Finalize();
    return 0;
}

// Server code
int server_control()
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, (void *)satellite, NULL); // Activate the satellite
    server();                                                  // Activate the server
    pthread_join(thread_id, NULL);
    return 0;
}

void satellite()
{
    MPI_Request request;
    int index_count = 0; // This keeps track of the index of the memory array
    int DEBUG = 0;
    unsigned timestamp_array[SATELLITE_CACHE]; // Stores the previous N timestamps
    float temperature_array[SATELLITE_CACHE];  // Stores the previous N temperatures (tmeperature 2 corresponds to timestamp 2, etc.)
    int coordinate_array[SATELLITE_CACHE][2];  // Stores the previous N coordinates associated with the above information

    // Will send with tag 1 and receive tag 0, to node 0 (us)
    printf("Satellite!\n");
    float received_coordinate[2]; // index 0 is the "x", index 1 is the "y"
    // Get ready to receive the request for data
    MPI_Irecv(received_coordinate, 2, MPI_INT, SERVER_ID, SATELLITE_REQUEST, MPI_COMM_WORLD, &request);
    for (int counter = 0; counter < ITERATIONS; counter++)
    {
        float temperature = generate_temp();
        if (temperature > THRESHOLD)
        {
            timestamp_array[index_count] = time(NULL);
            temperature_array[index_count] = temperature;
            coordinate_array[index_count][0] = DEBUG;
            coordinate_array[index_count][1] = DEBUG++;
            index_count++;
            index_count = index_count % SATELLITE_CACHE; // Make sure it doesn't go above the limit
        }
        int got_request;
        MPI_Test(&request, &got_request, MPI_STATUS_IGNORE);
        if (got_request == 1) // A request for data was received
        {
            MPI_Request sent_info;  // New requests for send and receive

            MPI_Request new_req;
            // Check if the received coordinates are in the list
            int found = 0;
            for (int index = 0; index < SATELLITE_CACHE; index++)
            {
                if (received_coordinate[0] == coordinate_array[index][0] && received_coordinate[1] == coordinate_array[index][1])
                {
                    // If coords match, return a successful response
                    printf("MATCH\n");
                    found = 1;
                    float send_buf[2];
                    send_buf[0] = (float) timestamp_array[index]; // The timestamp (cast as a float for ease, it will become unsigned again later)
                    send_buf[1] = temperature_array[index]; // The temperature info
                    MPI_Isend(send_buf, 2, MPI_FLOAT, SERVER_ID, BASE_REQUEST, MPI_COMM_WORLD, &sent_info);
                }
            }
            if (found == 0)
            { // Wasn't found

                float send_buf[2];
                send_buf[0] = -1;  // An impossible value for the time, so demonstrates it isn't present
                send_buf[1] = 0;
                MPI_Isend(send_buf, 2, MPI_FLOAT, SERVER_ID, BASE_REQUEST, MPI_COMM_WORLD, &sent_info);
            }

            // printf("Test response %d\n", got_request);
            // Now prepare to receive again
            MPI_Irecv(received_coordinate, 2, MPI_INT, SERVER_ID, SATELLITE_REQUEST, MPI_COMM_WORLD, &new_req);  // Can;t be the same request, try an array
        }
        // printf("Temperature: %f\n", temperature);
        usleep(INTERVAL);
    }

    char readable_timestamp[50];

    // Debug printing
    for (int index = 0; index < SATELLITE_CACHE; index++)
    {
        struct tm tolocal;
        time_t cast_time = (time_t)timestamp_array[index];
        tolocal = *localtime(&cast_time);
        strftime(readable_timestamp, sizeof(readable_timestamp), "%Y-%m-%d %H:%M:%S %Z", &tolocal);
        printf("Temperature: %f, Time: %s, Coords: %d, %d\n", temperature_array[index], readable_timestamp, coordinate_array[index][0], coordinate_array[index][1]);
    }
    // MPI_Cancel(&new_req);  // Remove any outstanding sends
}
void server()
{
    // Will send with tag 0 and receive tag 1, to node 0 (us)
    for (int counter = 0; counter < ITERATIONS; counter++)
    {
        int coords[2]; // Stores the cooreds of the fire
        coords[0] = 2;
        coords[1] = 4;
        MPI_Send(coords, 2, MPI_INT, SERVER_ID, SATELLITE_REQUEST, MPI_COMM_WORLD);
        float received[2]; // The returned info
        MPI_Recv(received, 2, MPI_FLOAT, SERVER_ID, BASE_REQUEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (received[0] > 0)
        { // Found
            printf("MAtch: %u, %f!\n", (unsigned)received[0], received[1]);
        }
        // printf("Server!\n");
        usleep(INTERVAL);
    }
}

// Node code

// General code
float generate_temp()
{
    float random = (float)rand() / SPLITTER;      // random number, made into a float
    float modded = fmod(random, MOD_DENOMINATOR); // Put it in the appropriate range
    return modded + MIN_TEMP;
}
