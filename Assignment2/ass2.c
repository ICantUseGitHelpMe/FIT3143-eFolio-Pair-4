/* FIT3143 - Parallel Computing 
 * Lab Time         Tuesday 18:00-20:00
 * Lab ID           6
 * Pair Number      4
 * Group Members    Philip Chen 	27833725
 *                  Ethan Nardella	29723299
 * --------------------------------------------------
 * Assignemnt 2
 * mpicc assignment2.c -o ass2_out -lm
 * mpirun -oversubscribe -np 21 ass2_out 4 5
 *
 *  README - this code currently hits deadlock when getting adjacent node temperatures
 *      it happens because of MPI_Wait when trying to receive neighbour temperatures
 *      on line 182 (it waits for neighbour temperature, but neighbour might be waiting 
 *      for another temperature and so is unable to receive the request to send its own 
 *      temperature) - I tried to use a variable to lock the process so that one node 
 *      does this at a time, but no luck yet :/ I tried to use broadcast to update that 
 *      variable. You the information is packaged and sent on line 197, 198 (feel free to comment that in or out when working with it~)
 */

/* Gets the neighbors in a cartesian communicator
* Orginally written by Mary Thomas
* - Updated Mar, 2015
* Link: https://edoras.sdsu.edu/~mthomas/sp17.605/lectures/MPICart-Comms-and-Topos.pdf
* Minor modifications to fix bugs and to revise print output
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#define SHIFT_ROW 0
#define SHIFT_COL 1
#define DISP 1
#define ITERATIONS 100        // The number of loops in the server and satellite
#define INTERVAL 10 * 1000    // How frequently the main processor will check for updates from nodes (in microseconds)
#define SPLITTER 157          // Divide random numbers by this to make them floats.  This number is a prime
#define THRESHOLD 80.0f       // The temperature that evokes a positive response (degrees)
#define MOD_DENOMINATOR 60.0f // The number the generated temperature is modded by.  Determines the upper range of the generated temperature
#define MIN_TEMP 25.0f        // The baseline temperature.
#define SATELLITE_CACHE 500   // The number of elements the satellite will keep in its memory
#define ROWS 2                // The number of rows of nodes
#define COLUMNS 2             // The number of columns of nodes
#define SERVER_ID 0           // The rank of the server node
#define TOLERANCE 5           // The difference temperatures can be apart while being considered as similar
#define GET_TEMPS 1           // The tag for a node asking for its neighbours' temperatures
#define GIVE_TEMPS 2          // The tag for a node giving its temperature
#define REPORT_BASE 3         // The tag for a node reporting to base
#define SHUTDOWN 10           // The tag the server uses to disable all nodes and finalise operation
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
struct Node_Report // The structure of the satellite cache
{
    struct timespec timestamp;
    int *coordinates;
    float node_temperature;
    float *neighbor_temperatures;
};

int request_temp(int requester_rank, int target_rank, MPI_Request node_send_request, MPI_Request node_recv_request, MPI_Status node_status);

// General headers
float generate_temp(); // Generate a random temperature

int main(int argc, char *argv[])
{
    // Initialize environment variables for cartesian topology and use of OpenMPI
    int ndims = 2, size, rank, reorder, my_cart_rank, ierr;
    int nrows, ncols, nbr_i_lo, nbr_i_hi, nbr_j_lo, nbr_j_hi;
    int dims[ndims], coord[ndims], wrap_around[ndims];
    int wsn_size, base_station_id, node_request_exclusive_lock;
    int pack_size;
    char package_buffer[100];
    struct Node_Report report;
    MPI_Datatype Node_Report;
    MPI_Request request;
    MPI_Comm comm2D;

    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    wsn_size = size - 1;

    // Handle command line inputs
    if (argc == 3)
    {
        nrows = atoi(argv[1]);
        ncols = atoi(argv[2]);
        dims[0] = nrows; // Number of rows
        dims[1] = ncols; // Number of columns
        base_station_id = nrows * ncols;
        if (base_station_id + 1 != size)
        { // Check that number of rows * number of columns + 1 = size (extra 1 for base station)
            if (rank == base_station_id)
                printf("ERROR: nrows*ncols)=%d * %d = %d != %d\n", nrows, ncols, nrows * ncols, size);
            MPI_Finalize();
            return 0;
        }
    }
    else
    {
        nrows = ncols = (int)sqrt(size);
        dims[0] = dims[1] = 0;
    }

    // Create cartesian topology
    MPI_Dims_create(wsn_size, ndims, dims);
    // Cartesian mapping
    wrap_around[0] = wrap_around[1] = 0;
    reorder = 1;
    ierr = 0;
    ierr = MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, wrap_around, reorder, &comm2D);
    if (ierr != 0)
        printf("ERROR[%d] creating CART\n", ierr);
    MPI_Request node_send_requests[base_station_id], node_recv_requests[base_station_id];
    MPI_Status node_status[base_station_id];
    node_request_exclusive_lock = base_station_id;
    srand(time(NULL) + rank);  // Seed the random with a unique value
    if (rank == base_station_id)
    {
        printf("I am base station - rank:%d. Comm Size: %d: Grid Dimension = [%d x %d] \n", rank, size, dims[0], dims[1]);

        server_control();
    }
    else
    {
        // Get current coordinates
        MPI_Cart_coords(comm2D, rank, ndims, coord);
        // Use current coordinates to find cartesian rank
        MPI_Cart_rank(comm2D, coord, &my_cart_rank);
        // Get neighbours
        /* axis=0 ==> shift along the rows: P[my_row-1]: P[me] : P[my_row+1] 
         * axis=1 ==> shift along the columns P[my_col-1]: P[me] : P[my_col+1] */
        MPI_Cart_shift(comm2D, SHIFT_ROW, DISP, &nbr_i_lo, &nbr_i_hi);
        MPI_Cart_shift(comm2D, SHIFT_COL, DISP, &nbr_j_lo, &nbr_j_hi);

        // Do WSN Node stuff here
        printf("Global rank: %d. Cart rank: %d. Coord: (%d, %d). Left: %d. Right: %d. Top: %d. Bottom: %d\n", rank, my_cart_rank, coord[0], coord[1], nbr_j_lo, nbr_j_hi, nbr_i_lo, nbr_i_hi);

        int neighbor_ranks[4];
        neighbor_ranks[0] = nbr_i_lo; // Top neighbor
        neighbor_ranks[1] = nbr_j_hi; // Right neighbor
        neighbor_ranks[2] = nbr_i_hi; // Bottom neighbor
        neighbor_ranks[3] = nbr_j_lo; // Left neighbor

        do
        {
            // Get current time
            struct timespec t_spec;
            clock_gettime(CLOCK_REALTIME, &t_spec);
            unsigned long now = (t_spec.tv_nsec + t_spec.tv_sec * 1e9) * 1e-6; // This converts from nanoseconds to miliseconds

            // Generate node temperature
            float node_temperature = generate_temp();
            float rank_temperature_data[4] = {0};
            rank_temperature_data[0] = node_temperature;
            // printf("[Rank %d]   Sensor: (%d, %d) Temperature: %f \n", rank, coord[0], coord[1], node_temperature);

            // printf("[Rank %d]   node_request_exclusive_lock allocated to %d\n", rank, node_request_exclusive_lock);
            if (node_temperature > THRESHOLD)
            {
                int notify_base[4] = {0, 0, 0, 0};
                float neighbor_temperatures[4] = {-1, -1, -1, -1};

                node_request_exclusive_lock = rank;
                // MPI_Bcast(&node_request_exclusive_lock, 1, MPI_INT, rank, MPI_COMM_WORLD);
                // printf("[Rank %d]   node_request_exclusive_lock ALLOCATED to %d\n", rank, node_request_exclusive_lock);
                // printf("[Rank %d]   node_request_exclusive_lock allocated to %d\n", rank, node_request_exclusive_lock);

                // printf("[Rank %d]   Sending requests to neighbouring ranks...\n", rank);

                MPI_Request node_recv_requests[4]; // Requests for all the receives

                int valid_neighbours = 0; //  Increment this by one for eah valid neighbour found; to be used as an index

                for (int iterator = 0; iterator < 4; ++iterator)
                {
                    int target_rank = neighbor_ranks[iterator];
                    if (target_rank != -2)
                    {
                        // printf("[Rank %d]   Sending request to rank %d\n", rank, target_rank);
                        float target_rank_temp;
                        MPI_Request node_send_request;

                        int info = 1; // The data we send
                        MPI_Request ask_temp;
                        MPI_Isend(&info, 1, MPI_INT, target_rank, GET_TEMPS, MPI_COMM_WORLD, &ask_temp);

                        MPI_Irecv(&neighbor_temperatures[valid_neighbours], 1, MPI_DOUBLE, target_rank, GIVE_TEMPS, MPI_COMM_WORLD, &node_recv_requests[valid_neighbours]);
                        valid_neighbours++; // Increment as we have found a target (MUST BE AFTER IRECV)
                    }
                }

                usleep(INTERVAL); // Sleep so it doesn't rapid-fire generate temperatures.  Used here, it doubles as a timeout
                // MPI_TEST HERE
                int lost_values = 0; // Stores the number of timed out requests
                // printf("[Rank %d]   SAbout to almost send\n", rank);
                printf("[Rank %d]   pre recv\n", rank);

                for (int i = 0; i < valid_neighbours; i++)
                {
                    int is_complete = 0;
                    printf("[Rank %d]   pre test\n", rank);
                    //TODO: why this crash
                    MPI_Test(&node_recv_requests[i], &is_complete, MPI_STATUS_IGNORE); // Check if each response has come in
                    printf("[Rank %d]   checking biz %d\n", rank, is_complete);

                    if (is_complete == 0)
                    {
                        // Destroy the request (timeout)
                        MPI_Cancel(&node_recv_requests[i]);
                        MPI_Request_free(&node_recv_requests[i]);
                    }
                    lost_values += 1 - is_complete; // +1 if a fail, +0 on success
                }
                printf("[Rank %d]   lost vals %d\n", rank, lost_values);

                if (lost_values > 0)
                {
                    continue; // Timeout, skip
                }
                printf("[Rank %d]   SsssssAbout to almost send\n", rank);

                int num_found = 0; // Notify base if this is two or more
                for (int i = 0; i < valid_neighbours; i++)
                {
                    int target_rank_temp = neighbor_temperatures[i];
                    int temperature_difference = abs((target_rank_temp - node_temperature));
                    if (temperature_difference <= TOLERANCE)
                    {
                        num_found++; // Within range
                    }
                }
                // Notify base
                // struct Node_Report report = {t_spec, coord, node_temperature, neighbor_temperatures};

                /* Send:  
                    Time
                    X
                    Y
                    Temp
                    NeighbourT1
                    NeighbourT2
                    NeighbourT3
                    NeighbourT4
                */
                printf("[Rank %d]   Sending to chief\n", rank);
                double send_buf[8];
                // printf("___________________________\n");

                send_buf[0] = now;
                // printf("%ld\n", now);
                send_buf[1] = coord[0];
                // printf("%d\n", coord[0]);

                send_buf[2] = coord[1];
                // printf("%d\n", coord[1]);

                send_buf[3] = node_temperature;
                // printf("%f\n", node_temperature);

                send_buf[4] = neighbor_temperatures[0];
                // printf("%f\n", neighbor_temperatures[0]);

                send_buf[5] = neighbor_temperatures[1];
                // printf("%f\n", neighbor_temperatures[1]);

                send_buf[6] = neighbor_temperatures[2];
                // printf("%f\n", neighbor_temperatures[2]);

                send_buf[7] = neighbor_temperatures[3];
                // printf("%f\n", neighbor_temperatures[3]);

                MPI_Request sender;
                MPI_Isend(send_buf, 8, MPI_DOUBLE, base_station_id, REPORT_BASE, MPI_COMM_WORLD, &sender);
                // MPI_Wait(&node_send_requests[rank], MPI_STATUS_IGNORE);
                printf("[Rank %d]   Sent to chief\n", rank);
                // node_request_exclusive_lock = base_station_id;
                // MPI_Bcast(&node_request_exclusive_lock, 1, MPI_INT, rank, MPI_COMM_WORLD);
                // printf("[Rank %d]   node_request_exclusive_lock reset to %d\n", rank, node_request_exclusive_lock);
            }

            for (int iterator = 0; iterator < 4; ++iterator)
            {
                int target_rank = neighbor_ranks[iterator];
                int recv = 0;
                // MPI_Irecv( buffer, 1, MPI_INT, target_rank, 1, MPI_COMM_WORLD, &request);
                MPI_Request test;
                MPI_Irecv(&recv, 1, MPI_INT, target_rank, GET_TEMPS, MPI_COMM_WORLD, &test);
                usleep(INTERVAL / 8);
                int check = 0;
                MPI_Test(&test, &check, MPI_STATUS_IGNORE);

                if (!check)
                {
                    MPI_Cancel(&test);
                    MPI_Request_free(&test);
                    continue;
                }

                // MPI_Wait(&test, MPI_STATUS_IGNORE);
                if (recv == 1)
                {
                    // printf("[Rank %d]   Sending my temperature: %f\n", rank, rank_temperature_data[0]);
                    MPI_Request send_temp;

                    MPI_Isend(rank_temperature_data, 1, MPI_DOUBLE, target_rank, GIVE_TEMPS, MPI_COMM_WORLD, &send_temp);
                    // MPI_Wait(&node_requests[target_rank], &node_status[target_rank]);
                }
            }
        } while (true);
        MPI_Comm_free(&comm2D);

        fflush(stdout);
    }

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
        // MPI_Request sender;
        // MPI_Isend(send_buf, 8, MPI_DOUBLE, base_station_id, REPORT_BASE, MPI_COMM_WORLD, &sender);
        MPI_Request check_logs;
        double recv_buf[8];
        MPI_Irecv(recv_buf, 8, MPI_DOUBLE, MPI_ANY_SOURCE, REPORT_BASE, MPI_COMM_WORLD, &check_logs);
        usleep(INTERVAL); // Sleep for a set time, as per the spec.  Also use this as a timeout
        /* Receive:  
            Time
            X
            Y
            Temp
            NeighbourT1
            NeighbourT2
            NeighbourT3
            NeighbourT4
        */

        int check = 0;
        MPI_Test(&check_logs, &check, MPI_STATUS_IGNORE);

        if (!check)
        {
            MPI_Cancel(&check_logs);
            MPI_Request_free(&check_logs);
            continue;
        }

        int node_x = (int)recv_buf[1]; // The coords of the reporting node
        int node_y = (int)recv_buf[2];
        float temperature = (float)recv_buf[3];               // The temperatureo reported by the node
        unsigned long timestamp = (unsigned long)recv_buf[0]; // TODO: Mention the assumed clock sync between processes in report

        printf("Server RECEIVED\n");
        // Now, check if there is any entry from the satellite that matches the node coords
        // To do this, loop backwards through the array starting from the current "head", then restarting when the end is reached
        // This ensures we take the latest first

        // TODO: Assign these variables through MPI_IRECV / RECV
        // int node_x = 0; // The coords of the reporting node
        // int node_y = 0;
        // float temperature = 86.3f;               // The temperatureo reported by the node
        // unsigned long timestamp = 1603625547246; // TODO: Mention the assumed clock sync between processes in report

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
int request_temp(int requester_rank, int target_rank, MPI_Request node_send_request, MPI_Request node_recv_request, MPI_Status node_status)
{
    int buffer[4] = {0}, requested_temp, probe_flag;
    float requested_temp_data[4] = {0};
    buffer[0] = 1;
    printf("[Rank %d]   ISend buffer output %d %d %d %d\n", requester_rank, buffer[0], buffer[1], buffer[2], buffer[3]);
    MPI_Isend(buffer, 1, MPI_INT, target_rank, 1, MPI_COMM_WORLD, &node_send_request);
    // MPI_Wait(&node_request, &node_status);

    // MPI_Iprobe(requester_rank, 1, MPI_COMM_WORLD, &probe_flag, &node_status);
    // printf("[Rank %d]   Iprobe flag %d\n", requester_rank, probe_flag);

    MPI_Irecv(requested_temp_data, 1, MPI_DOUBLE, target_rank, 0, MPI_COMM_WORLD, &node_recv_request);
    MPI_Wait(&node_recv_request, MPI_STATUS_IGNORE);
    printf("[Rank %d]   Irecv data output %f %f %f %f\n", requester_rank, requested_temp_data[0], requested_temp_data[1], requested_temp_data[2], requested_temp_data[3]);
    requested_temp = requested_temp_data[0];

    return requested_temp;
}

// General code
float generate_temp()
{
    float random = (float)rand() / SPLITTER;      // random number, made into a float
    float modded = fmod(random, MOD_DENOMINATOR); // Put it in the appropriate range
    return modded + MIN_TEMP;
}