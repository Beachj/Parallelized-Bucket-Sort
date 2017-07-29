#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
int find_bucket(double num, int p_num,  double pivots[]){
  int x;
  for(x = 0; x < p_num; x++){
	double bucket_range = pivots[x];
	//printf("Comparing %f to Range %f\n", num, bucket_range);
	if(num <= bucket_range){
		//printf("\nPut in bucket: %d\n", x);
	  return x; //return bucket number
	}
  }

}

//Bucket Sort - V3, with pivots for dealing with non-uniform distribution

int main(int argc, char *argv[]){
	int myrank, P;
	int sub_count[1];
	//double* pivots = (double*)malloc(P*sizeof(double));

	if(argc != 2){
		printf("\nPlease include N, problem size\n");
		return 0;
	}

	//Allocate Arrays	
	int N = strtol(argv[1], NULL, 10);

	//Init MPI, get process # and ranks
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &P);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);	

	//double t1 = MPI_Wtime();	

	//printf("\nP: %d\n", P);
	
	int S;
	S = 12*P*(log(N));
	int ele = S/P;
	int i;
	double tmp;
	double* ele_Arr = (double*)malloc(ele*sizeof(double));
	for(i = 0; i < ele; i++){
		tmp = (double)rand() / (double) RAND_MAX;
		tmp = tmp * tmp;
		ele_Arr[i] = tmp;
	}

	double *S_Arr = (double*)malloc(S*sizeof(double));

	//Gather S Samples At root process
	MPI_Gather(ele_Arr, ele, MPI_DOUBLE, S_Arr, ele, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	double* pivots = (double*)calloc(P, sizeof(double));

	if(myrank == 0){
		qsort_dbls(S_Arr, S);

		//Select Pivots
		for(i = 1; i < P; i++){
			pivots[i-1] = S_Arr[i * ele];
			//printf("\nPivot %d: %f\n", i-1, pivots[i-1]);
		}
		pivots[P-1] = 1.0;
	}
	MPI_Bcast( pivots, P, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	//double t2 = MPI_Wtime();
	double t1 = MPI_Wtime();
	//for(i = 0; i < P; i++){
	//printf("\nPIVOT %d: %f\n", i, pivots[i]);
//}

	//Generate N/P Elements on each processor 
	//Generate Array w/ random numbers, and counts of #s for each bucket
	int numpp = N/P;
	double *list = (double*)malloc(numpp*sizeof(double));
	int *count = (int*)calloc(P, sizeof(int));
	int bucket;
	double r;
	for(i = 0; i < numpp; i++){

		r = (double)rand() / (double) RAND_MAX;
		r = r*r;
		list[i] = r;
		//Determine bucket count to increase
		bucket = find_bucket(r, P, pivots);
		count[bucket]++;
		//printf("Count of bucket %d: %d, ", bucket, count[bucket]);
	}

	int *bucket_count = (int*)malloc(P*sizeof(int));
	MPI_Alltoall(count, 1, MPI_INT, bucket_count, 1, MPI_INT, MPI_COMM_WORLD);
	
	int loc_bcount = 0;
	//Add together counts
	for(i = 0; i < P; i++){
		loc_bcount+= bucket_count[i]; 
	}

	//Bucket Counts for Debugging
//	printf("\nBUCKET %d, Count: %d\n", myrank, loc_bcount);

	//Allocate arrays based on counts
	double *bucket_list = (double*)malloc(loc_bcount*sizeof(double));

	//Distribute list to other processes
	
	//Allocate arrays required for distribution
	int *displs = (int*)malloc(P*sizeof(int));
	double *dist_list = (double*)malloc(numpp*sizeof(double));
	int *index = (int*)calloc(P,sizeof(int));

	//Create Displacements for scatterv & gatherv
	//Send displacements
	displs[0] = 0;
	for(i = 1; i < P; i++){
		displs[i] = count[i-1] + displs[i-1];
	}

	//Receive displacements
	int *rdispls = (int*)malloc(P*sizeof(int));
	rdispls[0] = 0;
	for(i = 1; i < P; i++){
		rdispls[i] = bucket_count[i-1] + rdispls[i-1];
	}
	for(i = 0; i < numpp; i++){
		//Find bucket for double
		bucket = find_bucket(list[i], P, pivots);
		//Place double in list
		dist_list[displs[bucket] + index[bucket]] = list[i];
		//update index
		index[bucket]++;
	}
	free(list);

	MPI_Alltoallv(dist_list, count, displs, MPI_DOUBLE, bucket_list, bucket_count, rdispls, MPI_DOUBLE, MPI_COMM_WORLD); 	

	//Do Quicksort on each list locally
	qsort_dbls(bucket_list, loc_bcount);

	//Gather counts of each bucket to root
	int gathercounts[1];
	gathercounts[0] = loc_bcount;
	MPI_Gather(gathercounts, 1, MPI_INT, count, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//Count at root now holds size each bucket
	//
	if(myrank==0){
	displs[0] = 0;
	for(i = 1; i < P; i++){
		displs[i] = count[i-1] + displs[i-1];
	}
	}
	
	double* final_list = (double*)malloc(N*sizeof(double));
	//Gather all lists at root 
	MPI_Gatherv(bucket_list,loc_bcount, MPI_DOUBLE, final_list, count, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD );

	double t2 = MPI_Wtime();

	//Check Result
	if(myrank == 0){
		int sorted = 1;
		int k;
		for(k = 0; k < N - 2; k++){
			if(final_list[k] > final_list[k+1]){
				sorted = 0;
			}
		}

		if(sorted == 1){
			printf("\nSORTING CORRECT\n");
		}else{
			printf("\nSORTING NOT CORRECT\n");
		}
	printf("Execution: %f\n", t2-t1);
	}

	//Free allocated Arrays
	free(index);
	free(displs);
	free(rdispls);
	free(count);
	free(bucket_count);
	free(bucket_list);
	free(final_list);
	MPI_Finalize();

}
