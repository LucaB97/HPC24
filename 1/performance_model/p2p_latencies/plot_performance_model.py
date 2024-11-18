import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read the data
data = pd.read_csv('32768.csv')

# Create a mapping for algorithm labels
algorithm_labels = {
    0: 'baseline',
    2: 'chain',
    3: 'pipeline',
    5: 'binary tree',
    -1: 'naive'
}

# Filter out rows with missing latency values
data = data.dropna(subset=['Avg Latency(us) Mean', 'Avg Latency(us) Std Error'])

# Plot setup
plt.figure(figsize=(10, 6))

# Loop through each algorithm and plot its data
for algorithm in data['Algorithm'].unique():
    algo_data = data[data['Algorithm'] == algorithm]
    
    # Plot the points for Avg Latency
    plt.errorbar(algo_data['Procs'], algo_data['Avg Latency(us) Mean'], 
                 yerr=algo_data['Avg Latency(us) Std Error'], fmt='-o', 
                 label=algorithm_labels.get(algorithm, f'Alg {algorithm}'), 
                 capsize=3, elinewidth=0.5, markersize=5)
    
# Adding labels and title
plt.xlabel('Number of Processors (Procs)', fontsize=12)
plt.ylabel('Average Latency (us)', fontsize=12)
plt.title('1024 bytes', fontsize=14)
plt.legend(title="Algorithm", loc='upper left', bbox_to_anchor=(1, 1))

# Show the plot
plt.tight_layout()
plt.show()
# Save the plot as a PNG file
plt.savefig('32768.png', format='png', dpi=300)  # Save the plot with high resolution (300 dpi)