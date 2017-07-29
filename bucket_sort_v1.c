#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>

//////////QuickSort Stuff

// Comparison function used by qsort
int compare_dbls(const void* arg1, const void* arg2){
 double a1 = *(double *) arg1;
 double a2 = *(double *) arg2;
 if (a1 < a2) return -1;
 else if (a1 == a2) return 0;
 else return 1;
}

// Sort the array in place
void qsort_dbls(double *array, int array_len){
 qsort(array, (size_t)array_len, sizeof(double), compare_dbls);
} 

////////////////

//Function to find what bucket a double belongs to based
//off how many processors there are
int find_bucket(double num, int p_num){
  int x;
  for(x=1; x < p_num+1; x++){
	double bucket_range =(double) x / (double)p_num;
	if(num <= bucket_range){
	  return x - 1; //return bucket number
	}
  }

}

int main(int argc, char *argv[]){
	int myrank, P;
	int sub_count[1];

	if(argc != 2){
		printf("\nPlease include N, problem size\n");
		return 0;
	}

	//Allocate Arrays	
	int N = strtol(argv[1], NULL, 10);
	int *count = (int*)malloc(P*sizeof(int));
	double *list = (double*)malloc(N*sizeof(double));
	int *displs = (int*)malloc(P*sizeof(int));
	double *dist_list = (double*)malloc(N*sizeof(double));
	int *index = (int*)malloc(P*sizeof(int));

	//Init MPI, get process # and ranks
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &P);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);	

	//double t1 = MPI_Wtime();

	//Root
	if(myrank == 0){

		//Generate Array w/ random numbers, and counts of #s for each bucket
		int i;
		double r;
		for(i = 0; i < N; i++){

			r = (double)rand() / (double) RAND_MAX;

			list[i] = r;
			//Determine bucket count to increase
			int bucket = find_bucket(r, P);
			count[bucket]++;
		}

		//Bucket Counts for Debugging
		//printf("\nBUCKET COUNTS:\n");
		//for(i = 0; i < P; i++){
		//	printf("\nBUCKET %d, Count: %d\n", i, count[i]);
		//}
	}

	//double t2 = MPI_Wtime();

	//Scatter Bucket Counts
	MPI_Scatter(count, 1, MPI_INT, sub_count, 1, MPI_INT, 0, MPI_COMM_WORLD );
			
	//Allocate arrays based on counts
	double *bucket_list = (double*)malloc(sub_count[0]*sizeof(double));

	//Distribute list to other processes
	if(myrank == 0){

		int j;

		//Create Displacements for scatterv & gatherv
		displs[0] = 0;
		for(j = 1; j < P; j++){
			displs[j] = count[j-1] + displs[j-1];
		}


		for(j = 0; j < N; j++){
			//Find bucket for double
			int bucket = find_bucket(list[j], P);
			//Place double in list
			dist_list[displs[bucket] + index[bucket]] = list[j];
			//update index
			index[bucket]++;
		}
		free(list);
		free(index);
	}

	//double t3 = MPI_Wtime();

	//Scatter Bucket-sorted list using scatterv
	MPI_Scatterv(dist_list, count, displs, MPI_DOUBLE, bucket_list, sub_count[0], MPI_DOUBLE, 0, MPI_COMM_WORLD);

	//double t4 = MPI_Wtime();

	//Do Quicksort on each list locally
	qsort_dbls(bucket_list, sub_count[0]);

	double t5 = MPI_Wtime();

	//Gather all lists at root 
	MPI_Gatherv(bucket_list,sub_count[0], MPI_DOUBLE, dist_list, count, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD );

	double t6 = MPI_Wtime();

	free(bucket_list);

	//Check Result
	if(myrank == 0){
		int sorted = 1;
		int k;
		for(k = 0; k < N - 2; k++){
			if(dist_list[k] > dist_list[k+1]){
				sorted = 0;
			}
		}

		if(sorted == 1){
			printf("\nSORTING CORRECT\n");
		}else{
			printf("\nSORTING NOT CORRECT\n");
		}
		
		free(displs);
		free(count);
		free(dist_list);
	//printf("\nTime to Generate: %f\n", t2-t1);
	//printf("\nTime to Bin: %f\n", t3-t2);
	//printf("\nTime to Distribute: %f\n", t4-t3);
	//printf("\nTime to Sort: %f\n", t5-t4);
	printf("\nTime to Gather: %f\n", t6-t5);
	//printf("\nTotal Execution Time: %f\n", t6-t1);

	}

	MPI_Finalize();

}
