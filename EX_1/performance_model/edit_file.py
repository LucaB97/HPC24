import sys

def extract_latency_data(input_file):
    rank_ids = []
    latencies = []
    
    with open(input_file, 'r') as file:
        lines = file.readlines()
        for i in range(len(lines)):
            if lines[i].startswith("Message to core:"):
                # Extract the rank ID
                rank_id = lines[i].strip().split(": ")[1]
                rank_ids.append(rank_id)
                
                # Extract the latency
                latency_line = lines[i+4].strip()  # The latency is typically 4 lines down
                latency = latency_line.split()[-1]
                latencies.append(latency)
    
    # Write the data to a new file with tab-separated values
    with open(input_file, 'w') as out_file:
        out_file.write("Rank ID\tLatency (us)\n")
        for rank_id, latency in zip(rank_ids, latencies):
            out_file.write(f"{rank_id}\t{latency}\n")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 edit_file.py <input_file>")
        sys.exit(1)
    
    input_filename = sys.argv[1]
    
    extract_latency_data(input_filename)
