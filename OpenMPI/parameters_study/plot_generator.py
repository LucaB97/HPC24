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

    # Plotting for each value of iterations
    for iterations in iterations_values:
        plt.figure(figsize=(12, 8))
        for warmup in warmup_values:
            subset = data[(data['Iterations'] == iterations) & (data['Warmup'] == warmup)]
            plt.plot(subset['Size'], subset['Avg Latency(us)'], label=f'Avg Latency (Warmup {warmup})', marker='o')
            plt.plot(subset['Size'], subset['Min Latency(us)'], label=f'Min Latency (Warmup {warmup})', marker='o')
            plt.plot(subset['Size'], subset['Max Latency(us)'], label=f'Max Latency (Warmup {warmup})', marker='o')
        
        plt.title(f'{operation}, {algorithm}, {mapping}  --- Iterations = {iterations}')
        plt.xlabel('Size')
        plt.ylabel('Latency (us)')
        plt.xscale('log')
        plt.yscale('log')
        plt.legend()
        plt.grid(True)
        plt.savefig(f"test__{test_file}__iterations_{iterations}.png")  # Unique filename
        plt.close()  # Close the plot to avoid displaying

    # Plotting for each value of warmup
    for warmup in warmup_values:
        plt.figure(figsize=(12, 8))
        for iterations in iterations_values:
            subset = data[(data['Warmup'] == warmup) & (data['Iterations'] == iterations)]
            plt.plot(subset['Size'], subset['Avg Latency(us)'], label=f'Avg Latency (Iterations {iterations})', marker='o')
            plt.plot(subset['Size'], subset['Min Latency(us)'], label=f'Min Latency (Iterations {iterations})', marker='o')
            plt.plot(subset['Size'], subset['Max Latency(us)'], label=f'Max Latency (Iterations {iterations})', marker='o')

        plt.title(f'{operation}, {algorithm}, {mapping}  --- Warmup = {warmup}')
        plt.xlabel('Size')
        plt.ylabel('Latency (us)')
        plt.xscale('log')
        plt.yscale('log')
        plt.legend()
        plt.grid(True)
        plt.savefig(f"test__{test_file}__warmup_{warmup}.png")  # Unique filename
        plt.close()  # Close the plot to avoid displaying

if __name__ == "__main__":
    # If running the script from the command line, you can pass the filename as an argument
    if len(sys.argv) > 1:
        test_file = sys.argv[1]  # e.g., 'data' if the file is named 'test__data.csv'
    else:
        test_file = 1  # Default file name if no argument is given
    main(test_file)
