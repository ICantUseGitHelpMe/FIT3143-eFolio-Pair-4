/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Lab 10 - Task 2 
 * mpicc task2.c -o task2_out
 * mpirun -np 4 task2_out
 */

 // Sample solution focuses on the thread function and master_io function.
void* ProcessFunc(void *pArg){ // Common function prototype
    int i = 0, size, nslaves, firstmsg;
    char buf[256], buf2[256];
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &size );
    int* p = (int*)pArg;
    nslaves = *p;
    while (nslaves > 0) {
        MPI_Recv(buf, 256, MPI_CHAR, MPI_ANY_SOURCE,
        MPI_ANY_TAG, MPI_COMM_WORLD, &status );
        switch (status.MPI_TAG) {
            case MSG_EXIT: nslaves--; break;
            case MSG_PRINT_UNORDERED:
            printf("Thread prints: %s", buf);
            fflush(stdout);
            break;
            case MSG_PRINT_ORDERED:
            firstmsg = status.MPI_SOURCE;
            for (i=0; i<size-1; i++) {
                if (i == firstmsg){
                    printf("Thread prints: %s", buf);
                    fflush(stdout);
                }
                else {
                    MPI_Recv( buf2, 256, MPI_CHAR, i,
                    MSG_PRINT_ORDERED, MPI_COMM_WORLD, &status );
                    printf("Thread prints: %s", buf2);
                    fflush(stdout);
                }
            }
            break;
        }
    }
    return 0;
}

int master_io(MPI_Comm world_comm, MPI_Comm comm){
    int size, nslaves;
    MPI_Comm_size(world_comm, &size );
    nslaves = size - 1;
    pthread_t tid;
    pthread_create(&tid, 0, ProcessFunc, &nslaves); // Create the thread
    pthread_join(tid, NULL); // Wait for the thread to complete.
    return 0;
}
