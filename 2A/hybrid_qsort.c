#if defined(__STDC__)
#  if (__STDC_VERSION__ >= 199901L)
#     define _XOPEN_SOURCE 700
#  endif
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/time.h> // for gettimeofday
#include <mpi.h>


// ================================================================
//  MACROS and DATATYPES
// ================================================================
#define BUF_SIZE 256

// measure ther cpu process time
#define CPU_TIME (clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ), (double)ts.tv_sec + \
                  (double)ts.tv_nsec * 1e-9)

#if defined(DEBUG)
#define VERBOSE
#endif

#if defined(VERBOSE)
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

// The data to be sorted consists in a structure, just to mimic that you
// may have to sort not basic types, whch may have some effects on the memory efficiency.
// The structure, defined below as data_t is just an array of DATA_SIZE double,
// where DATA_SIZE is defined here below.
// However, for the sake of simplicity, we assume that we sort with respect one of
// the double in each structure, the HOTst one, where HOT is defined below (my
// choice was to set HOT to 0, so the code will use the first double to sort the data).

#if !defined(DATA_SIZE)
#define DATA_SIZE 8
#endif
#define HOT       0

// let's define the default amount of data
//
#if (!defined(DEBUG))
#define N_dflt    100000
#else
#define N_dflt    10000
#endif

// let's define the data_t type
//
typedef struct
{
  double data[DATA_SIZE];
} data_t;


// let's defined convenient macros for max and min between
// two data_t objects
//
#define MAX( a, b ) ( (a)->data[HOT] >(b)->data[HOT]? (a) : (b) );
#define MIN( a, b ) ( (a)->data[HOT] <(b)->data[HOT]? (a) : (b) );

#define min(a, b) (((a) < (b)) ? (a) : (b));
// ================================================================
//  PROTOTYPES
// ================================================================

// let'ìs define the compare funciton that will be used by the sorting routine//
typedef int (compare_t)(const void*, const void*);

// let's define the verifying function type, used to test the results//
typedef int (verify_t)(data_t *, int, int, int);

// declare the functions//
extern inline compare_t compare;        // the compare function
extern inline compare_t compare_ge;     // the compare for "greater or equal"
verify_t  verify_partitioning;          // verification functions
verify_t  verify_sorting;

double median_of_three(double, double, double );                  // needed for the pivot 
extern inline int partitioning( data_t *, int, int, compare_t );

void generate_data(data_t *, const int );
void quicksort( data_t *, int, int, compare_t ); 
void merge( data_t **, const int, const data_t *, const int ); 

void get_meminfo(unsigned long *, unsigned long *, unsigned long *,
                 unsigned long *, unsigned long *, unsigned long *,
                 unsigned long *);

// ================================================================
//  CODE
// ================================================================

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    int N = N_dflt;

    /* check command-line arguments */
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

    FILE *file;
    unsigned long total_mem, free_mem, available_mem, buffers, cached;
    unsigned long total_swap, free_swap;
    unsigned long used_mem;
    unsigned long buff_cache;

    //// 1 process
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
            printf("%d\t\t%d", size, N);
            if (N >= 10000000)
                printf("\t");
            else
                printf("\t\t");
            printf("%.6f\t0.000000\t%.6f\t0.000000\n", end_time - init_time, end_time - init_time);
        } else {
            printf("the array is not sorted correctly\n");
        }
        free(data);

        MPI_Finalize();
        return 0;
    }

    //// multiple processes
    double scatter_time, sorting_time;
    double step_time;
    MPI_Datatype mpi_data_type;
    MPI_Type_contiguous(DATA_SIZE, MPI_DOUBLE, &mpi_data_type);
    MPI_Type_commit(&mpi_data_type);

    int myN = N / size + (rank < N % size);
    data_t *mydata = (data_t*)malloc(myN * sizeof(data_t));

    // Data generation and distribution
    if (rank == 0) {
        file = fopen("steps.log", "a");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
        fprintf(file, "Allocation started\n");
        get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
        fprintf(file, "               total        used        free      shared  buff/cache   available\n");
        fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
                total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
        fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
        fclose(file);

        data_t *data = (data_t*)malloc(N * sizeof(data_t));
        if (data == NULL) {
            fprintf(stderr, "Error: Unable to allocate memory for data on rank %d.\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        file = fopen("steps.log", "a");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
        fprintf(file, "Generation started\n");
        get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
        fprintf(file, "               total        used        free      shared  buff/cache   available\n");
        fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
                total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
        fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
        fclose(file);

        generate_data(data, N);
        
        file = fopen("steps.log", "a");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
        fprintf(file, "Generation ended\n");
        get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
        fprintf(file, "               total        used        free      shared  buff/cache   available\n");
        fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
                total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
        fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
        fclose(file);

        init_time = MPI_Wtime();
        int displs[size];
        int sendcounts[size];
        displs[0] = 0;

        for (int i = 0; i < size; i++) {
            sendcounts[i] = N / size + (i < N % size);
            if (i > 0) {
                displs[i] = i * (N / size) + min(i, N % size);
            }
        }
        
        file = fopen("steps.log", "a");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
        fprintf(file, "Distribution started\n");
        get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
        fprintf(file, "               total        used        free      shared  buff/cache   available\n");
        fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
                total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
        fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
        fclose(file);

        MPI_Scatterv(data, sendcounts, displs, mpi_data_type, mydata, N, mpi_data_type, 0, MPI_COMM_WORLD);
        
        file = fopen("steps.log", "a");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
        fprintf(file, "Distribution ended\n");
        get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
        fprintf(file, "               total        used        free      shared  buff/cache   available\n");
        fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
                total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
        fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
        fclose(file);

        free(data);
	scatter_time = MPI_Wtime();
	file = fopen("steps.log", "a");
	if (file == NULL) {
	  perror("Error opening file");
	  return EXIT_FAILURE;
	}
	fprintf(file, "Freeing completed\n");
	get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
	fprintf(file, "               total        used        free      shared  buff/cache   available\n");
	fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
	total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
	fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
	fclose(file);
        
    } else {
        MPI_Scatterv(NULL, NULL, NULL, mpi_data_type, mydata, myN, mpi_data_type, 0, MPI_COMM_WORLD);
    }

    // Sorting
    quicksort(mydata, 0, myN, compare_ge);
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        file = fopen("steps.log", "a");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
        fprintf(file, "Merging started\n");
        get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
        fprintf(file, "               total        used        free      shared  buff/cache   available\n");
        fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
                total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
        fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
        fclose(file);
    }
    sorting_time = MPI_Wtime();

    // Merging
    int own_chunk_size = myN;
    for (int step = 1; step < size; step = 2 * step) {

        if (rank % (2 * step) != 0) {
            MPI_Send(&own_chunk_size, 1, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
            MPI_Send(mydata, own_chunk_size, mpi_data_type, rank - step, 1, MPI_COMM_WORLD);
            break;
        }

        if (rank + step < size) {
            int received_chunk_size;
            MPI_Recv(&received_chunk_size, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, &status);
            data_t *chunk_received = (data_t *)malloc(received_chunk_size * sizeof(data_t));
            if (chunk_received == NULL) {
                fprintf(stderr, "Error: Unable to allocate memory to receive data on rank %d.\n", rank + step);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
            MPI_Recv(chunk_received, received_chunk_size, mpi_data_type, rank + step, 1, MPI_COMM_WORLD, &status);

            merge(&mydata, own_chunk_size, chunk_received, received_chunk_size);
            free(chunk_received);
            own_chunk_size = own_chunk_size + received_chunk_size;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        file = fopen("steps.log", "a");
        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
        fprintf(file, "Merging ended\n");
        get_meminfo(&total_mem, &free_mem, &available_mem, &buffers, &cached, &total_swap, &free_swap);
        fprintf(file, "               total        used        free      shared  buff/cache   available\n");
        fprintf(file, "Mem:       %8lu  %8lu  %8lu  %8lu  %8lu  %8lu\n",
                total_mem, used_mem, free_mem, 0UL, buff_cache, available_mem);
        fprintf(file, "Swap:      %8lu  %8lu  %8lu\n", total_swap, total_swap - free_swap, free_swap);
        fclose(file);
    }
    end_time = MPI_Wtime();

    // Correctness check and time printing
    if (rank == 0) {
        if (verify_sorting(mydata, 0, N, 0)) {
            printf("%d\t\t%d", size, N);
            if (N >= 10000000)
                printf("\t");
            else
                printf("\t\t");
            printf("%.6f\t%.6f\t%.6f\t%.6f\n", end_time - init_time, scatter_time - init_time, sorting_time - scatter_time, end_time - sorting_time);
        } else {
            printf("the array is not sorted correctly\n");
        }
    }

    free(mydata);

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


double median_of_three(double a, double b, double c) {
  if ((a > b) != (a > c)) {
    return a;
  } else if ((b > a) != (b > c)) {
      return b;
  } else {
      return c;
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
    
  PRINTF("ssed is % ld\n", seed);
    
  for ( int i = 0; i < N; i++ )
    data[i].data[HOT] = drand48();
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


void merge(data_t **arr1, int n1, const data_t *arr2, int n2) {
    int total_size = n1 + n2;
    data_t *result = (data_t*)malloc(total_size * sizeof(data_t));

    if (result == NULL) {
        // Handle memory allocation failure (e.g., log error, exit, etc.)
        fprintf(stderr, "Error: Unable to allocate memory for merging arrays.\n");
        return;
    }

    int i = 0, j = 0, k = 0;

    // Merging arrays
    while (i < n1 && j < n2) {
        if ((*arr1)[i].data[HOT] <= arr2[j].data[HOT]) {
            result[k++] = (*arr1)[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }

    // Copy remaining elements
    while (i < n1) {
        result[k++] = (*arr1)[i++];
    }
    while (j < n2) {
        result[k++] = arr2[j++];
    }

    // Reallocate memory for arr1 to hold the merged data
    data_t *resized_arr1 = (data_t*)realloc(*arr1, total_size * sizeof(data_t));
    if (resized_arr1 == NULL) {
        // Handle realloc failure (original *arr1 is still valid)
        fprintf(stderr, "Error: Unable to reallocate memory for merged array.\n");
        free(result);
        free(*arr1);
        return;
    }

    *arr1 = resized_arr1;

    // Copy the merged data into the resized array
    memcpy(*arr1, result, total_size * sizeof(data_t));

    // Free the temporary result array
    free(result);
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
