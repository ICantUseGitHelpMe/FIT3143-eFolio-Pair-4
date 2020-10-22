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
    if (rank == 0)
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
    server();  // Activate the server
    pthread_join(thread_id, NULL);
    return 0;
}

void satellite()
{
    
    int index_count = 0;  // This keeps track of the index of the memory array
    int DEBUG = 0;
    unsigned timestamp_array[SATELLITE_CACHE];  // Stores the previous N timestamps
    float temperature_array[SATELLITE_CACHE];   // Stores the previous N temperatures (tmeperature 2 corresponds to timestamp 2, etc.)
    int coordinate_array[SATELLITE_CACHE][2];   // Stores the previous N coordinates associated with the above information

    // Will send with tag 1 and receive tag 0, to node 0 (us)
    printf("Satellite!\n");
    for (int counter = 0; counter < ITERATIONS; counter++)
    {
        float temperature = generate_temp();
        if (temperature > THRESHOLD){
            timestamp_array[index_count] = time(NULL);
            temperature_array[index_count] = temperature;
            coordinate_array[index_count][0] = DEBUG;
            coordinate_array[index_count][1] = DEBUG++;
            index_count ++;
            index_count = index_count % SATELLITE_CACHE;  // Make sure it doesn't go above the limit
        }
        printf("Temperature: %f\n", temperature);
        usleep(INTERVAL);
    }

    char readable_timestamp[100];
    for (int index = 0; index < SATELLITE_CACHE; index ++){
        struct tm tolocal;
        time_t cast_time = (time_t) timestamp_array[index];
        tolocal = *localtime(&cast_time);
        strftime(readable_timestamp, sizeof(readable_timestamp), "%Y-%m-%d %H:%M:%S %Z", &tolocal);
        printf("Temperature: %f, Time: %s, Coords: %d, %d\n", temperature_array[index], readable_timestamp, coordinate_array[index][0], coordinate_array[index][1]);
    }

}
void server()
{
    // Will send with tag 0 and receive tag 1, to node 0 (us)
    for (int counter = 0; counter < 100; counter++)
    {
        printf("Server!\n");
        usleep(INTERVAL);
    }
}

// Node code

// General code
float generate_temp()
{
    float random = (float) rand() / SPLITTER;                  // random number, made into a float
    float modded = fmod(random, MOD_DENOMINATOR); // Put it in the appropriate range
    return modded + MIN_TEMP;
}
