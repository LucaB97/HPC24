import pandas as pd
import matplotlib.pyplot as plt
import sys

def create_graphs(input_file):
    # Read the CSV file into a pandas DataFrame
    csv_file = str(input_file + ".csv")
    df = pd.read_csv(csv_file)

    # Get unique algorithms
    algorithms = df['Algorithm'].unique()

    for algo in algorithms:
        # Filter the DataFrame for the current algorithm
        algo_df = df[df['Algorithm'] == algo]

        # Extract the sizes and the average latencies for each mapping
        sizes = algo_df['Size'].unique()
        mappings = algo_df['Mapping'].unique()

        # Create a plot
        plt.figure(figsize=(10, 6))

        # Plot each mapping's latencies
        for mapping in mappings:
            mapping_df = algo_df[algo_df['Mapping'] == mapping]
            latencies = mapping_df['Avg Latency(us)'].values
            plt.plot(sizes, latencies, marker='o', label=mapping)

        # Set logarithmic scale
        plt.xscale('log')
        plt.yscale('log')

        # Set plot title and labels
        plt.xlabel('Message Size (Bytes)')
        plt.ylabel('Average Latency (us)')
        # plt.title(f'Algorithm {algo}')
        # plt.title(f'Latency Comparison for Algorithm {algo}')
        plt.legend(title='Mapping')
        plt.grid(True, which="both", linestyle='--')
        plt.savefig(str(input_file + ".png"))

        # Show the plot
        plt.show()

if __name__ == "__main__":
    create_graphs(sys.argv[1])
