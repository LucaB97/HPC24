import sys
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

def main(file_name):
    data = pd.read_csv(file_name)
    
    # Ensure required columns exist
    required_columns = ['Procs', 'Size', 'Avg Latency(us) Mean']
    for col in required_columns:
        if col not in data.columns:
            print(f"Error: Missing required column '{col}' in the CSV file.")
            return

    # Pivot the data for heatmap
    pivot_table = data.pivot(index='Size', columns='Procs', values='Avg Latency(us) Mean')
    
    # Plot heatmap
    plt.figure(figsize=(12, 8))
    sns.heatmap(
        pivot_table,
        cmap="coolwarm",
        annot=False,
        fmt=".2f",
        cbar_kws={'label': 'Avg Latency (us)'},
        norm=None  # Adjust norm or use LogNorm for log scaling if needed
    )
    plt.title("Heatmap of Avg Latency")
    plt.xlabel("Procs")
    plt.ylabel("Size")
    plt.tight_layout()
    plt.savefig("heatmap.png")
    print("Heatmap saved as heatmap.png")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python heatmap.py <file_name>")
    else:
        main(sys.argv[1])
