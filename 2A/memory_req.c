#include <stdio.h>
#include <stdlib.h>

#if !defined(DATA_SIZE)
#define DATA_SIZE 8
#endif

// let's define the data_t type
//
typedef struct
{
  double data[DATA_SIZE];
} data_t;

int main() {
    
    printf("%ld\n", sizeof(data_t));
    return 0;
}
