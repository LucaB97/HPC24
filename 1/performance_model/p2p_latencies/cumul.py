import pandas as pd

# Read the data from the file
df = pd.read_csv("1048576.txt", delimiter="\t")

df["Algorithm"] = -1
df["Size"] = 1048576
# Calculate the cumulative sum of latency
df["Cumulative Latency (us)"] = df["Latency (us)"].cumsum()

# Starting counter from 2 (we will modify the index to start from 2)
df["Counter"] = range(2, len(df) + 2)
df["Sd"] = 0

# Select only the "Counter" and "Cumulative Latency (us)" columns
df = df[["Algorithm", "Counter", "Size", "Cumulative Latency (us)", "Sd"]]

# Save the result to a CSV file
output_file_name = "cumulative_latency.csv"
df.to_csv(output_file_name, index=False)

print(f"CSV file saved to {output_file_name}")



