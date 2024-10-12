#ifndef MULTI_THREADED_PREPROCESSOR_H
#define MULTI_THREADED_PREPROCESSOR_H

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
#include <iterator>
#include <iomanip>
#include <cmath>

std::string copy_reverse_complement(const std::string& cp);

void read_preprocessor(const std::string& file_input_path, const std::string& preprocessed_reads_path, int lib_length);

std::string process_line(const std::string& line, const std::string& front_primer, const std::string& back_primer, int lib_length);

#endif // MULTI_THREADED_PREPROCESSOR_H