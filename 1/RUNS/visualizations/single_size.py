import sys
import pandas as pd
import matplotlib.pyplot as plt

def main(file_name):
    data = pd.read_csv(file_name)
    message_size = data['Size'].unique()[0]  # Adjust for a specific message size
    subset = data[data['Size'] == message_size]

    plt.figure(figsize=(10, 6))
    plt.plot(subset['Procs'], subset['Avg Latency(us) Mean'], marker='o')

    plt.xlabel("Number of Processors")
    plt.ylabel("Avg Latency (us) Mean")
    plt.title(f"Average Latency vs. Number of Processors (Message Size: {message_size} bytes)")
    plt.grid()
    plt.savefig("plot3.png")
    print("Plot saved as plot3.png")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot3.py <file_name>")
    else:
        main(sys.argv[1])
