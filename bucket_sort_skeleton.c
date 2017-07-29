#include <stdio.h>
#include <stdlib.h>


// Comparison function used by qsort
int compare_dbls(const void* arg1, const void* arg2)
{
    double a1 = *(double *) arg1;
    double a2 = *(double *) arg2;
    if (a1 < a2) return -1;
    else if (a1 == a2) return 0;
    else return 1;
}
// Sort the array in place
void qsort_dbls(double *array, int array_len)
{
    qsort(array, (size_t)array_len, sizeof(double),
          compare_dbls);
}

void main(int argc, char* argv[]){
    
    double *input_array;   // the input array
    double *bucketlist;    //this array will contain input elements in order of the processors
                            //e.g elements of process 0 will be stored first, then elements of process 1, and so on
    double *local_array;    //This array will contain the elements in each process
    
    int n;
    int p;
    int i;
    int my_rank;
    
    int *scounts;           //This array will contain the counts of elements each processor will receive
    int *dspls;             //The relative offsets in bucketlist array where the elements of different processes
                            //will be stored
    int *bin_elements;       //it will keep track of how many elements have been included in the pth bin
    
    n = atoi(argv[1]);
    
    p = 4;                  //dummy processor count
    
    input_array = malloc(n*sizeof(double));
    bucketlist = malloc(n*sizeof(double));
    scounts = malloc(p*sizeof(int));
    dspls = malloc(p*sizeof(int));
    bin_elements = malloc(p*sizeof(int));
    
    for(i = 0 ; i < n ; i++){
        input_array[i] = ((double) rand()/RAND_MAX);
    }
    
    for(i = 0 ; i < p ; i++){
        scounts[i] = 0 ;
    }
    
    //counting the elements in each processor
    for(i = 0 ; i < n ; i++){
        scounts[(int)(input_array[i]/(1.0/p))]++;
    }
    
    
    
    for(i = 0 ; i<p ; i++){
        bin_elements[i] = scounts[i];
    }
    
   
    
    dspls[0] = 0;
    for(i = 0 ; i< p-1 ;i++){
        dspls[i+1] = dspls[i] + scounts[i];
    }
    
    
    int bin;
    int pos;
    for(i = 0 ; i < n ; i++){
        bin = (int)(input_array[i]/(1.0/p));
        pos = dspls[bin] + scounts[bin] - bin_elements[bin];
        bucketlist[pos] = input_array[i];
        bin_elements[bin]--;
    }
    
    

    /*
     Now the scount array contains the number of elements each process will receive from the root. dspls 
     contains the relative offsets where elements from different processes will be stored and bucketlist array 
     contains the elements stored in the order of the processors
     */
    
}
