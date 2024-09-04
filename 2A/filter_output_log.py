from datetime import datetime

# Define the paths to your files
memory_log_file = "output.log"  # Replace with your actual file name
event_log_file = "steps.log"  # Replace with your actual file name
output_file = "output.log"

# Function to parse the date from the log
def parse_date(date_str):
    # return datetime.strptime(date_str, "%a %b %d %H:%M:%S %Z %Y")
    return datetime.strptime(date_str, "%a %b %d %H:%M:%S.%f %Z %Y")

# Read the timestamps from the second file
with open(event_log_file, "r") as f:
    event_lines = f.readlines()

# Extract the timestamps
event_timestamps = []
for line in event_lines:
    if ":" in line:
        time_str = line.split(" at: ")[-1].strip()
        event_timestamps.append(parse_date(time_str))

# Read the memory log file and filter relevant lines
with open(memory_log_file, "r") as f:
    memory_lines = f.readlines()

# Prepare the output
filtered_lines = []
i = 0
while i < len(memory_lines):
    line = memory_lines[i].strip()
    if len(line) > 0 and line[0].isalpha():
        # Try to parse the date
        try:
            log_time = parse_date(line.rstrip(":"))
            if log_time in event_timestamps:
                # If the date matches, retain this and the next two lines
                filtered_lines.append("\n" + line + "\n")
                filtered_lines.append(memory_lines[i + 1])
                filtered_lines.append(memory_lines[i + 2])
        except ValueError:
            pass
    i += 1

# Write the filtered lines to a new output file
with open(output_file, "w") as f:
    f.writelines(filtered_lines)

print(f"Filtered memory log saved to {output_file}")
