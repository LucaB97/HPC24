#define _GNU_SOURCE
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

int main(int argc, char *argv[]) {
    int rank, size, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int node_id;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the name of the processor
    MPI_Get_processor_name(hostname, &len);

    // Extract the numeric part of the hostname (assuming it follows the pattern "node123")
    sscanf(hostname, "%*[^0-9]%d", &node_id);

    // Get the CPU ID and NUMA node where the process is currently running
    unsigned int cpu, numa;
    if (getcpu(&cpu, &numa) == -1) {
        perror("getcpu");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int core_id = (int)cpu;
    int numa_id = (int)numa;

    // Print the core ID and node ID for each rank (debug)
    printf("Rank %d on host %s (Node ID: %d) is on core %d (NUMA ID: %d)\n", rank, hostname, node_id, core_id, numa_id);

    // Root process collects all core IDs, NUMA IDs, and node IDs
    int *core_ids = NULL;
    int *numa_ids = NULL;
    int *node_ids = NULL;

    if (rank == 0) {
        core_ids = (int *)malloc(size * sizeof(int));
        numa_ids = (int *)malloc(size * sizeof(int));
        node_ids = (int *)malloc(size * sizeof(int));
        if (core_ids == NULL || numa_ids == NULL || node_ids == NULL) {
            perror("malloc");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Gather all core IDs, NUMA IDs, and node IDs to the root process
    MPI_Gather(&core_id, 1, MPI_INT, core_ids, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&numa_id, 1, MPI_INT, numa_ids, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&node_id, 1, MPI_INT, node_ids, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Root process writes the data to a CSV file
    if (rank == 0) {
        FILE *file = fopen(argv[1], "w");
        if (file == NULL) {
            perror("fopen");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fprintf(file, "Rank,Core_ID,NUMA_ID,Node_ID\n");
        for (int i = 0; i < size; i++) {
            fprintf(file, "%d,%d,%d,%d\n", i, core_ids[i], numa_ids[i], node_ids[i]);
        }

        fclose(file);
        free(core_ids);
        free(numa_ids);
        free(node_ids);
    }

    // Finalize the MPI environment
    MPI_Finalize();

    return 0;
}
