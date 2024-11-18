import re
import csv
import numpy as np
import sys 

def parse_latency_data(file_content):
    data = []
    algorithm = None
    procs = None
    test = None
    
    for line in file_content.splitlines():
        line = line.strip()
        
        if line.startswith('Algorithm:'):
            algorithm = int(line.split(':')[1].strip())
        
        elif line.startswith('Procs:'):
            procs = int(line.split(':')[1].strip())
        
        elif line.startswith('Test:'):
            test = int(line.split(':')[1].strip())
        
        elif re.match(r'^\d+', line):  # Line starts with a digit (Size)
            values = re.split(r'\s+', line)
            size = int(values[0])
            avg_latency = float(values[1])
            
            data_point = {
                'Algorithm': algorithm,
                'Procs': procs,
                'Test': test,
                'Size': size,
                'Avg Latency(us)': avg_latency,
            }
            
            data.append(data_point)
    
    return data

def compute_statistics(data):
    stats = {}
    
    for entry in data:
        key = (entry['Algorithm'], entry['Procs'], entry['Size'])
        if key not in stats:
            stats[key] = {
                'Avg Latency(us)': [],
            }
        
        for metric in stats[key]:
            stats[key][metric].append(entry[metric])
    
    results = []
    for key, metrics in stats.items():
        algorithm, procs, size = key
        
        result = {
            'Algorithm': algorithm,
            'Procs': procs,
            'Size': size,
            'Avg Latency(us) Mean': np.mean(metrics['Avg Latency(us)']),
            'Avg Latency(us) Std Error': np.std(metrics['Avg Latency(us)'], ddof=1) / np.sqrt(len(metrics['Avg Latency(us)'])),
        }
        
        results.append(result)
    
    return results

def write_to_csv(data, output_file):
    if data:
        headers = data[0].keys()
        
        with open(output_file, 'w', newline='') as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=headers)
            
            writer.writeheader()
            writer.writerows(data)


if __name__ == "__main__":
    file_path = sys.argv[1]
    output_file = sys.argv[2]  

    with open(file_path, 'r') as file:
        file_content = file.read()

    parsed_data = parse_latency_data(file_content)
    statistics_data = compute_statistics(parsed_data)
    write_to_csv(statistics_data, output_file)

    print(f"Data with statistics has been written to {output_file}")
