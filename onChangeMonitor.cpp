#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <map>
#include <vector>
#include <ctime>
#include <string>
#include "binning.h"
#include "multi_threaded_preprocessor.h"

// Function to scan a directory and return a map of file names and their modification times
std::map<std::string, std::filesystem::file_time_type> scan_directory(const std::string& directory) {
    std::map<std::string, std::filesystem::file_time_type> files;

    // Iterate over the contents of the directory
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (std::filesystem::is_regular_file(entry.status())) {
            // Store the file name and its last modification time in the map
            files[entry.path().filename().string()] = std::filesystem::last_write_time(entry);
        }
    }
    
    return files;
}

// this parameter presents the lib length of 140
int lib_length = 140;

// path to the sequencing output - can be .txt or a .fastq file or a path to a folder with many .fastq files
// this is the file the includes all the basecalled reads that we want to processed
std::string reads_path = "fastq_runid.fastq";

std::string design_file_path = "./deep_design.csv";

void monitor_directory(const std::string& directory, int interval = 5) {
    std::cout << "Monitoring directory: " << directory << std::endl;

    auto previous_files = scan_directory(directory);  // Initial snapshot of the directory

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));  // Wait for the specified interval

        auto current_files = scan_directory(directory);  // Scan the directory again

        // Check for new files
        std::vector<std::string> new_files;
        for (const auto& [file, _] : current_files) {
            if (previous_files.find(file) == previous_files.end()) {
                new_files.push_back(file);
            }
        }
        if (!new_files.empty()) {
            for (const auto& file : new_files) {
                std::string processed_file = "mock_data/" + file;
                std::cout << "New files added: " << processed_file << std::endl;
                std::string reads_trimmed_path = "processed_data/reads_trimmed_" + 
                    std::to_string(std::time(nullptr)) + ".txt";  // Format date as you need
                int lib_length = 100;  // Assume lib_length is available
                std::string design_file_path = "path/to/design_file";  // Assume design_file_path is available

                read_preprocessor(processed_file, reads_trimmed_path, lib_length);
                binner(design_file_path, reads_trimmed_path);
            }
        }

        // Check for deleted files
        std::vector<std::string> deleted_files;
        for (const auto& [file, _] : previous_files) {
            if (current_files.find(file) == current_files.end()) {
                deleted_files.push_back(file);
            }
        }
        if (!deleted_files.empty()) {
            std::cout << "Files deleted: ";
            for (const auto& file : deleted_files) {
                std::cout << file << " ";
            }
            std::cout << std::endl;
        }

        // Check for modified files
        std::vector<std::string> modified_files;
        for (const auto& [file, timestamp] : current_files) {
            if (previous_files.find(file) != previous_files.end() && previous_files[file] != timestamp) {
                modified_files.push_back(file);
            }
        }
        if (!modified_files.empty()) {
            std::cout << "Files modified: ";
            for (const auto& file : modified_files) {
                std::cout << file << " ";
            }
            std::cout << std::endl;
        }

        // Update previous_files for the next iteration
        previous_files = current_files;
    }
}

void main() {
    std::string directory_to_monitor = "mock_data";
    monitor_directory(directory_to_monitor, 5);
}
