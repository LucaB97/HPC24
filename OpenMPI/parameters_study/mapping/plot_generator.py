import pandas as pd
import numpy as np
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
    threshold_1 = 16384
    threshold_2 = 131072

    # Define specific colors for different parameter configurations
    color_mapping = {
        0: 'blue',   
        1: 'orange', 
        2: 'green',  
        3: 'purple',    
        4: 'red',
    }

    # Use Matplotlib's built-in math text rendering (not full LaTeX)
    plt.rcParams['text.usetex'] = False

    # Plotting for each value of iterations
    iterations = np.min(iterations_values)
    fig, axs = plt.subplots(1, 3, figsize=(12, 9), constrained_layout=True)  # Two subplots (2 rows, 1 column)
        
    for i, warmup in enumerate(warmup_values):
        color = color_mapping.get(i, 'black')  # Default to black if index out of range

        # Subset data for size < size_threshold
        subset_small = data[(data['Iterations'] == iterations) & (data['Warmup'] == warmup) & (data['Size'] < threshold_1) & (data['Size'] != 256)]
        axs[0].plot(subset_small['Size'], subset_small['Avg Latency(us)'], label=f'Avg Latency (Warmup {warmup})', color=color, marker='o', linewidth=2)
        axs[0].fill_between(subset_small['Size'], subset_small['Min Latency(us)'], subset_small['Max Latency(us)'], color=color, alpha=0.2)
        axs[0].plot([], [], color=color, linestyle='-', label=f'Range (Warmup {warmup})')  # Dummy plot for the legend

        # Subset data for size < size_threshold
        subset_medium = data[(data['Iterations'] == iterations) & (data['Warmup'] == warmup) & (data['Size'] >= threshold_1) & (data['Size'] < threshold_2)]
        axs[1].plot(subset_medium['Size'], subset_medium['Avg Latency(us)'], label=f'Avg Latency (Warmup {warmup})', color=color, marker='o')
        axs[1].fill_between(subset_medium['Size'], subset_medium['Min Latency(us)'], subset_medium['Max Latency(us)'], color=color, alpha=0.2)
        axs[1].plot([], [], color=color, linestyle='-', label=f'Range (Warmup {warmup})')  # Dummy plot for the legend

        # Subset data for size >= size_threshold
        subset_large = data[(data['Iterations'] == iterations) & (data['Warmup'] == warmup) & (data['Size'] >= threshold_2)]
        axs[2].plot(subset_large['Size'], subset_large['Avg Latency(us)'], label=f'Avg Latency (Warmup {warmup})', color=color, marker='o')
        axs[2].fill_between(subset_large['Size'], subset_large['Min Latency(us)'], subset_large['Max Latency(us)'], color=color, alpha=0.2)
        axs[2].plot([], [], color=color, linestyle='-', label=f'Range (Warmup {warmup})')  # Dummy plot for the legend

    # Titles with LaTeX-like formatting using built-in math text
    axs[0].set_title(rf'$\mathrm{{Size < }}$ {threshold_1}$B$')
    axs[1].set_title(rf'{threshold_1}$B$ $\mathrm{{\leq Size < }}$ {threshold_2}$B$')
    axs[2].set_title(rf'$\mathrm{{Size \geq }}$ {threshold_2}$B$')
        
    axs[0].set_xticks(subset_small['Size'])  # Set x-ticks to the data points
    axs[1].set_xticks(subset_medium['Size'])  # Set x-ticks to the data points
    axs[2].set_xticks(subset_large['Size'])  # Set x-ticks to the data points

    for ax in axs:
        ax.set_xlabel('Message size (bytes)')
        ax.set_ylabel('Latency (us)')
        # ax.set_xscale('log')
        # ax.set_yscale('log')
        ax.get_xaxis().set_major_formatter(plt.ScalarFormatter())  # Use scalar formatting for readability
        ax.legend()
        ax.grid(True)
    
    plt.suptitle(rf'$\mathrm{{Operation:}}$ {operation}, $\mathrm{{Algorithm:}}$ {algorithm}, $\mathrm{{Mapping:}}$ {mapping}' + '\n' + rf'$\mathrm{{Iterations:}}$ {iterations}' + ' (default)\n\n', ha='center')
    # plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig(f"test__{test_file}__iterations_{iterations}.png")  # Unique filename
    plt.close()

    # Plotting for each value of warmup
    warmup = np.min(warmup_values)
    fig, axs = plt.subplots(1, 3, figsize=(12, 9), constrained_layout=True)  # Two subplots (2 rows, 1 column)

    for i, iterations in enumerate(iterations_values):
        color = color_mapping.get(i+2, 'black')  # Default to black if index out of range

        # Subset data for size < size_threshold
        subset_small = data[(data['Warmup'] == warmup) & (data['Iterations'] == iterations) & (data['Size'] < threshold_1) & (data['Size'] != 256)]
        axs[0].plot(subset_small['Size'], subset_small['Avg Latency(us)'], label=f'Avg Latency (Iterations {iterations})', color=color, marker='o')
        axs[0].fill_between(subset_small['Size'], subset_small['Min Latency(us)'], subset_small['Max Latency(us)'], color=color, alpha=0.2)
        axs[0].plot([], [], color=color, linestyle='-', label=f'Range (Iterations {iterations})')  # Dummy plot for the legend

        # Subset data for size < size_threshold
        subset_medium = data[(data['Warmup'] == warmup) & (data['Iterations'] == iterations) & (data['Size'] >= threshold_1) & (data['Size'] < threshold_2)]
        axs[1].plot(subset_medium['Size'], subset_medium['Avg Latency(us)'], label=f'Avg Latency (Iterations {iterations})', color=color, marker='o')
        axs[1].fill_between(subset_medium['Size'], subset_medium['Min Latency(us)'], subset_medium['Max Latency(us)'], color=color, alpha=0.2)
        axs[1].plot([], [], color=color, linestyle='-', label=f'Range (Iterations {iterations})')  # Dummy plot for the legend

        # Subset data for size >= size_threshold
        subset_large = data[(data['Warmup'] == warmup) & (data['Iterations'] == iterations) & (data['Size'] >= threshold_2)]
        axs[2].plot(subset_large['Size'], subset_large['Avg Latency(us)'], label=f'Avg Latency (Iterations {iterations})', color=color, marker='o', linewidth=2)
        axs[2].fill_between(subset_large['Size'], subset_large['Min Latency(us)'], subset_large['Max Latency(us)'], color=color, alpha=0.2)
        axs[2].plot([], [], color=color, linestyle='-', label=f'Range (Iterations {iterations})')  # Dummy plot for the legend

    # Titles with LaTeX-like formatting using built-in math text
    axs[0].set_title(rf'$\mathrm{{Size < }}$ {threshold_1}$B$')
    axs[1].set_title(rf'{threshold_1}$B$ $\mathrm{{\leq Size < }}$ {threshold_2}$B$')
    axs[2].set_title(rf'$\mathrm{{Size \geq }}$ {threshold_2}$B$')
        
    axs[0].set_xticks(subset_small['Size'])  # Set x-ticks to the data points
    axs[1].set_xticks(subset_medium['Size'])  # Set x-ticks to the data points
    axs[2].set_xticks(subset_large['Size'])  # Set x-ticks to the data points

    for ax in axs:
        ax.set_xlabel('Message size (bytes)')
        ax.set_ylabel('Latency (us)')
        # ax.set_xscale('log')
        # ax.set_yscale('log')
        ax.get_xaxis().set_major_formatter(plt.ScalarFormatter())  # Use scalar formatting for readability
        ax.legend()
        ax.grid(True)

    plt.suptitle(rf'$\mathrm{{Operation:}}$ {operation}, $\mathrm{{Algorithm:}}$ {algorithm}, $\mathrm{{Mapping:}}$ {mapping}' + '\n' + rf'$\mathrm{{Warmup:}}$ {warmup}' + ' (default)\n\n', ha='center')
    # plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig(f"test__{test_file}__warmup_{warmup}.png")  # Unique filename
    plt.close()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        test_file = sys.argv[1]
    else:
        test_file = 1
    main(test_file)
