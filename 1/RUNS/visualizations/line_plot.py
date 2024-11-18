import sys
import pandas as pd
import matplotlib.pyplot as plt

def main(file_name):
    data = pd.read_csv(file_name)
    plt.figure(figsize=(10, 6))

    for procs in data['Procs'].unique():
        subset = data[data['Procs'] == procs]
        plt.plot(subset['Size'], subset['Avg Latency(us) Mean'], label=f"{procs} Procs")

    plt.xlabel("Message Size (bytes)")
    plt.ylabel("Avg Latency (us)")
    plt.title("Average Latency vs. Message Size")
    plt.legend()
    plt.grid()
    plt.savefig("plot1.png")
    print("Plot saved as plot1.png")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot1.py <file_name>")
    else:
        main(sys.argv[1])
