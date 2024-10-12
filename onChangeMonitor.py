import os
import time
from datetime import datetime
from binning import binner
from multi_threaded_preprocessor import read_preprocessor


# Function to scan the directory and return a dictionary of files with their modification times
def scan_directory(directory):
    files = {}
    for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if os.path.isfile(filepath):
            files[filename] = os.path.getmtime(filepath)  # Get the modification time
    return files

# this parameter presents the lib length of 140
lib_length = 140

# path to the sequencing output - can be .txt or a .fastq file or a path to a folder with many .fastq files
# this is the file the includes all the basecalled reads that we want to processed
reads_path = "fastq_runid.fastq"

design_file_path = "./deep_design.csv"

# Function to monitor the directory for changes
def monitor_directory(directory, interval=5):
    print(f"Monitoring directory: {directory}")

    previous_files = scan_directory(directory)  # Initial snapshot of the directory
    # path (including file name) in which the trimmed reads will be stored
    while True:
        time.sleep(interval)  # Wait for the specified interval before scanning again

        current_files = scan_directory(directory)  # Scan the directory again

        # Check for new files
        new_files = [f for f in current_files if f not in previous_files]
        if new_files:
            for file in new_files:
                file = f'mock_data/{file}'
                print(f"New files added: {file}")
                reads_trimmed_path = "processed_data/reads_trimmed_" + datetime.today().strftime('%Y_%m_%d') + ".txt"
                read_preprocessor(file, reads_trimmed_path, lib_length)
                binner(design_file_path=design_file_path, reads_file_path=reads_trimmed_path)

        # Check for deleted files
        deleted_files = [f for f in previous_files if f not in current_files]
        if deleted_files:
            print(f"Files deleted: {deleted_files}")

        # Check for modified files
        modified_files = [f for f in current_files if f in previous_files and current_files[f] != previous_files[f]]
        if modified_files:
            print(f"Files modified: {modified_files}")

        # Update previous_files for the next iteration
        previous_files = current_files


# Specify the directory to monitor
directory_to_monitor = 'mock_data'

# Start monitoring
monitor_directory(directory_to_monitor, interval=5)  # Check every 5 seconds
