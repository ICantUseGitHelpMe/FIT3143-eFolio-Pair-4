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
#include <stdbool.h>
#define ITERATIONS 100        // The number of loops in the server and satellite
#define INTERVAL 100 * 1000   // How frequently the main processor will check for updates from nodes (in microseconds)
#define SPLITTER 157          // Divide random numbers by this to make them floats.  This number is a prime
#define THRESHOLD 80.0f       // The temperature that evokes a positive response (degrees)
#define MOD_DENOMINATOR 60.0f // The number the generated temperature is modded by.  Determines the upper range of the generated temperature
#define MIN_TEMP 25.0f        // The baseline temperature.
#define SATELLITE_CACHE 3     // The number of elements the satellite will keep in its memory
#define ROWS 2                // The number of rows of nodes
#define COLUMNS 2             // The number of columns of nodes
#define SERVER_ID 0           // The rank of the server node

// Server headers

struct Sat_Cache // The structure of the satellite cache
{
    unsigned timestamp_array[SATELLITE_CACHE]; // Stores the previous N timestamps
    float temperature_array[SATELLITE_CACHE];  // Stores the previous N temperatures (tmeperature 2 corresponds to timestamp 2, etc.)
    int coordinate_array[SATELLITE_CACHE][2];  // Stores the previous N coordinates associated with the above information
    bool process;                              // If this is keep processing, else stop.
    int index;                                 // The index of the current start of the circular arrays
};
int server_control();
void satellite(struct Sat_Cache *Cache);
void server(struct Sat_Cache *Cache);
void file_append(char *out);
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
        // READ ME
        // Philip, start your work here, you can use task 1 and 2 of Lab 10 as inspiration
        // I need (at least) the reporting node's x, y and the timestamp in the messages you send
        // Also you MAY want to consider using POSIX threads (pthreads, like what I've used) to make more threads
        // Otherwise we'll only have 4 nodes ever.

        // Get the current time using this, otherwise we won't have miliseconds
        // struct timespec t_spec;
        // clock_gettime(CLOCK_REALTIME, &t_spec);
        // unsigned long now = (t_spec.tv_nsec + t_spec.tv_sec*1e9) * 1e-6;  // This converts from nanoseconds to miliseconds
        // the unsigned long will have the time, you may have to experiment to see what MPI has that represents this
    }

    // Rejoin everything together again
    MPI_Finalize();
    return 0;
}

// Server code
int server_control()
{

    struct Sat_Cache Cache;
    Cache.process = true; // Default this to true
    Cache.index = 0;      // Default this to 0
    for (int i = 0; i < SATELLITE_CACHE; i++)
    {
        // Default all the x coords to -1, so there is never a false positive on an uninitialised array when looking up 0,0.
        Cache.coordinate_array[i][0] = -1;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, (void *)satellite, &Cache); // Activate the satellite.  We pass the address of the cache as this is all the thread takes
    server(&Cache);
    pthread_join(thread_id, NULL);

    return 0;
}

void satellite(struct Sat_Cache *Cache)
{
    // The satellite will only write to coordinate array, temperature array, and the timestamp array.
    // It will read the processing variable
    printf("Satellite\n");
    Cache->index = 0; // This keeps track of the index of the memory array

    while (Cache->process) // Continue while the processing variable is true
    {
        usleep(INTERVAL); // Sleep for a set time
        // Must use -> as Cache came in as a pointer

        float temperature = generate_temp();                         // A random temperature in range
        Cache->timestamp_array[Cache->index] = time(NULL);           // Store the timestamp
        Cache->temperature_array[Cache->index] = temperature;        // Store the generated temp
        Cache->coordinate_array[Cache->index][0] = rand() % COLUMNS; // Get the coordinates stored, in X, Y orientation
        Cache->coordinate_array[Cache->index][1] = rand() % ROWS;

        // Increment the counter variable, wrapping around when it goes over
        Cache->index++;
        Cache->index = Cache->index % SATELLITE_CACHE; // Make sure it doesn't go above the limit
    }
}

void server(struct Sat_Cache *Cache)
{
    // The server will write to the processing variable
    // It will only read from the coordinate array, temperature array, and the timestamp array.

    printf("Server\n");
    for (int counter = 0; counter < ITERATIONS; counter++) // This is the mainloop
    {
        usleep(INTERVAL); // Sleep for a set time

        // Now, check if there is any entry from the satellite that matches the node coords
        // To do this, loop backwards through the array starting from the current "head", then restarting when the end is reached
        // This ensures we take the latest first

        // TODO: Assign these variables through MPI_IRECV / RECV
        int node_x = 0; // The coords of the reporting node
        int node_y = 0;
        float temperature = 86.3f;               // The temperatureo reported by the node
        unsigned long timestamp = 1603625547246; // TODO: Mention the assumed clock sync between processes in report

        // Get the current time in miliseconds
        struct timespec t_spec;
        clock_gettime(CLOCK_REALTIME, &t_spec);                            // Get the struct wiht the time in s and ns
        unsigned long now = (t_spec.tv_nsec + t_spec.tv_sec * 1e9) * 1e-6; // Combine and convert into ms

        // Convert time to a readable string
        char readable_timestamp[50];
        struct tm tolocal;
        time_t cast_time = (time_t)timestamp;
        tolocal = *localtime(&cast_time);
        strftime(readable_timestamp, sizeof(readable_timestamp), "%Y-%m-%d %H:%M:%S %Z", &tolocal);

        int index = Cache->index;
        bool found_entry = false;
        do
        {
            index--; // Decrement at the beginning
            if (index == -1)
            {
                index = SATELLITE_CACHE - 1; // Reset to the end of the array
            }
            // printf("Data: %d, %d.  %f.  %u\n", Cache->coordinate_array[index][0], Cache->coordinate_array[index][1], Cache->temperature_array[index], Cache->timestamp_array[index]);
            if (Cache->coordinate_array[index][0] == node_x && Cache->coordinate_array[index][1] == node_y)
            {
                // The node has a reported temperature in the memory cache of the satellite
                // Make the timestamp readable
                char sat_timestamp[50];
                struct tm tolocal;
                time_t cast_time = (time_t)Cache->timestamp_array[index];
                tolocal = *localtime(&cast_time);
                strftime(sat_timestamp, sizeof(sat_timestamp), "%Y-%m-%d %H:%M:%S %Z", &tolocal);

                char buf[256];
                sprintf(buf, "Message duration: %lums  |  X: %d  |  Y: %d  |  Temperature: %f  |  Node Timestamp: %s  \n  Satellite Temperature: %f  |  Satellite Timestamp: %s\n\n\n\n", now - timestamp, Cache->coordinate_array[index][0], Cache->coordinate_array[index][1], temperature, readable_timestamp, Cache->temperature_array[index], sat_timestamp);
                found_entry = true;
                file_append(buf); // Write to file
                break;            // Don't continue, in case it finds an older (outdated) response that we don't want to use
            }
        } while (index != Cache->index);

        if (!found_entry)
        {

            char buf[256];
            sprintf(buf, "Message duration: %lums  |  X: %d  |  Y: %d  |  Temperature: %f  |  Node Timestamp: %s  \n  Satellite Temperature: NOT FOUND  |  Satellite Timestamp: NOT FOUND\n\n\n\n", now - timestamp, node_x, node_y, temperature, readable_timestamp);

            file_append(buf); // Write to file
        }
    }
    Cache->process = false; // Tell the other thread to stop
}
//File output:
void file_append(char *out)
{
    FILE *output;

    char *outputFileBuffer;                        // Declare buffer for name of output file
    outputFileBuffer = malloc(sizeof(char) * 256); // Allocate space for declared buffer
    strcpy(outputFileBuffer, "");
    snprintf(outputFileBuffer, sizeof(char) * 256, "reports.txt");
    output = fopen(outputFileBuffer, "a");

    if (output == NULL)
    {
        printf("ERROR");
        exit(1);
    }

    fprintf(output, "%s", out);
    fclose(output);
}

// Node code

// General code
float generate_temp()
{
    float random = (float)rand() / SPLITTER;      // random number, made into a float
    float modded = fmod(random, MOD_DENOMINATOR); // Put it in the appropriate range
    return modded + MIN_TEMP;
}
