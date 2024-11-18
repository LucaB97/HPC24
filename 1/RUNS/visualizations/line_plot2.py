import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys

def main(file_name):
    # Load data into a pandas DataFrame
    data = pd.read_csv(file_name)  # Replace 'data.csv' with your file name
    data['Size'] = data['Size'].astype(int)

    # Plot settings
    plt.figure(figsize=(12, 8))
    for (algorithm, procs), group in data.groupby(['Algorithm', 'Procs']):
        plt.errorbar(
            group['Size'], group['Avg Latency(us) Mean'], yerr=group['Avg Latency(us) Std Error'],
            label=f'Algorithm {algorithm}, Procs {procs}', capsize=3
        )

    plt.xscale('log')
    plt.xlabel('Size (log scale)')
    plt.ylabel('Avg Latency (us)')
    plt.title('Latency Trends Across Sizes')
    plt.legend()
    plt.grid()
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot1.py <file_name>")
    else:
        main(sys.argv[1])