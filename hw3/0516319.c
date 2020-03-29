#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#ifndef W
#define W 20 // Width
#endif
int main(int argc, char **argv) {
	int my_rank, p;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	int L = atoi(argv[1]); // Length
	int iteration = atoi(argv[2]); // Iteration
	srand(atoi(argv[3])); // Seed
	float d = (float) random() / RAND_MAX * 0.2; // Diffusivity
	int *temp = malloc(L*W*sizeof(int)); // Current temperature
	int *next = malloc(L*W*sizeof(int)); // Next time step
	
 	int len = L / p;
	for (int i = 0; i < L; i++) {
		for (int j = 0; j < W; j++) {
			temp[i*W+j] = random()>>3;
		}
	}
	int count = 0, balance = 0;
	while (iteration--) { // Compute with up, left, right, down points
		balance = 1;
		count++;
		for (int i = my_rank * len; i < (my_rank + 1) * len; i++) {
			for (int j = 0; j < W; j++) {
				float t = temp[i*W+j] / d;
				t += temp[i*W+j] * -4;
				t += temp[(i - 1 < 0 ? 0 : i - 1) * W + j];
				t += temp[(i + 1 >= L ? i : i + 1)*W+j];
				t += temp[i*W+(j - 1 < 0 ? 0 : j - 1)];
				t += temp[i*W+(j + 1 >= W ? j : j + 1)];
				t *= d;
				next[i*W+j] = t ;
				if (next[i*W+j] != temp[i*W+j]) {
					balance = 0;
				}
			}
		}
		//exchange balance between four machine
		if(my_rank != 0){
			MPI_Send(&balance, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Recv(&balance, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		}
		else{
			int balance_temp;
			for(int i = 1;i < p;i++){
				MPI_Recv(&balance_temp, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
				balance = balance & balance_temp;
			}
			for(int i = 1;i < p;i++){
				MPI_Send(&balance, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			}
		}

		if (balance) {
			break;
		}

		//exchange vector between four machine
		if(my_rank != 0){//machine 1 2 3 to 0 1 2
			MPI_Send(next + my_rank * W * len, W, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD);
			MPI_Recv(next + (my_rank * len - 1) * W, W, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &status);
		}
		if(my_rank != p - 1){//machine 0 1 2 to 1 2 3 
			MPI_Recv(next + (my_rank + 1)* len * W, W, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &status);
			MPI_Send(next + ((my_rank + 1) * len - 1) * W, W, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD);
		}
		int *tmp = temp;
		temp = next;
		next = tmp;
	}
	int min = temp[my_rank * len * W];
	for (int i = my_rank * len; i < (my_rank + 1) * len; i++) {
		for (int j = 0; j < W; j++) {
			if (temp[i*W+j] < min) {
				min = temp[i*W+j];
			}
		}
	}


	if(my_rank == 0){
		int min_tmp;
		for(int i = 1; i < p; i++){
			MPI_Recv(&min_tmp, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
			if(min_tmp < min)min = min_tmp;
		}
		printf("Size: %d*%d, Iteration: %d, Min Temp: %d\n", L, W, count, min);
	}
	else{
		MPI_Send(&min, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	
	return 0;
}
