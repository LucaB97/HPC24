#include <stdlib.h>
#include <stdio.h>

int main() {
    int *p;
    long long total_allocated = 0;
    int inc = 1000 * 1000 * 1024 * sizeof(char);
    int iteration = 0;

    while (1) {
        p = (int *) calloc(1,inc);
	if (!p) {
	    printf("Memory allocation failed. Total memory allocated: %lld KiB\n", total_allocated / (1024));
	    break;
	}
	total_allocated += inc;
	iteration++;
        printf("Iteration %d: Allocated %lld KiB\n", iteration, total_allocated / (1024));
    }  

  return 0;
}
