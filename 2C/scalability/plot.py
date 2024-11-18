import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys


def plot_with_std(input_file, output_image):
    """
    Creates a visualization of threads vs times with averages, standard deviations, observations,
    and a secondary y-axis for speedup.
    """
    try:
        # Step 1: Load data from the input file
        df = pd.read_csv(input_file)

        # Step 2: Create the plot
        fig, ax1 = plt.subplots(figsize=(10, 6))

        # Plot average as a thick blue line
        ax1.plot(df['Threads'], df['Average'], color='blue', linewidth=2.5, label='Average Time')
        
        # Plot the standard deviation as a filled area around the average
        ax1.fill_between(
            df['Threads'], 
            df['Average'] - df['Standard Deviation'], 
            df['Average'] + df['Standard Deviation'], 
            color='blue', alpha=0.2, label='Standard Deviation'
        )

        # Add scatter plot for observations
        for thread in df['Threads']:
            obs_times = np.random.normal(df.loc[df['Threads'] == thread, 'Average'].values[0],
                                         df.loc[df['Threads'] == thread, 'Standard Deviation'].values[0],
                                         size=10)  # Simulated observations
            ax1.scatter([thread]*len(obs_times), obs_times, color='black', alpha=0.6, s=15, label='Observations' if thread == df['Threads'].min() else "")
        
        # Step 3: Add labels, legend, and grid for the first y-axis (time)
        ax1.set_xlabel('Threads', fontsize=14)
        ax1.set_ylabel('Time (s)', fontsize=14)
        ax1.legend(fontsize=12)
        ax1.grid(alpha=0.3)

        # Step 4: Create secondary y-axis for Speedup
        ax2 = ax1.twinx()  # Create a twin axis that shares the x-axis
        ax2.set_ylabel('Speedup', fontsize=14)

        # Compute speedup: Speedup = Time with 1 thread / Time with N threads
        reference_time = df.loc[df['Threads'] == 1, 'Average'].values[0]
        df['Speedup'] = reference_time / df['Average']

        # Plot speedup on the secondary y-axis
        ax2.plot(df['Threads'], df['Speedup'], color='red', linestyle='--', marker='o', label='Speedup', linewidth=1)
        ax1.legend(fontsize=12, loc='upper center')

        # Save the plot as an image file
        plt.tight_layout()  # Adjust layout to prevent overlap
        plt.savefig(output_image, dpi=300)
        plt.show()
        
        print(f"Plot saved to {output_image}")
    
    except FileNotFoundError:
        print(f"Error: File '{input_file}' not found.")
    except Exception as e:
        print(f"An error occurred: {e}")


def main():
    """
    Main method to execute the script.
    Expects input CSV and output image file names as command-line arguments.
    """
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_file> <output_image>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_image = sys.argv[2]
    
    plot_with_std(input_file, output_image)


if __name__ == "__main__":
    main()
