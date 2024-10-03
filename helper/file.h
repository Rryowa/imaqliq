#pragma once
#include "const.h"
#include <fstream>
#include <string>

std::ifstream open_file(const std::string file_path);

void open_output_file(const std::string output_filename, std::ofstream &outfile);

std::string read_filepath(int argc, char* argv[]);

std::string get_filename(const std::string file_path);

std::string create_output_filename(const std::string filename);
