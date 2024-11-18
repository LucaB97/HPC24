import seaborn as sns
import matplotlib.pyplot as plt
import sys
import pandas as pd

def main(file_name):
    data = pd.read_csv(file_name)
    # Create a FacetGrid for Line Plots
    g = sns.FacetGrid(data, row='Algorithm', col='Procs', margin_titles=True, sharey=False, sharex=True, height=4)
    g.map(
        sns.lineplot, 'Size', 'Avg Latency(us) Mean', marker="o"
    )
    g.map(
        plt.errorbar, 'Size', 'Avg Latency(us) Mean', 'Avg Latency(us) Std Error', fmt='none', capsize=3
    )

    for ax in g.axes.flat:
        ax.set_xscale('log')

    g.set_axis_labels("Size (log scale)", "Avg Latency (us)")
    g.set_titles(row_template="Algorithm {row_name}", col_template="Procs {col_name}")
    plt.subplots_adjust(top=0.9)
    g.fig.suptitle("Latency Trends by Algorithm and Processor Count")
    # plt.show()
    plt.savefig("facets.png")
    print("Plot saved as facets.png")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot3.py <file_name>")
    else:
        main(sys.argv[1])

