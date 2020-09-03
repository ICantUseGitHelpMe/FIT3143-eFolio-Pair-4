#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

struct valuestruct {
    int a;
    double b;
};

int main(int argc, char *argv[]){
    struct valuestruct values;  // Prepare the struct with MPI data types
	int myrank, size;

    MPI_Datatype Valuetype;
    MPI_Datatype type[2] = { MPI_INT, MPI_DOUBLE };
    int blocklen[2] = { 1, 1};
    MPI_Aint disp[2];

	int i;
	char c[100];
	char buffer[110];
	int position = 0;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_myrank(MPI_COMM_WORLD, &myrank);

    MPI_Get_address(&values.a, &disp[0]);
    MPI_Get_address(&values.b, &disp[1]);
    //Make relative
    disp[1]=disp[1]-disp[0];
    disp[0]=0;
    // Create MPI struct
    MPI_Type_create_struct(2, blocklen, disp, type, &Valuetype);
    MPI_Type_commit(&Valuetype);

	if(size < 2)
	{
		printf("Please run with 2 processes.\n");
		fflush(stdout);
		MPI_Finalize();
		return 0;
	}

	if(myrank == 0)
	{
		for (i=0; i<100; i++)
		    c[i] = i;
		i = 123;
		MPI_Pack(&i, 1, MPI_INT, buffer, 110, &position, MPI_COMM_WORLD);
		//MPI_Send(buffer, position, MPI_PACKED, 1, 100, MPI_COMM_WORLD);
	}

	if(myrank == 1)
	{
		MPI_Recv(buffer, 110, MPI_PACKED, 0, 100, MPI_COMM_WORLD, &status);
		MPI_Unpack(buffer, 110, &position, &i, 1, MPI_INT, MPI_COMM_WORLD);
		MPI_Unpack(buffer, 110, &position, c, 100, MPI_CHAR, MPI_COMM_WORLD);
		printf("i=%d\nc[0] = %d\n...\nc[99] = %d\n", i, (int)c[0], (int)c[99]);
		fflush(stdout);
	}

	MPI_Finalize();
	return 0;
}