#if defined(__STDC__)
#  if (__STDC_VERSION__ >= 199901L)
#     define _XOPEN_SOURCE 700
#  endif
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mpi.h>


// ================================================================
//  MACROS and DATATYPES
// ================================================================
#define BUF_SIZE 256

// default amount of data
//
#if (!defined(DEBUG))
#define N_dflt    100000
#else
#define N_dflt    10000
#endif

// The structure, defined below as data_t is just an array of DATA_SIZE double, where DATA_SIZE is defined here below.
// We assume that we sort with respect one of the double in each structure, the HOTst one.
//
#if !defined(DATA_SIZE)
#define DATA_SIZE 8
#endif
#define HOT       0

typedef struct
{
  double data[DATA_SIZE];
} data_t;

#define min(a, b) (((a) < (b)) ? (a) : (b));


// ================================================================
//  PROTOTYPES
// ================================================================

// compare function type, used by the sorting routine
//
typedef int (compare_t)(const void*, const void*);

// verifying function type, used to test the results
//
typedef int (verify_t)(data_t *, int, int, int);

// declare the functions
//
extern inline compare_t compare;        // the compare function
extern inline compare_t compare_ge;     // the compare for "greater or equal"
verify_t  verify_partitioning;          // verification functions
verify_t  verify_sorting;
verify_t  show_array;


void quicksort( data_t *, int, int, compare_t ); 
extern inline int partitioning( data_t *, int, int, compare_t );

void generate_data(data_t *, const int );
double median_of_three(double, double, double );                  // needed for the pivot 
void merge( data_t *, int, const data_t *, int, int); 

void get_meminfo(unsigned long *, unsigned long *, unsigned long *,
                 unsigned long *, unsigned long *, unsigned long *,
                 unsigned long *);

int print_memory_info(char *, bool );

void print_results(int , long int , double , double , double , double );



// ================================================================
//  CODE
// ================================================================

int main(int argc, char **argv) {
    int N = N_dflt;
    // check command-line arguments
    //
    {
        int a = 0;
        if (argc > ++a) N = atoi(*(argv+a));
    }

    double init_time, end_time;
    int size;
    int rank;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

       
    // ======================
    // 1 MPI process

    if (size == 1) {
        data_t *data = (data_t*)malloc(N*sizeof(data_t));
        if (data == NULL) {
            fprintf(stderr, "Error: Unable to allocate memory for data on rank %d.\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        generate_data(data, N);

        init_time = MPI_Wtime();
        quicksort(data, 0, N, compare_ge);
        end_time = MPI_Wtime();

        if (verify_sorting(data, 0, N, 0)) {
            // show_array(data, 0, N, 0);
            print_results(size, N, init_time, 0, end_time, end_time);
        } else {
            printf("the array is not sorted correctly\n");
        }
        free(data);

        MPI_Finalize();
        return 0;
    }


    // ======================
    // Multiple MPI processes

    double scatter_time, sorting_time;
    MPI_Datatype mpi_data_type;
    MPI_Type_contiguous(DATA_SIZE, MPI_DOUBLE, &mpi_data_type);
    MPI_Type_commit(&mpi_data_type);

    if (rank == 0) {
        
        FILE *file = fopen("steps.log", "a");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
        fprintf(file, "\n\nNprocs=%d", size);
        fclose(file);
        print_memory_info("\nAllocation started------", false);
    }

    // Definition of two arrays to store, for each process:
    // own_size, assuming that the data will be distributed uniformly across the processes,
    // total_size, considering the data the process will receive from others in the merge phase later
    //
    int own_sizes[size];
    int total_sizes[size];
    bool sent[size];
    memset(sent, false, size * sizeof(bool));
    
    for (int i = 0; i < size; i++) {
        own_sizes[i] = N / size + (i < N % size);
        total_sizes[i] = N / size + (i < N % size);
    }
    
    for (int step = 1; step < size; step = 2 * step) {
        for (int i = 0; i < size; i++) {
            if (i % (2 * step) != 0 && sent[i]==false) {
                total_sizes[i-step] += total_sizes[i];
                sent[i] = true;
            }
        }
    }

    int my_own_size = own_sizes[rank];
    int my_total_size = total_sizes[rank];

    // Each process allocates an array with size = total_size
    // Although not necessary now, this choice avoids future reallocations improving memory locality and reducing data fragmentation
    //
    data_t *mydata = (data_t*)malloc(my_total_size * sizeof(data_t));
    if (mydata == NULL) {
      fprintf(stderr, "Error: Unable to allocate memory for mydata on rank %d.\n", rank);
      MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    
    // Each process also allocates an array for the data to be received, with size = total size/2 + 1 = maximum number of entries the process will receive
    // Since one process might receive data in more than one merging step, the maximum required memory is allocated all at once, instead of reallocating later
    //
    data_t *receive_buffer = NULL;
    if (my_total_size > my_own_size) {
        receive_buffer = (data_t*)malloc((my_total_size/2 + 1) * sizeof(data_t));
        if (receive_buffer == NULL) {
	        fprintf(stderr, "Error: Unable to allocate memory for receive_buffer on rank %d.\n", rank);
	        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	    }
    }

    // Root process distributes the data to all the other processes
    //
    if (rank == 0) {
        print_memory_info("\nGeneration started------", false);
        
        //// Memory allocation
        ////
        data_t *data = (data_t*)malloc(N * sizeof(data_t));
        if (data == NULL) {
            fprintf(stderr, "Error: Unable to allocate memory for data on rank %d.\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        //// Data generation
        ////
        generate_data(data, N);
        print_memory_info("\nGeneration ended--------", false);

        //// Execution time is computed from here
        ////
        init_time = MPI_Wtime();

        //// Size and starting point of each chunk are determined
        ////
        int displs[size];
        int sendcounts[size];
        displs[0] = 0;

        for (int i = 0; i < size; i++) {
            sendcounts[i] = N / size + (i < N % size);
            if (i > 0) {
                displs[i] = i * (N / size) + min(i, N % size);
            }
        }

        //// Data distribution
        ////
        print_memory_info("\nDistribution started----", false);    
        MPI_Scatterv(data, sendcounts, displs, mpi_data_type, &mydata[my_total_size-my_own_size], sendcounts[rank], mpi_data_type, 0, MPI_COMM_WORLD);   
        print_memory_info("\nDistribution ended------", false);

        //// Freeing the memory used to hold the whole data on the root process
        ////
        free(data);
        print_memory_info("\nFreeing ended-----------", false);
        scatter_time = MPI_Wtime();

    } else {
        //// MPI_Scatterv is a collective operation, thus all processes in the communicator must call it
        //// non-root processes pass NULL for sendbuf, sendcounts, and displs
        ////
        MPI_Scatterv(NULL, NULL, NULL, mpi_data_type, &mydata[my_total_size-my_own_size], own_sizes[rank], mpi_data_type, 0, MPI_COMM_WORLD);
    }    

    // SORTING
    //
    quicksort(mydata, my_total_size-my_own_size, my_total_size, compare_ge);
    MPI_Barrier(MPI_COMM_WORLD);
    sorting_time = MPI_Wtime();

    if (rank == 0) {
        print_memory_info("\nMerging started---------", false);
    }

    // MERGING: iterative binary tree-based reduction process (logarithmic steps)
    //
    for (int step = 1; step < size; step = 2 * step) {

        if (rank % (2 * step) != 0) {
            //// A process sends its data and exits the loop
            ////
            MPI_Send(mydata, own_sizes[rank], mpi_data_type, rank - step, 1, MPI_COMM_WORLD);
            free(receive_buffer);
            break;
        }

        if (rank + step < size) {            
            //// The receiver gets the data and merges it with its own, then updates own chunk size
            ////
	    MPI_Recv(receive_buffer, total_sizes[rank+step], mpi_data_type, rank + step, 1, MPI_COMM_WORLD, &status);
            print_memory_info("\nWhile merging------", false);
	    merge(mydata, own_sizes[rank], receive_buffer, total_sizes[rank+step], total_sizes[rank]);
            own_sizes[rank] += total_sizes[rank+step];
        }
    }


    // Each non-process frees its own data after exiting the merging phase
    //
    if (rank != 0) {
        free(mydata);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        print_memory_info("\nMerging ended-----------", false);
        
        if (verify_sorting(mydata, 0, N, 0)) {
            // show_array(mydata, 0, N, 0);
            print_results(size, N, init_time, scatter_time, sorting_time, end_time);
        } else {
            printf("the array is not sorted correctly\n");
        }
        
        free(mydata);
    }


    MPI_Finalize();
    return 0;
}



//FUNCTION DEFINITIONS//

 #define SWAP(A, B, SIZE) do { \
    char *a = (char *)(A);    \
    char *b = (char *)(B);    \
    for (int i = 0; i < (SIZE); i++) { \
        char _temp = a[i];    \
        a[i] = b[i];          \
        b[i] = _temp;         \
    }                         \
} while (0)


inline int compare( const void *A, const void *B )
{
  data_t *a = (data_t*)A;
  data_t *b = (data_t*)B;

  double diff = a->data[HOT] - b->data[HOT];
  return ( (diff > 0) - (diff < 0) );
}


inline int compare_ge( const void *A, const void *B )
{
  data_t *a = (data_t*)A;
  data_t *b = (data_t*)B;

  return (a->data[HOT] >= b->data[HOT]);
}


int verify_sorting( data_t *data, int start, int end, int not_used )
{
  int i = start;
  while( (++i < end) && (data[i].data[HOT] >= data[i-1].data[HOT]) );
  return ( i == end );
}


int verify_partitioning( data_t *data, int start, int end, int mid )
{
  int failure = 0;
  int fail = 0;
  
  for( int i = start; i < mid; i++ )
    if ( compare( (void*)&data[i], (void*)&data[mid] ) >= 0 )
      fail++;

  failure += fail;
  if ( fail )
    { 
      printf("failure in first half\n");
      fail = 0;
    }

  for( int i = mid+1; i < end; i++ )
    if ( compare( (void*)&data[i], (void*)&data[mid] ) < 0 )
      fail++;

  failure += fail;
  if ( fail )
    printf("failure in second half\n");

  return failure;
}


int show_array( data_t *data, int start, int end, int not_used )
{
  for ( int i = start; i < end; i++ )
    printf( "%f ", data[i].data[HOT] );
  printf("\n");
  return 0;
}


void quicksort( data_t *data, int start, int end, compare_t cmp_ge )
{

 #if defined(DEBUG)
 #define CHECK {							\
    if ( verify_partitioning( data, start, end, mid ) ) {		\
      printf( "partitioning is wrong\n");				\
      printf("%4d, %4d (%4d, %g) -> %4d, %4d  +  %4d, %4d\n",		\
	     start, end, mid, data[mid].data[HOT],start, mid, mid+1, end); \
      }}
 #else
 #define CHECK
 #endif

  int size = end-start;
  if ( size > 2 )
    {
      int mid = partitioning( data, start, end, cmp_ge );

      CHECK;
      
      quicksort( data, start, mid, cmp_ge );    // sort the left half
      quicksort( data, mid+1, end , cmp_ge );   // sort the right half
    }
  else
    {
      if ( (size == 2) && cmp_ge ( (void*)&data[start], (void*)&data[end-1] ) )
	SWAP( (void*)&data[start], (void*)&data[end-1], sizeof(data_t) );
    }
}


inline int partitioning( data_t *data, int start, int end, compare_t cmp_ge )
{
  
  int mid = start + (end - start) / 2;

  // Find the median of the start, middle, and end elements
  double pivot_value = median_of_three(data[start].data[HOT], 
                                        data[mid].data[HOT], 
                                        data[end-1].data[HOT]);

  // Find the index of the pivot value to swap with the last element
  if (pivot_value == data[start].data[HOT]) {
    SWAP(&data[start], &data[end-1], sizeof(data_t));
  } else if (pivot_value == data[mid].data[HOT]) {
    SWAP(&data[mid], &data[end-1], sizeof(data_t));
  }

  // pick up the last element as pivot
  --end;  
  void *pivot = (void*)&data[end];

  // partition around the pivot element  
  int pointbreak = end-1;

  for ( int i = start; i <= pointbreak; i++ )
    if( cmp_ge( (void*)&data[i], pivot ) )
      {
	while( (pointbreak > i) && cmp_ge( (void*)&data[pointbreak], pivot ) ) pointbreak--;
	if (pointbreak > i ) 
	  SWAP( (void*)&data[i], (void*)&data[pointbreak--], sizeof(data_t) );
      }  
  pointbreak += !cmp_ge( (void*)&data[pointbreak], pivot ) ;
  SWAP( (void*)&data[pointbreak], pivot, sizeof(data_t) );
  
  return pointbreak;
}


void generate_data(data_t *data, const int N) 
{  
  long int seed;
  seed = time(NULL);
  srand48(seed);
  //printf("ssed is % ld\n", seed);
    
  for ( int i = 0; i < N; i++ )
    data[i].data[HOT] = drand48();
}


double median_of_three(double a, double b, double c) {
  if ((a > b) != (a > c)) {
    return a;
  } else if ((b > a) != (b > c)) {
      return b;
  } else {
      return c;
  }
}


void merge(data_t *arr1, int n1, const data_t *arr2, int n2, int tot_n1) {

    int arr1_start = tot_n1 - n1;
    int merge_start = tot_n1 - n1 - n2;
    int i = 0, j = 0, k = merge_start;

    // Merging arrays
    while (i < n1 && j < n2) {
        if (arr1[arr1_start+i].data[HOT] <= arr2[j].data[HOT]) {
            arr1[k] = arr1[arr1_start+i];
            i++;
        } else {
            arr1[k] = arr2[j];
            j++;
        }
        k++;
    }

    while (j < n2) {
        arr1[k] = arr2[j];
        j++;
        k++;
    }
}


// Function to read and parse /proc/meminfo into various memory metrics
void get_meminfo(unsigned long *total_mem, unsigned long *free_mem, unsigned long *available_mem,
                 unsigned long *buffers, unsigned long *cached, unsigned long *total_swap,
                 unsigned long *free_swap) {
    FILE *file;
    char line[BUF_SIZE];
    
    // Initialize all values to 0
    *total_mem = *free_mem = *available_mem = *buffers = *cached = *total_swap = *free_swap = 0;
    
    file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "MemTotal:")) {
            sscanf(line, "MemTotal: %lu kB", total_mem);
        } else if (strstr(line, "MemFree:")) {
            sscanf(line, "MemFree: %lu kB", free_mem);
        } else if (strstr(line, "MemAvailable:")) {
            sscanf(line, "MemAvailable: %lu kB", available_mem);
        } else if (strstr(line, "Buffers:")) {
            sscanf(line, "Buffers: %lu kB", buffers);
        } else if (strstr(line, "Cached:")) {
            sscanf(line, "Cached: %lu kB", cached);
        } else if (strstr(line, "SwapTotal:")) {
            sscanf(line, "SwapTotal: %lu kB", total_swap);
        } else if (strstr(line, "SwapFree:")) {
            sscanf(line, "SwapFree: %lu kB", free_swap);
        }
    }
    
    fclose(file);
}


int print_memory_info(char * information_string, bool print_all) 
{
    unsigned long total_mem, free_mem, available_mem, buffers, cached;
    unsigned long total_swap, free_swap;
    unsigned long used_mem;
    unsigned long buff_cache;
    FILE *file = fopen("steps.log", "a");

    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
    
    if (print_all) {
        fprintf(file, "%s\n", information_string);
        used_mem = total_mem - free_mem;
        buff_cache = buffers + cached;
        fprintf(file, "               total        used        free      shared  buff/cache   available\n");
        fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
                total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
        fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
    }
    else {
        fprintf(file, "%s%ld", information_string, free_mem);
    }
    fclose(file);
}


void print_results(int size, long int N, double init_time, double scatter_time, double sorting_time, double end_time) 
{
    printf("%d\t\t%ld", size, N);
    if (N >= 10000000)
        printf("\t");
    else
        printf("\t\t");
    printf("%.6f\t%.6f\t%.6f\t%.6f\n", end_time - init_time, scatter_time - init_time, sorting_time - scatter_time, end_time - sorting_time);
};
