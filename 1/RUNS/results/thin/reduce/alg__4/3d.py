import pandas as pd
import plotly.graph_objects as go
import numpy as np

# Read data from CSV file
df = pd.read_csv('summarized2.csv')

# Extract the relevant columns
procs = df['Procs'].to_numpy()
size = df['Size'].to_numpy()
latency = df['Avg Latency(us) Mean'].to_numpy()

# Apply the logarithmic transformation to the 'Size' values (log base 10)
log_size = np.log10(size)

# Create a meshgrid for plotting using the log-transformed size
unique_procs = np.unique(procs)
unique_log_sizes = np.unique(log_size)
X, Y = np.meshgrid(unique_log_sizes, unique_procs)
print(unique_log_sizes)
print(X.shape)
# Create a grid of latency values (initialize with NaNs for missing data)
Z = np.full(X.shape, np.nan)

# Fill the Z array with corresponding latency values
for i, proc in enumerate(unique_procs):
    for j, log_sz in enumerate(unique_log_sizes):
        # Find the corresponding latency value for each proc and log-transformed size
        latency_value = latency[(procs == proc) & (log_size == log_sz)]
        if len(latency_value) > 0:
            Z[i, j] = latency_value[0]  # Only assign if data is available

# Create the plotly 3D surface plot
fig = go.Figure(data=[go.Surface(
    z=Z,
    x=X,
    y=Y,
    colorscale='Viridis',
    colorbar_title='Avg Latency (us)',
    showscale=True
)])

# Set axis labels, including log-transformed size
fig.update_layout(
    scene=dict(
        xaxis_title='Log(Size)',  # Update axis label for log-transformed size
        yaxis_title='Procs',
        zaxis_title='Avg Latency (us)'
    ),
    title='3D Surface Plot of Latency vs Log(Size) and Procs'
)

# Save as HTML (interactive)
fig.write_html("interactive_3d_plot_log_size.html")
