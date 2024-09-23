import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys

def create_graphs(input_file):
    # Read the CSV file into a pandas DataFrame
    csv_file = str(input_file + ".csv")
    df = pd.read_csv(csv_file)
    
    # Get unique algorithms
    algorithms = df['Algorithm'].unique()

    # Set the plot style
    sns.set(style="whitegrid")

    for algo in algorithms:
        # Filter the DataFrame for the current algorithm
        algo_df = df[df['Algorithm'] == algo]

        # Create a line plot
        plt.figure(figsize=(10, 6))
        sns.lineplot(x='Size', y='Avg Latency(us)', hue='Mapping', data=algo_df, marker="o")

        # Set plot title and labels
        plt.title(f'Latency Comparison for Algorithm {algo}', fontsize=16)
        plt.xlabel('Size', fontsize=14)
        plt.ylabel('Average Latency (us)', fontsize=14)

        # Save the plot as an image file
        plt.legend(title='Mapping')
        plt.savefig(str(input_file + ".png"))

        # Display the plot
        plt.show()

if __name__ == "__main__":
    # Assuming the CSV file is named "output.csv"
    create_graphs(sys.argv[1])
