#define _GNU_SOURCE
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

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
    unsigned int cpu, node;
    if (getcpu(&cpu, &node) == -1) {
        perror("getcpu");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int core_id = (int)cpu;
    int node_id = (int)node;
    // Print the core ID for each rank (debug)
    printf("Rank %d on host %s is on core %d\n", rank, hostname, core_id);

    // Root process collects all core IDs
    int *core_ids = NULL;
    int *node_ids = NULL;
    if (rank == 0) {
        core_ids = (int *)malloc(size * sizeof(int));
	node_ids = (int *)malloc(size * sizeof(int));
        if (core_ids == NULL) {
            perror("malloc");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Gather all core IDs to the root process
    MPI_Gather(&core_id, 1, MPI_INT, core_ids, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&node_id, 1, MPI_INT, node_ids, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Root process writes the core IDs to a CSV file
    if (rank == 0) {
        FILE *file = fopen(argv[1], "w");
        if (file == NULL) {
            perror("fopen");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fprintf(file, "Rank,Core_ID,Node_ID\n");
        for (int i = 0; i < size; i++) {
            fprintf(file, "%d,%d,%d\n", i, core_ids[i], node_ids[i]);
        }

        fclose(file);
        free(core_ids);
	free(node_ids);
    }

    // Finalize the MPI environment
    MPI_Finalize();

    return 0;
}
