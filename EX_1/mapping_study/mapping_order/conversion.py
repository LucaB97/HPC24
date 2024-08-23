import csv

def convert_csv_to_txt(input_file, output_file):
    """
    Converts a CSV file to a TXT file with tab-separated values.

    Parameters:
    input_file (str): The name of the input CSV file.
    output_file (str): The name of the output TXT file.
    """
    try:
        # Open the input CSV file
        with open(input_file, 'r') as csv_file:
            csv_reader = csv.reader(csv_file)
            
            # Open the output TXT file
            with open(output_file, 'w') as txt_file:
                for row in csv_reader:
                    # Join the row data with tabs and write to the TXT file
                    txt_file.write("\t".join(row) + "\n")
        
        print(f"File '{input_file}' has been successfully converted to '{output_file}'.")
    
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage:
input_filename = input("Enter the input CSV file name (with .csv extension): ")
output_filename = input("Enter the output TXT file name (with .txt extension): ")

convert_csv_to_txt(input_filename, output_filename)
