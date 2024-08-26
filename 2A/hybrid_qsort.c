
/* ────────────────────────────────────────────────────────────────────────── *
 │                                                                            │
 │ This file is part of the exercises for the Lectures on                     │
 │   "Foundations of High Performance Computing"                              │
 │ given at                                                                   │
 │   Master in HPC and                                                        │
 │   Master in Data Science and Scientific Computing                          │
 │ @ SISSA, ICTP and University of Trieste                                    │
 │                                                                            │
 │ contact: luca.tornatore@inaf.it                                            │
 │                                                                            │
 │     This is free software; you can redistribute it and/or modify           │
 │     it under the terms of the GNU General Public License as published by   │
 │     the Free Software Foundation; either version 3 of the License, or      │
 │     (at your option) any later version.                                    │
 │     This code is distributed in the hope that it will be useful,           │
 │     but WITHOUT ANY WARRANTY; without even the implied warranty of         │
 │     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          │
 │     GNU General Public License for more details.                           │
 │                                                                            │
 │     You should have received a copy of the GNU General Public License      │
 │     along with this program.  If not, see <http://www.gnu.org/licenses/>   │
 │                                                                            │
 * ────────────────────────────────────────────────────────────────────────── */


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
#include <omp.h>
#include <time.h>
#include <mpi.h>


// ================================================================
//  MACROS and DATATYPES
// ================================================================


#if defined(_OPENMP)

// measure the wall-clock time
#define CPU_TIME (clock_gettime( CLOCK_REALTIME, &ts ), (double)ts.tv_sec + \
                  (double)ts.tv_nsec * 1e-9)

// measure the cpu thread time
#define CPU_TIME_th (clock_gettime( CLOCK_THREAD_CPUTIME_ID, &myts ), (double)myts.tv_sec +     \
                     (double)myts.tv_nsec * 1e-9)

#else

// measure ther cpu process time
#define CPU_TIME (clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ), (double)ts.tv_sec + \
                  (double)ts.tv_nsec * 1e-9)
#endif


#if defined(DEBUG)
#define VERBOSE
#endif

#if defined(VERBOSE)
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

//
// The data to be sorted consists in a structure, just to mimic that you
// may have to sort not basic types, whch may have some effects on the memory efficiency.
// The structure, defined below as data_t is just an array of DATA_SIZE double,
// where DATA_SDIZE is defined here below.
// However, for the sake of simplicity, we assume that we sort with respect one of
// the double in each structure, the HOTst one, where HOT is defined below (my
// choice was to set HOT to 0, so the code will use the first double to sort the
// data).
//
// Note that you can chnage the data structure and the ordering as you like.

#if !defined(DATA_SIZE)
#define DATA_SIZE 8
#endif
#define HOT       0

// let's define the default amount of data
//
#if (!defined(DEBUG) || defined(_OPENMP))
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

// let'ìs define the compare funciton that will be used by the
// sorting routine
//
typedef int (compare_t)(const void*, const void*);

// let's define the verifying function type, used to test the
// results
//
typedef int (verify_t)(data_t *, int, int, int);


// declare the funcitons
//
extern inline compare_t compare;        // the compare function
extern inline compare_t compare_ge;     // the compare for "greater or equal"
verify_t  verify_partitioning;          // verification functions
verify_t  verify_sorting;
verify_t  show_array;

// declare the partitioning function
//
extern inline int partitioning( data_t *, int, int, compare_t );

// declare the sorting function
//
void quicksort( data_t *, int, int, compare_t ); 
void generate_data(data_t *, const int );
void copy_data( data_t *, const data_t *, const int );
void merge( data_t **, const int, const data_t *, const int ); 

// ================================================================
//  CODE
// ================================================================


int main ( int argc, char **argv )
{

  int N          = N_dflt;
  
  /* check command-line arguments */
  {
    int a = 0;    
    if ( argc > ++a ) N = atoi(*(argv+a));
  }
  
  int nthreads;
  double init_time, end_time;
  int size;
  int rank;
  MPI_Status status;
  int tag = 123;
  MPI_Init( &argc, &argv );
  MPI_Comm_size(MPI_COMM_WORLD, &size); 
  

  ////Only 1 MPI process: OpenMP execution
  if (size==1) {
    data_t *data = (data_t*)malloc(N*sizeof(data_t));
    generate_data(data, N);
    
    init_time = MPI_Wtime();
    quicksort(data, 0, N, compare_ge);
    double end_time = MPI_Wtime();
    
    if ( verify_sorting( data, 0, N, 0) ) {
      printf("Size\tN\t\tTotal time\tCommunication\tSorting\t\tMerging\n");
      printf("%d\t%d\t\t%g\t0.000000\t%g\t0.000000\n", size, N, end_time-init_time, end_time-init_time);
    } 
    else
      printf("the array is not sorted correctly\n");
    free( data );
    
    MPI_Finalize();
    return 0;
  } 
  
  ////Hybrid (MPI+OpenMP) execution
  double scatter_time, sorting_time;
  MPI_Datatype mpi_data_type;
  MPI_Type_contiguous(DATA_SIZE, MPI_DOUBLE, &mpi_data_type);
  MPI_Type_commit(&mpi_data_type);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
  int myN = N/size + (rank < N%size);  
  data_t *mydata = (data_t*)malloc(myN*sizeof(data_t));

  //Data generation and distribution
  if (rank==0) {    
    data_t *data = (data_t*)malloc(N*sizeof(data_t));
    generate_data(data, N);

    init_time = MPI_Wtime();
    int displs[size];
    int sendcounts[size];
    displs[0] = 0;

    for (int i=0 ; i<size ; i++) {
      sendcounts[i] = N/size + (i < N%size);
      if (i > 0) {
        displs[i] = i*(N/size) + min(i, N%size);
      }
    }
    
    MPI_Scatterv(data, sendcounts, displs, mpi_data_type, mydata, N, mpi_data_type, 0, MPI_COMM_WORLD);
    free(data);
    scatter_time = MPI_Wtime();
  } 
  else {
    MPI_Scatterv(NULL, NULL, NULL, mpi_data_type, mydata, myN, mpi_data_type, 0, MPI_COMM_WORLD);
  }
  
  /*
  printf("-------------------\nI am %d\n", rank);
  show_array(mydata, 0,myN, 0);
  */
  
  //Sorting (quicksort using OpenMP within each process)
  quicksort(mydata, 0, myN, compare_ge);
  MPI_Barrier(MPI_COMM_WORLD);
  sorting_time = MPI_Wtime();

  //Merging phase
  int own_chunk_size = myN;
  for (int step = 1; step < size; step = 2 * step) {
    if (rank % (2 * step) != 0) {
      MPI_Send(&own_chunk_size, 1, MPI_INT, rank-step, 0, MPI_COMM_WORLD);
      MPI_Send(mydata, own_chunk_size, mpi_data_type, rank-step, 0, MPI_COMM_WORLD);
      break;
    }
 
    if (rank + step < size) {
      int received_chunk_size;
      MPI_Recv(&received_chunk_size, 1, MPI_INT, rank+step, 0, MPI_COMM_WORLD, &status);
      data_t * chunk_received = (data_t *)malloc(received_chunk_size * sizeof(data_t));
      MPI_Recv(chunk_received, received_chunk_size, mpi_data_type, rank+step, 0, MPI_COMM_WORLD, &status);
      
      merge(&mydata, own_chunk_size, chunk_received, received_chunk_size);
      free(chunk_received);
      own_chunk_size = own_chunk_size + received_chunk_size;
    }
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  end_time = MPI_Wtime();

  //Correctness check and time printing
  if (rank == 0) {
    if ( verify_sorting( mydata, 0, N, 0) ) {
      //printf("-------------------------\n-------------------------\nSorted array:\n");
      //show_array(mydata, 0, own_chunk_size, 0);
      printf("Size\tN\t\tTotal time\tCommunication\tSorting\t\tMerging\n");
      printf("%d\t%d\t\t%g\t%g\t%g\t%g\n", size, N, end_time-init_time, scatter_time-init_time, sorting_time-scatter_time , end_time-sorting_time);
    } 
    else
      printf("the array is not sorted correctly\n");
  }

  free(mydata);  

  MPI_Finalize();
  return 0;
}


//FUNCTION DEFINITIONS//

 #define SWAP(A,B,SIZE) do {int sz = (SIZE); char *a = (A); char *b = (B); \
    do { char _temp = *a;*a++ = *b;*b++ = _temp;} while (--sz);} while (0)

inline int partitioning( data_t *data, int start, int end, compare_t cmp_ge )
{
  
  // pick up the meadian of [0], [mid] and [end] as pivot
  //
  
  /* to be done */

  // pick up the last element as pivot
  //
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


void quicksort( data_t *data, int start, int end, compare_t cmp_ge )
{

 #if defined(DEBUG)
 #define CHECK {							\
    if ( verify_partitioning( data, start, end, mid ) ) {		\
      printf( "partitioning is wrong\n");				\
      printf("%4d, %4d (%4d, %g) -> %4d, %4d  +  %4d, %4d\n",		\
	     start, end, mid, data[mid].data[HOT],start, mid, mid+1, end); \
      show_array( data, start, end, 0 ); }}
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


void generate_data(data_t *data, const int N) 
{  
  long int seed;
  seed = time(NULL);
  srand48(seed);
    
  PRINTF("ssed is % ld\n", seed);
    
  for ( int i = 0; i < N; i++ )
    data[i].data[HOT] = drand48();
}


void copy_data(data_t * new_struct, const data_t * old_struct, const int N) 
{
  for (int i=0 ; i<N ; i++) {
    //printf("New: %f \t Old: %f\n", new_struct[i].data[HOT], old_struct[i].data[HOT]);
    new_struct[i].data[HOT] = old_struct[i].data[HOT];
  }
}


void merge( data_t ** arr1, const int n1, const data_t * arr2, const int n2) 
{
    data_t* result = (data_t*)malloc((n1 + n2) * sizeof(data_t));
    
    int i = 0;
    int j = 0;

    for (int k = 0; k < n1 + n2; k++) {
        if (i >= n1) {
            result[k].data[HOT] = arr2[j].data[HOT];
            j++;
        }
        else if (j >= n2) {
            result[k].data[HOT] = (*arr1)[i].data[HOT];
            i++;
        }
 
        // Indices in bounds as i < n1
        // && j < n2
        else if ((*arr1)[i].data[HOT] < arr2[j].data[HOT]) {
            result[k].data[HOT] = (*arr1)[i].data[HOT];
            i++;
        }
 
        // v2[j] <= v1[i]
        else {
            result[k].data[HOT] = arr2[j].data[HOT];
            j++;
        }
    }

    data_t* resized_arr1 = (data_t*)realloc(*arr1, (n1 + n2) * sizeof(data_t));
    if (!resized_arr1) {
        // Handle memory allocation failure
        // (e.g., log an error, clean up, and return)
        free(result);
        return;
    }
    
    *arr1 = resized_arr1;
    copy_data(*arr1, result, n1+n2);
    free(result);
}