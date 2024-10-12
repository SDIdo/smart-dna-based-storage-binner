#ifndef BINNING_H
#define BINNING_H

#include <string>
#include <vector>
#include <unordered_map>


// Declare a function to process bins
void process_bin(int index, const std::string& sequence, const std::vector<std::string>& reads);

// Declare a function to create JSON files in parallel
void create_json_files_parallel(const std::unordered_map<int, std::vector<std::string>>& binned_reads, 
                                const std::unordered_map<int, std::string>& barcode_data);

std::map<std::string, std::string> load_csv(const std::string& filename);

void printCredentials(const std::map<std::string, std::string>& data);

void binner(std::string design_file_path, std::string reads_file_path);

#endif // BINNING_H