#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <future>
#include <filesystem>
#include <fstream>
#include <thread>
#include <iomanip>
#include <cmath>

namespace fs = std::filesystem;

std::string copy_reverse_complement(const std::string& cp) {
    std::string trimmed_cp = cp;
    trimmed_cp.erase(trimmed_cp.find_last_not_of(" \n\r\t") + 1); // Trim

    std::unordered_map<char, char> complement = {
        {'A', 'T'}, {'C', 'G'}, {'G', 'C'}, {'T', 'A'}
    };

    std::string bases;
    for (char base : trimmed_cp) {
        auto it = complement.find(base);
        if (it != complement.end()) {
            bases += it->second; // Append complement
        }
    }

    std::reverse(bases.begin(), bases.end());
    return bases;
}

std::string process_line(const std::string& line, const std::string& front_primer, const std::string& back_primer, int lib_length) {
    // std::string front_primer_rev_com = copy_reverse_complement(front_primer);
    // std::string back_primer_rev_com = copy_reverse_complement(back_primer);

    size_t front_ind = line.find(front_primer);
    size_t back_ind = line.find(back_primer);

    if (front_ind != std::string::npos && back_ind != std::string::npos) {
        std::string seq_without_primers = line.substr(front_ind + front_primer.length(), back_ind - (front_ind + front_primer.length()));
        if (std::abs(static_cast<int>(seq_without_primers.length()) - lib_length) <= 5) {
            return seq_without_primers;
        }
    } else if (front_ind != std::string::npos) {
        std::string seq_without_primers = line.substr(front_ind + front_primer.length(), std::min(static_cast<int>(lib_length), static_cast<int>(line.length()) - (static_cast<int>(front_ind) + static_cast<int>(front_primer.length()))));
        if (std::abs(static_cast<int>(seq_without_primers.length()) - lib_length) <= 5) {
            return seq_without_primers;
        }
    } else if (back_ind != std::string::npos) {
        std::string seq_without_primers = line.substr(std::max(0, static_cast<int>(back_ind) - lib_length), back_ind);
        if (std::abs(static_cast<int>(seq_without_primers.length()) - lib_length) <= 5) {
            return seq_without_primers;
        }
    } else {
        std::string reversed_line = copy_reverse_complement(line);
        front_ind = reversed_line.find(front_primer);
        back_ind = reversed_line.find(back_primer);

        if (front_ind != std::string::npos && back_ind != std::string::npos) {
            std::string seq_without_primers = reversed_line.substr(front_ind + front_primer.length(), back_ind - (front_ind + front_primer.length()));
            if (std::abs(static_cast<int>(seq_without_primers.length()) - lib_length) <= 5) {
                return seq_without_primers;
            }
        } else if (front_ind != std::string::npos) {
            std::string seq_without_primers = reversed_line.substr(front_ind + front_primer.length(), std::min(static_cast<int>(lib_length), static_cast<int>(reversed_line.length()) - (static_cast<int>(front_ind) + static_cast<int>(front_primer.length()))));
            if (std::abs(static_cast<int>(seq_without_primers.length()) - lib_length) <= 5) {
                return seq_without_primers;
            }
        } else if (back_ind != std::string::npos) {
            std::string seq_without_primers = reversed_line.substr(std::max(0, static_cast<int>(back_ind) - lib_length), back_ind);
            if (std::abs(static_cast<int>(seq_without_primers.length()) - lib_length) <= 5) {
                return seq_without_primers;
            }
        }
    }

    return ""; // Return empty string if no valid sequence found
}

void read_preprocessor(const std::string& file_input_path = "./Calls.fastq", const std::string& preprocessed_reads_path = "./reads_processed.fastq", int lib_length = 140) {
    std::ofstream file_output_without_primers(preprocessed_reads_path);
    if (!file_output_without_primers.is_open()) {
        std::cerr << "Failed to open output file: " << preprocessed_reads_path << std::endl;
        return;
    }

    std::string front_primer = "TAAGAGACAG";
    std::string back_primer = "CTGTCTCTTA";

    if (fs::is_regular_file(file_input_path)) {
        // std::cout << "The path '" << file_input_path << "' is a file." << std::endl;
        std::ifstream file_input_fastq(file_input_path);

        if (!file_input_fastq.is_open()) {
            std::cerr << "Failed to open input file: " << file_input_path << std::endl;
            return;
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file_input_fastq, line)) {
            if (!line.empty()) lines.push_back(line);
        }

        // Process lines in parallel
        std::vector<std::future<std::string>> futures;
        for (const auto& seq_line : lines) {
            futures.push_back(std::async(std::launch::async, process_line, seq_line, front_primer, back_primer, lib_length));
        }

        // Write valid sequences to file
        for (auto& future : futures) {
            try {
                auto result = future.get();
                if (!result.empty()) {
                    file_output_without_primers << result << "\n";
                    // std::cout << "Wrote sequence: " << result << "\n"; // Debug print
                // } else {
                //     std::cout << "No valid sequence found\n"; // Debug print
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing line: " << e.what() << std::endl;
            }
        }

        // Ensure that everything is written before closing the file
        file_output_without_primers.flush();
        file_output_without_primers.close();
    } else {
        std::cerr << "The path '" << file_input_path << "' is neither a file nor a folder, or it doesn't exist." << std::endl;
    }
}
