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
    iterations_values = [10000, 50000]  # Focus on 10000 and 100000 for iterations
    warmup_values = [200, 1000]         # Focus on 200 and 1000 for warmup

    # Define specific colors for different parameter configurations
    color_mapping = {
        10000: 'blue',   
        50000: 'red', 
        200: 'purple',  
        1000: 'yellow',    
    }

    # Use Matplotlib's built-in math text rendering (not full LaTeX)
    plt.rcParams['text.usetex'] = False

    # Create a figure with two subplots sharing the same x-axis (stacked vertically)
    fig, (ax_top, ax_bottom) = plt.subplots(2, 1, figsize=(10, 10), sharex=True, constrained_layout=True)

    #### Bottom Plot: Fix warmup = 200, vary iterations ####
    warmup_fixed = 200
    for iterations in iterations_values:
        subset = data[(data['Warmup'] == warmup_fixed) & (data['Iterations'] == iterations)]

        # Plot average latency on log-log scale
        ax_bottom.plot(subset['Size'], subset['Avg Latency(us)'], 
                       label=f'{iterations}', 
                       color=color_mapping[iterations], marker='o', linestyle='-', linewidth=1)
        
        # Fill between for range (Min, Max latency)
        ax_bottom.fill_between(subset['Size'], subset['Min Latency(us)'], subset['Max Latency(us)'], 
                               color=color_mapping[iterations], alpha=0.2)

    ax_bottom.set_xlabel('Message Size (Bytes)')
    ax_bottom.set_ylabel('Latency (us)')
    ax_bottom.set_xscale('log')
    ax_bottom.set_yscale('log')
    ax_bottom.set_title('Latency vs Message Size (Fixed Warmup = 200, Varying Iterations)', fontsize=10)
    ax_bottom.grid(True, which="both", linestyle='--')
    ax_bottom.legend(title='Iterations')

    #### Top Plot: Fix iterations = 10000, vary warmup ####
    iterations_fixed = 10000
    for warmup in warmup_values:
        subset = data[(data['Iterations'] == iterations_fixed) & (data['Warmup'] == warmup)]

        # Plot average latency on log-log scale
        ax_top.plot(subset['Size'], subset['Avg Latency(us)'], 
                    label=f'{warmup}', 
                    color=color_mapping[warmup], marker='o', linestyle='-', linewidth=1)
        
        # Fill between for range (Min, Max latency)
        ax_top.fill_between(subset['Size'], subset['Min Latency(us)'], subset['Max Latency(us)'], 
                            color=color_mapping[warmup], alpha=0.2)

    ax_top.set_ylabel('Latency (us)')
    ax_top.set_xscale('log')
    ax_top.set_yscale('log')
    ax_top.set_title('Latency vs Message Size (Fixed Iterations = 10000, Varying Warmup)', fontsize=10)
    ax_top.grid(True, which="both", linestyle='--')
    ax_top.legend(title='Warmup')

    # Shared x-axis label
    plt.suptitle(f'Operation: {operation[0]}, Algorithm: {algorithm[0]}, Mapping: {mapping[0]}', ha='center')

    # Save the plot as an image file
    plt.savefig(f"test__{test_file}_double_vertical_plot.png")
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        test_file = sys.argv[1]
    else:
        test_file = 1
    main(test_file)
