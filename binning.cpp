#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <filesystem>
#include <future>
#include <thread>

using namespace std;

// Function to load CSV data into an unordered map
std::unordered_map<std::string, std::string> load_csv(const std::string& design_file) {
    std::unordered_map<std::string, std::string> data;
    std::ifstream file(design_file);
    std::string line;

    if (file.is_open()) {
        std::getline(file, line); // Skip the header line

        while (std::getline(file, line)) {
            std::istringstream lineStream(line);
            std::string barcode, sequence;

            // Assuming "barcode" is the first column and "sequence" is the second column
            if (std::getline(lineStream, barcode, ',') && std::getline(lineStream, sequence, ',')) {
                data[barcode] = sequence;
            }
        }

        file.close();
    } else {
        std::cerr << "Unable to open file " << design_file << std::endl;
    }

    return data;
}

// Function to load reads from a text file and bin them by the first 12 characters
std::unordered_map<std::string, std::vector<std::string>> load_reads(const std::string& reads_file) {
    std::unordered_map<std::string, std::vector<std::string>> bins;
    std::ifstream file(reads_file);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            line = line.substr(0, line.find_last_not_of(" \n\r\t") + 1); // Strip trailing whitespace

            if (!line.empty()) {
                std::string index = line.substr(0, 12); // Get the first 12 symbols
                bins[index].push_back(line); // Add the line to the appropriate bin
            }
        }

        file.close();
    } else {
        std::cerr << "Unable to open file " << reads_file << std::endl;
    }

    return bins;
}

// Custom shuffle function
void custom_shuffle(std::vector<std::string>& reads) {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (size_t i = reads.size() - 1; i > 0; --i) {
        std::uniform_int_distribution<size_t> dist(0, i);
        std::swap(reads[i], reads[dist(gen)]);
    }
}

// Function to limit the size of the reads vector
void shuffle_and_resize(std::vector<std::string>& reads, size_t max_size) {
    custom_shuffle(reads); // Use the custom shuffle
    if (reads.size() > max_size) {
        reads.resize(max_size);
    }
}

// Function to process each bin and write to a .json file
void process_bin(const std::string& index, const std::string& sequence, std::vector<std::string> reads) {
    // Limit reads to 16 random samples if there are more than 16
    shuffle_and_resize(reads, 16);

    // Define the output filename with .json suffix
    std::string json_filename = "C:\\Users\\booki\\Desktop\\FunMoney\\DNA Storage\\concise_cpp\\clusters\\" + index + ".json";

    // Write bin data to the .json file
    std::ofstream out_file(json_filename);
    if (out_file.is_open()) {
        out_file << "{\n"; // Start of JSON object
        out_file << "  \"index\": \"" << index << "\",\n";
        out_file << "  \"data\": \"" << sequence << "\",\n";
        out_file << "  \"noisy_copies\": [\n";

        for (size_t i = 0; i < reads.size(); ++i) {
            out_file << "    \"" << reads[i] << "\"";
            if (i < reads.size() - 1) {
                out_file << ",";
            }
            out_file << "\n";
        }

        out_file << "  ]\n"; // End of noisy_copies array
        out_file << "}\n";   // End of JSON object
        out_file.close();
        std::cout << "Data written to " << json_filename << "\n"; // Confirmation message
    } else {
        std::cerr << "Failed to open file " << json_filename << " for writing.\n";
    }
}

// Function to create JSON files in parallel
void create_json_files_parallel(const std::unordered_map<std::string, std::vector<std::string>>& binned_reads,
                                 const std::unordered_map<std::string, std::string>& barcode_data) {
    std::vector<std::future<void>> futures;

    for (const auto& [index, sequence] : barcode_data) {
        if (binned_reads.find(index) != binned_reads.end()) {
            std::vector<std::string> reads = binned_reads.at(index);
            futures.push_back(std::async(std::launch::async, process_bin, index, sequence, reads));
        }
    }

    std::cout << "Processing bins...\n";
    for (auto& future : futures) {
        future.get(); // Wait for each future to complete
    }
    std::cout << "All bins processed.\n";
}

int main() {
    // Define paths to the CSV and reads files
    std::string design_file_path = "C:\\Users\\booki\\Desktop\\FunMoney\\DNA Storage\\concise_cpp\\deep_design.csv";
    std::string reads_file_path = "C:\\Users\\booki\\Desktop\\FunMoney\\DNA Storage\\concise_cpp\\reads_trimmed.txt";

    // Load data from CSV file into a map structure
    std::unordered_map<std::string, std::string> index_design = load_csv(design_file_path);
    if (index_design.empty()) {
        std::cerr << "Failed to load index design data from " << design_file_path << std::endl;
        return 1; // Exit with error code
    }
    std::cout << "Index design data loaded successfully.\n";

    // Load reads from the text file and bin them using the first 12 characters as the key
    std::unordered_map<std::string, std::vector<std::string>> binned_reads = load_reads(reads_file_path);
    if (binned_reads.empty()) {
        std::cerr << "Failed to load reads data from " << reads_file_path << std::endl;
        return 1; // Exit with error code
    }
    std::cout << "Reads data loaded and binned successfully.\n";

    // Create JSON files in parallel
    create_json_files_parallel(binned_reads, index_design);
    return 0;
}