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
#define ITERATIONS 100000     // The number of loops in the server and satellite
#define INTERVAL 10 * 1000    // How frequently the main processor will check for updates from nodes (in microseconds)
#define SPLITTER 157          // Divide random numbers by this to make them floats.  This number is a prime
#define THRESHOLD 80.0f       // The temperature that evokes a positive response (degrees)
#define MOD_DENOMINATOR 60.0f // The number the generated temperature is modded by.  Determines the upper range of the generated temperature
#define MIN_TEMP 25.0f        // The baseline temperature.
#define SATELLITE_CACHE 5     // The number of elements the satellite will keep in its memory
#define ROWS 2                // The number of rows of nodes
#define COLUMNS 2             // The number of columns of nodes
#define BASE_REQUEST 99       // The Tag used for the requests to the base
#define SATELLITE_REQUEST 77  // The Tag used for requests to the satellite
#define SERVER_STOP 3         // The Tag used for the base stations stopping the satellite
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
    server();
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

    // When this is received, stop all communication and processing
    MPI_Request stop_code;
    int stop;
    MPI_Irecv(&stop, 1, MPI_INT, SERVER_ID, SERVER_STOP, MPI_COMM_WORLD, &stop_code);

    printf("Satellite Initialised\n");
    // Will send with tag 1 and receive tag 0, to node 0 (us)
    int received_coordinate[3]; // index 0 is the "x", index 1 is the "y"
    // Get ready to receive the request for data
    MPI_Irecv(received_coordinate, 3, MPI_INT, SERVER_ID, SATELLITE_REQUEST, MPI_COMM_WORLD, &request);
    while (1 == 1)
    {
        float temperature = generate_temp();
        timestamp_array[index_count] = time(NULL);
        temperature_array[index_count] = temperature;
        coordinate_array[index_count][0] = rand() % COLUMNS;
        coordinate_array[index_count][1] = rand() % ROWS;
        index_count++;
        index_count = index_count % SATELLITE_CACHE; // Make sure it doesn't go above the limit

        int got_request;
        MPI_Test(&request, &got_request, MPI_STATUS_IGNORE);
        // printf("Before check %d\n", got_request);

        if (got_request == 1) // A request for data was received
        {
            printf("Rollib 2 %d\n ", received_coordinate[2]);

            MPI_Request sent_info; // New requests for send and receive

            // Check if the received coordinates are in the list
            int found = 0;
            for (int index = 0; index < SATELLITE_CACHE; index++)
            {
                if (received_coordinate[0] == coordinate_array[index][0] && received_coordinate[1] == coordinate_array[index][1])
                {
                    // If coords match, return a successful response
                    // printf("MATCH\n");
                    found = 1;
                    float send_buf[2];
                    send_buf[0] = (float)timestamp_array[index]; // The timestamp (cast as a float for ease, it will become unsigned again later)
                    send_buf[1] = temperature_array[index];      // The temperature info
                    printf("Presend1 %d\n", request == MPI_REQUEST_NULL);

                    MPI_Send(send_buf, 2, MPI_FLOAT, SERVER_ID, BASE_REQUEST, MPI_COMM_WORLD); // Ssend as we don't care about the response
                    printf("Postsend\n");

                    break;
                }
            }
            if (found == 0)
            { // Wasn't found
                // printf("NO MAtch\n");
                float send_buf[2];
                send_buf[0] = -1.0f; // An impossible value for the time, so demonstrates it isn't present
                send_buf[1] = 0.0f;
                printf("Presend2 %d\n", request == MPI_REQUEST_NULL);

                MPI_Send(send_buf, 2, MPI_FLOAT, SERVER_ID, BASE_REQUEST, MPI_COMM_WORLD); // Ssend as we don't care about the response
                printf("Postsend\n");
            }
            MPI_Request req2;
            usleep(1); // Pause before next iteration
            request = req2;
            MPI_Irecv(received_coordinate, 3, MPI_INT, SERVER_ID, SATELLITE_REQUEST, MPI_COMM_WORLD, &request); // Can;t be the same request, try an array
            printf("AFTER new recv\n");
        }
        printf("Temperature: %f\n", temperature);
        int got_stop;
        MPI_Test(&stop_code, &got_stop, MPI_STATUS_IGNORE);
        if (got_stop == 1) // The Kill Order was received
        {
            break; // Leave and end
        }
        usleep(INTERVAL);
    }

    printf("Satellite Has Finalised\n");
}
void server()
{
    printf("Server Initialised\n");
    // Will send with tag 0 and receive tag 1, to node 0 (us)
    char readable_timestamp[50]; // Stores the timestamp when it is getting cast
    for (int counter = 0; counter < ITERATIONS; counter++)
    {
        usleep(INTERVAL); // Wait

        if (counter % 50 == 0)
        {
            // printf("%d\n", counter);
        }
        int coords[3]; // Stores the cooreds of the fire
        coords[0] = 0;
        coords[1] = 0;
        coords[2] = counter; // debug

        printf("Preblock\n");
        // Blocking send is used here to maintain the strict order

        MPI_Send(coords, 3, MPI_INT, SERVER_ID, SATELLITE_REQUEST, MPI_COMM_WORLD);
        printf("Midblock %d\n", coords[2]);

        MPI_Request testr;
        float received[2]; // The returned info
        MPI_Irecv(received, 2, MPI_FLOAT, SERVER_ID, BASE_REQUEST, MPI_COMM_WORLD, &testr);
        printf("Waiting\n");
        // Implement a timeout here:
        int did_timeout = 0;
        for (int wait; wait < 10; wait++)
        {
            int m_check;
            MPI_Test(&testr, &m_check, MPI_STATUS_IGNORE);
            if (m_check == 1){
                break;
            }
            else if (wait == 9){  // Last iteration.  Time out
                did_timeout = 1;
                MPI_Cancel( &testr);
                MPI_Request_free( &testr);
            }
            usleep(10000);  // Wait for the timeout a bit
        }
        printf("Postblock\n");
        if (did_timeout == 1){
            //TODO:  Log the failed response
            received[0] = -1;
            received[1] = 0;
        }
        if (received[0] > 0)
        { // Found
            printf("FUNd\n");
            struct tm tolocal;
            time_t cast_time = (time_t)received[0];
            tolocal = *localtime(&cast_time);
            strftime(readable_timestamp, sizeof(readable_timestamp), "%Y-%m-%d %H:%M:%S %Z", &tolocal);
            printf("done finded\n");
            // printf("MAtch: %s, %f!\n", readable_timestamp, received[1]);
        }
        else{
            // Log the lack of findage.  If the satellite failed, log that too
        }
    }
    int flag = 0;
    printf("Server Finalising\n");

    MPI_Send(&flag, 1, MPI_INT, SERVER_ID, SERVER_STOP, MPI_COMM_WORLD);
    printf("Server Finalised\n");
}

// Node code

// General code
float generate_temp()
{
    float random = (float)rand() / SPLITTER;      // random number, made into a float
    float modded = fmod(random, MOD_DENOMINATOR); // Put it in the appropriate range
    return modded + MIN_TEMP;
}
