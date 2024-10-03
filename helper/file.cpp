#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

std::ifstream open_file(const std::string file_path) {
    std::filesystem::path path(file_path);
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error(std::string("File does not exist: ") + file_path);
    }

    std::ifstream infile(path, std::ios::binary);
    if (!infile) {
        throw std::runtime_error(std::string("Failed to open file ") + path.string());
    }
    return infile;
}

void open_output_file(const std::string output_filename, std::ofstream &outfile) {
    outfile.open(output_filename, std::ios::binary);
    if (!outfile) {
        throw std::system_error(errno, std::generic_category(), "Failed to open output file: " + output_filename);
    }
}

std::string read_filepath(int argc, char* argv[]) {
    if (argc != 2) {
        throw std::invalid_argument("Usage: ./client <file_path>");
    }

    return argv[1];
}

std::string get_filename(const std::string file_path) {
    return std::filesystem::path(file_path).filename().string();
}

std::string create_output_filename(const std::string filename) {
    std::filesystem::path original_path(filename);
    std::filesystem::path output_path = original_path;

    output_path.replace_filename(
        original_path.stem().string() + "_out" + original_path.extension().string()
    );

    return output_path.string();
}

