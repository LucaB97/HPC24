import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Function to read the CSV and plot the graph
def plot_latency_with_std(csv_file):
    # Read the CSV file
    df = pd.read_csv(csv_file)
    
    # Map algorithm codes to labels
    algorithm_labels = {
        0: "baseline",
        2: "chain",
        3: "pipeline",
        5: "binary tree",
        -1: "Naive"
    }
    
    # Apply labels to the 'Algorithm' column
    df['Algorithm'] = df['Algorithm'].map(algorithm_labels)
    
    # Set up the plot style and colors
    sns.set(style="whitegrid")
    plt.figure(figsize=(10, 6))
    
    # Create the plot for each algorithm
    for algorithm in df['Algorithm'].unique():
        algorithm_data = df[df['Algorithm'] == algorithm]
        
        # Plotting the mean latency with a thick line and dots
        plt.plot(algorithm_data['Procs'], algorithm_data['Avg Latency(us) Mean'], marker='o', label=algorithm, linewidth=1)
        
        # Plotting the standard deviation as a shaded area if available
        if 'Avg Latency(us) Std Error' in algorithm_data.columns:
            plt.fill_between(algorithm_data['Procs'], 
                             algorithm_data['Avg Latency(us) Mean'] - algorithm_data['Avg Latency(us) Std Error'], 
                             algorithm_data['Avg Latency(us) Mean'] + algorithm_data['Avg Latency(us) Std Error'], 
                             alpha=0.2)  # Shaded area with 20% transparency

    # Add labels and title
    plt.xlabel('Number of Procs', fontsize=12)
    plt.ylabel('Average Latency (us)', fontsize=12)
    plt.title('1048576 bytes', fontsize=14)
    
    # Show the legend
    plt.legend(title="Algorithm", fontsize=10)
    
    # Show the plot
    plt.tight_layout()
    # plt.show()
    plt.savefig('1048576.png', format='png', dpi=300)

# Call the function with the path to your CSV file
plot_latency_with_std('1048576.csv')
