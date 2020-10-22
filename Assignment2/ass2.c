#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#define INTERVAL 100 * 1000  // How frequently the main processor will check for updates from nodes (in microseconds)
#define SPLITTER 157  // Divide random numbers by this to make them floats.  This number is a prime
#define THRESHOLD 80  // The temperature that evokes a positive response (degrees)
#define MOD_DENOMINATOR 60 // The number the generated temperature is modded by.  Determines the upper range of the generated temperature
#define MIN_TEMP 25  // The baseline temperature.  
// Server headers
int server_control();
void satellite();
void server();

// Node headers


// General headers
float generate_temp();  // Generate a random temperature

// Main loop
int main(int argc, char **argv) {
    // First, split between the server and the nodes
    // Standard boilerplate 
    int rank, size;
    MPI_Comm new_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_split( MPI_COMM_WORLD,rank == size-1, 0, &new_comm);
    srand((unsigned) time(NULL) + rank);  // Seed the random, uniquely for each thread

    //Server Controls:
    if (rank == 0){
        server_control();
    }
    // Node controls
    else{ 
        // Philip, start your work here and reference task 1 and 2 of Lab 10
    }

    // Rejoin everything together again
    MPI_Finalize();
    return 0;
}

// Server code
int server_control(){
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, (void *) satellite,  NULL);  // Activate the satellite
    server();   // Activate the server
    pthread_join(thread_id, NULL);
    return 0;
}

void satellite(){
    // Will send with tag 1 and receive tag 0, to node 0 (us)
    printf("Satellite!\n");
}
void server(){
    // Will send with tag 0 and receive tag 1, to node 0 (us)
    for (int counter = 0; counter < 100; counter ++){
        printf("Server!\n");
        usleep(INTERVAL);
    }
}

// Node code

// General code
float generate_temp(){
    float random = rand() / 157;  // random number, made into a float
    float modded = random % MOD_DENOMINATOR;  // Put it in the appropriate range
    return modded + MIN_TEMP;
}
