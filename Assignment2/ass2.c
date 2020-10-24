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
#define ITERATIONS 10000      // The number of loops in the server and satellite
#define INTERVAL 10 * 1000    // How frequently the main processor will check for updates from nodes (in microseconds)
#define SPLITTER 157          // Divide random numbers by this to make them floats.  This number is a prime
#define THRESHOLD 80.0f       // The temperature that evokes a positive response (degrees)
#define MOD_DENOMINATOR 60.0f // The number the generated temperature is modded by.  Determines the upper range of the generated temperature
#define MIN_TEMP 25.0f        // The baseline temperature.
#define SATELLITE_CACHE 5     // The number of elements the satellite will keep in its memory
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
};
int server_control();
void satellite(struct Sat_Cache *Cache);
void server(struct Sat_Cache *Cache);
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

    struct Sat_Cache Cache;
    Cache.process = true;  // Default this to true
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
    int index_count = 0; // This keeps track of the index of the memory array

    while (Cache->process) // Continue while the processing variable is true
    {
        usleep(INTERVAL); // Sleep for a set time
            // Must use -> as Cache came in as a pointer

        float temperature = generate_temp();  // A random temperature in range
        Cache->timestamp_array[index_count] = time(NULL);  // Store the timestamp
        Cache->temperature_array[index_count] = temperature;  // Store the generated temp
        Cache->coordinate_array[index_count][0] = rand() % COLUMNS;  // Get the coordinates stored, in X, Y orientation
        Cache->coordinate_array[index_count][1] = rand() % ROWS;

        // Increment the counter variable, wrapping around when it goes over
        index_count++;
        index_count = index_count % SATELLITE_CACHE; // Make sure it doesn't go above the limit

    }

}

void server(struct Sat_Cache *Cache)
{
    // The server will write to the processing variable
    // It will only read from the coordinate array, temperature array, and the timestamp array.

    printf("Server\n");
    for (int counter = 0; counter < ITERATIONS; counter++)
    {                     // This is the mainloop
        usleep(INTERVAL); // Sleep for a set time
        printf("Data: %d, %d.  %f.  %u\n", Cache->coordinate_array[0][0], Cache->coordinate_array[0][1], Cache->temperature_array[0], Cache->timestamp_array[0]);

    }
    Cache->process = false;  // Tell the other thread to stop
}


// Node code

// General code
float generate_temp()
{
    float random = (float)rand() / SPLITTER;      // random number, made into a float
    float modded = fmod(random, MOD_DENOMINATOR); // Put it in the appropriate range
    return modded + MIN_TEMP;
}
