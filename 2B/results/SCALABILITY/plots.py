import matplotlib.pyplot as plt
import pandas as pd
import sys

def read_data(input_file):
    """
    Reads the input file and returns a pandas DataFrame.
    Assumes the file has space-separated values with headers.
    """
    try:
        df = pd.read_csv(input_file, sep="\s+", header=0)
        return df
    except Exception as e:
        print(f"Error reading the file: {e}")
        sys.exit(1)

def plot_data(df, output_file):
    """
    Plots the data with the specified formatting and saves the image as a PNG file.
    - 'Total' is plotted as a thicker black line with dots and annotated with 'Processes'.
    - 'Communication', 'Sorting', 'Merging' are plotted as thinner colored lines with dots.
    """
    plt.figure(figsize=(10, 6))

    # Plot the 'Total' time as a thicker black line with dots
    plt.plot(df["Processes"], df["Total"], color="black", label="Total", linewidth=2, marker="o", markersize=6)

    # Plot the components ('Communication', 'Sorting', 'Merging') as thinner colored lines with dots
    plt.plot(df["Processes"], df["Communication"], label="Communication", color="blue", linestyle="--", linewidth=1, marker="o", markersize=6)
    plt.plot(df["Processes"], df["Sorting"], label="Sorting", color="green", linestyle="--", linewidth=1, marker="o", markersize=6)
    plt.plot(df["Processes"], df["Merging"], label="Merging", color="red", linestyle="--", linewidth=1, marker="o", markersize=6)

    for i in range(len(df)):
        plt.text(df["Processes"][i], df["Total"][i], f'{df["Processes"][i]}', 
                fontsize=9, color="black", ha="center", va="bottom")

    # Add labels and title
    plt.xlabel("Processes")
    plt.ylabel("Time (sec)")
    plt.title(r"$N=10^6*Processes$", fontsize=14)

    # Set a log scale for the x-axis (optional, depending on how your data is structured)
    plt.xscale("log")

    # Add grid, legend, and other plot details
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.legend()

    # Save the plot as a PNG file
    plt.tight_layout()
    plt.savefig(output_file, format="png")

    print(f"Plot saved as {output_file}")

def main():
    # Check if the script is being run with the correct number of arguments
    if len(sys.argv) != 3:
        print("Usage: python process_data.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]  # Input file path from command line arguments
    output_file = sys.argv[2]  # Output file path for the PNG image

    # Read the data from the input file
    df = read_data(input_file)

    # Plot the data and save it as a PNG image
    plot_data(df, output_file)

if __name__ == "__main__":
    # Entry point of the script
    main()
