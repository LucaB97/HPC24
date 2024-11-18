import pandas as pd

# Load the CSV file
filter = [1048576]
file_name = "performance.csv"  # Replace with your actual file name
df = pd.read_csv(file_name)

# Filter rows where Size is 1024, 32768, or 1048576
filtered_df = df[df['Size'].isin(filter)]

# Save the filtered rows to a new CSV file
filtered_file_name = "1048576.csv"  # Name of the new file
filtered_df.to_csv(filtered_file_name, index=False)

print(f"Filtered data saved to {filtered_file_name}")
