#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

int server_control();
void satellite();
void server();

int main(int argc, char **argv) {
    // First, split between the server and the nodes
    int rank, size;
    MPI_Comm new_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_split( MPI_COMM_WORLD,rank == size-1, 0, &new_comm);


    //Server Controls:
    if (rank == 0){
        server_control();
    }
    // Node controls
    else{ 

    }
    MPI_Finalize();
    return 0;
}

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

    printf("Server!\n");
}

// int master_io(MPI_Comm world_comm, MPI_Comm comm){
//     int size, nslaves;
//     MPI_Comm_size(world_comm, &size );
//     nslaves = size - 1;
//     pthread_t tid;
//     pthread_create(&tid, 0, ProcessFunc, &nslaves); // Create the thread
//     pthread_join(tid, NULL); // Wait for the thread to complete.
//     return 0;