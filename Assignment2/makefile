build-assignment: ass2.c
	clear
	mpicc ass2.c -o ass2_out -lm

run-assignment: ass2.c
	clear
	mpirun -oversubscribe -np 21 ass2_out 4 5 1000

show-assignment: ass2.c
	clear
	mpicc ass2.c -o ass2_out -lm
	mpirun -oversubscribe -np 21 ass2_out 4 5 1000