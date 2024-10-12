#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <cstring>
#include <filesystem>
#include <future>
using namespace std;

std::map<std::string, std::string> load_csv(const std::string& filename) {
    std::ifstream file(filename);
    std::map<std::string, std::string> data;
    
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return data;
    }
    
    std::string line;
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string first, second;
        
        // Assume the file has two columns, separated by commas
        if (getline(ss, first, ',') && getline(ss, second, ',')) {
            // Insert into the map with the username as the key and password as the value
            data[first] = second;
        }
    }
    
    file.close();
    return data;
}

void printCredentials(const std::map<std::string, std::string>& data) {
    for (const auto& username : data) {
        std::cout << username.first << ": " << username.second << std::endl;
    }
}

void create_json_files_parallel(const std::unordered_map<int, std::vector<std::string>>& binned_reads, 
                                const std::unordered_map<int, std::string>& barcode_data) {
    std::vector<std::future<void>> futures;

    for (const auto& [index, sequence] : barcode_data) {
        auto it = binned_reads.find(index);
        if (it != binned_reads.end()) {
            const auto& reads = it->second;
            futures.push_back(std::launch::async(std::launch::async, process_bin, index, sequence, reads));
        }
    }

    // Wait for all threads to finish
    for (auto& fut : futures) {
        fut.get();
    }
}

void process_bin(std::string& index, std::string sequence, std::string reads){
    if (strlen(reads) > 16){
        // Number of samples to pick
        int num_samples = 16;

        // Random number generator
        std::random_device rd;
        std::mt19937 g(rd());

        // Shuffle the reads
        std::shuffle(reads.begin(), reads.end(), g);

        // Resize the vector to keep only the first 16 elements
        reads.resize(num_samples);
        }

    struct bin_data {
    string index = index;
    string data = sequence;
    string noisy_copies = reads;
    };

    std::filesystem::create_directories("./clusters");

    // Build the JSON filename
    std::string json_filename = "./clusters/" + index + ".json";

    // Open the file and write the JSON data to it
    std::ofstream json_file(json_filename);
    if (json_file.is_open()) {
        json_file << bin_data.dump(4);  // Writing the JSON data with indentation
        json_file.close();
    } else {
        std::cerr << "Error: Unable to open file " << json_filename << std::endl;
    }
}

void binner(std::string design_file_path = 'deep_design.csv', std::string reads_file_path = 'reads_trimmed.txt'){
    index_design = load_csv(design_file_path)
    binned_reads = load_reads(reads_file_path)
    create_json_files_parallel(binned_reads, index_design)
    }
    
int main() {
    std::map<std::string, std::string> data = load_csv("deep_design.csv");
    printCredentials(data);
    return 0;
}