import plotly.graph_objects as go
import numpy as np
import sys
import pandas as pd

def main(file_name):
    data = pd.read_csv(file_name)
    # Filter data for a single Algorithm for better clarity
    # data = data[data['Algorithm'] == 0]

    # Create meshgrid for Procs and Size
    sizes = sorted(data['Size'].unique())
    procs = sorted(data['Procs'].unique())
    Z = data.pivot(index='Size', columns='Procs', values='Avg Latency(us) Mean').values
    print("Z shape:", Z.shape)
    print("Number of Sizes (x-axis):", len(sizes))
    print("Number of Procs (y-axis):", len(procs))

    # Create the 3D surface plot
    fig = go.Figure(data=[go.Surface(z=Z, x=np.log10(sizes), y=procs)])
    fig.update_layout(
        title='3D Surface Plot of Avg Latency',
        scene=dict(
            xaxis_title='Log(Size)',
            yaxis_title='Procs',
            zaxis_title='Avg Latency (us)'
        )
    )
    
    # # Save the plot as a PNG file
    # fig.write_image("3d_surface_plot.png")  # Save as PNG
    # print("3D Surface Plot saved as 3d_surface_plot.png")

    # Optionally, save as an interactive HTML file
    fig.write_html("3d_surface_plot.html")  # Save as HTML
    print("3D Surface Plot saved as 3d_surface_plot.html")

    # Show the plot interactively
    # fig.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot3.py <file_name>")
    else:
        main(sys.argv[1])
