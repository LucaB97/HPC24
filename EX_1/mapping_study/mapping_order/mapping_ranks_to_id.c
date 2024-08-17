#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int rank, size, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    
    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Get the name of the processor
    MPI_Get_processor_name(hostname, &len);
    
    // Get the CPU ID where the process is currently running
    int core_id = sched_getcpu();
    
    // Allocate an array to store core_ids of all ranks (only on the root process)
    int *core_ids = NULL;
    if (rank == 0) {
        core_ids = malloc(size * sizeof(int));
    }
    
    // Gather all core_ids to the root process (rank 0)
    MPI_Gather(&core_id, 1, MPI_INT, core_ids, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Only the root process saves the data to a CSV file
    if (rank == 0) {
        char filename[256];
        if (argc > 1) {
            snprintf(filename, sizeof(filename), "%s.csv", argv[1]);
        } else {
            snprintf(filename, sizeof(filename), "core_mapping.csv");
        }

        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        fprintf(fp, "Rank,Core_ID\n");
        for (int i = 0; i < size; i++) {
            fprintf(fp, "%d,%d\n", i, core_ids[i]);
        }

        fclose(fp);
        free(core_ids);
    }
    
    // Finalize the MPI environment
    MPI_Finalize();
    
    return 0;
}
