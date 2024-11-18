import pandas as pd
import sys

def compute_statistics(input_file, output_file):
    """
    Reads a CSV input file, computes the average and standard deviation of times
    grouped by thread counts, and writes the results to a specified output file.
    """
    try:
        # Step 1: Read the input CSV file
        df = pd.read_csv(input_file, header=None, names=["Threads", "Time"])
        
        # Step 2: Group by thread count and calculate statistics
        summary = df.groupby("Threads")["Time"].agg(["mean", "std"]).reset_index()
        summary.rename(columns={"mean": "Average", "std": "Standard Deviation"}, inplace=True)
        
        # Step 3: Round the results to 3 decimal places
        summary["Average"] = summary["Average"].round(3)
        summary["Standard Deviation"] = summary["Standard Deviation"].round(3)
        
        # Step 4: Write the output to a file
        summary.to_csv(output_file, index=False)
        print(f"Statistics written to {output_file}")
    except FileNotFoundError:
        print(f"Error: File '{input_file}' not found.")
        sys.exit(1)
    except Exception as e:
        print(f"An error occurred: {e}")
        sys.exit(1)

def main():
    """
    Main method to execute the script.
    Expects input and output file names as command-line arguments.
    """
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    compute_statistics(input_file, output_file)

if __name__ == "__main__":
    main()
