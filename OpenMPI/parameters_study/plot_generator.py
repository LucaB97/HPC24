import pandas as pd
import matplotlib.pyplot as plt
import sys

def main(test_file):
    # Load the data
    data = pd.read_csv(f"test__{test_file}.csv", delimiter=',')

    operation = data['Operation'].unique()
    algorithm = data['Algorithm'].unique()
    mapping = data['Mapping'].unique()

    # Set up a list of unique iteration and warmup values
    iterations_values = data['Iterations'].unique()
    warmup_values = data['Warmup'].unique()

    # Define the size threshold for splitting the plots
    size_threshold = 32768

    # Define colors for different parameter configurations
    colors = plt.cm.get_cmap('tab10', len(warmup_values))

    # Plotting for each value of iterations
    for iterations in iterations_values:
        fig, axs = plt.subplots(2, 1, figsize=(12, 16))  # Two subplots (2 rows, 1 column)
        
        for i, warmup in enumerate(warmup_values):
            color = colors(i)

            # Subset data for size < size_threshold
            subset_small = data[(data['Iterations'] == iterations) & (data['Warmup'] == warmup) & (data['Size'] < size_threshold)]
            axs[0].plot(subset_small['Size'], subset_small['Avg Latency(us)'], label=f'Avg Latency (Warmup {warmup})', color=color, marker='o', linewidth=2)
            axs[0].plot(subset_small['Size'], subset_small['Min Latency(us)'], label=f'Min Latency (Warmup {warmup})', color=color, marker='o', linestyle='dashed')
            axs[0].plot(subset_small['Size'], subset_small['Max Latency(us)'], label=f'Max Latency (Warmup {warmup})', color=color, marker='o', linestyle='dotted')

            # Subset data for size >= size_threshold
            subset_large = data[(data['Iterations'] == iterations) & (data['Warmup'] == warmup) & (data['Size'] >= size_threshold)]
            axs[1].plot(subset_large['Size'], subset_large['Avg Latency(us)'], label=f'Avg Latency (Warmup {warmup})', color=color, marker='o', linewidth=2)
            axs[1].plot(subset_large['Size'], subset_large['Min Latency(us)'], label=f'Min Latency (Warmup {warmup})', color=color, marker='o', linestyle='dashed')
            axs[1].plot(subset_large['Size'], subset_large['Max Latency(us)'], label=f'Max Latency (Warmup {warmup})', color=color, marker='o', linestyle='dotted')
        
        # Titles, labels, and scales for each subplot
        axs[0].set_title(f'{operation}, {algorithm}, {mapping} --- Iterations = {iterations} (Size < {size_threshold})')
        axs[1].set_title(f'{operation}, {algorithm}, {mapping} --- Iterations = {iterations} (Size >= {size_threshold})')
        for ax in axs:
            ax.set_xlabel('Size')
            ax.set_ylabel('Latency (us)')
            # ax.set_xscale('log')
            # ax.set_yscale('log')
            ax.legend()
            ax.grid(True)
        
        plt.tight_layout()
        plt.savefig(f"test__{test_file}__iterations_{iterations}.png")  # Unique filename
        plt.close()

    # Plotting for each value of warmup
    for warmup in warmup_values:
        fig, axs = plt.subplots(2, 1, figsize=(12, 16))  # Two subplots (2 rows, 1 column)

        for i, iterations in enumerate(iterations_values):
            color = colors(i)

            # Subset data for size < size_threshold
            subset_small = data[(data['Warmup'] == warmup) & (data['Iterations'] == iterations) & (data['Size'] < size_threshold)]
            axs[0].plot(subset_small['Size'], subset_small['Avg Latency(us)'], label=f'Avg Latency (Iterations {iterations})', color=color, marker='o', linewidth=2)
            axs[0].plot(subset_small['Size'], subset_small['Min Latency(us)'], label=f'Min Latency (Iterations {iterations})', color=color, marker='o', linestyle='dashed')
            axs[0].plot(subset_small['Size'], subset_small['Max Latency(us)'], label=f'Max Latency (Iterations {iterations})', color=color, marker='o', linestyle='dotted')

            # Subset data for size >= size_threshold
            subset_large = data[(data['Warmup'] == warmup) & (data['Iterations'] == iterations) & (data['Size'] >= size_threshold)]
            axs[1].plot(subset_large['Size'], subset_large['Avg Latency(us)'], label=f'Avg Latency (Iterations {iterations})', color=color, marker='o', linewidth=2)
            axs[1].plot(subset_large['Size'], subset_large['Min Latency(us)'], label=f'Min Latency (Iterations {iterations})', color=color, marker='o', linestyle='dashed')
            axs[1].plot(subset_large['Size'], subset_large['Max Latency(us)'], label=f'Max Latency (Iterations {iterations})', color=color, marker='o', linestyle='dotted')

        # Titles, labels, and scales for each subplot
        axs[0].set_title(f'{operation}, {algorithm}, {mapping} --- Warmup = {warmup} (Size < {size_threshold})')
        axs[1].set_title(f'{operation}, {algorithm}, {mapping} --- Warmup = {warmup} (Size >= {size_threshold})')
        for ax in axs:
            ax.set_xlabel('Size')
            ax.set_ylabel('Latency (us)')
            # ax.set_xscale('log')
            # ax.set_yscale('log')
            ax.legend()
            ax.grid(True)

        plt.tight_layout()
        plt.savefig(f"test__{test_file}__warmup_{warmup}.png")  # Unique filename
        plt.close()

if __name__ == "__main__":
    # If running the script from the command line, you can pass the filename as an argument
    if len(sys.argv) > 1:
        test_file = sys.argv[1]  # e.g., 'data' if the file is named 'test__data.csv'
    else:
        test_file = 1  # Default file name if no argument is given
    main(test_file)
