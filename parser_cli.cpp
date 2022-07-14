#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "lib/include/parser.h"

int main(int argc, char *argv[]) {
//    if (argc != 2) {
//        std::cerr << "Using " << argv[0] << " <file name>\n";
//        return 1;
//    }
    std::filesystem::path filename("/Users/yuazajcev/Documents/ffmpegParser/test0.txt");
    if (!std::filesystem::exists(filename)) {
        std::cerr << "File doesn't exists\n";
        return 1;
    }
    std::ifstream is(filename, std::ios_base::in | std::ios_base::binary);
    if (is.fail()) {
        std::cerr << "Unable to open file\n";
        return 1;
    }

    std::string query, cur_line;
    while (std::getline(is, cur_line)) {
        query += cur_line + ' ';
    }
    std::cout << query;


//    std::filesystem::path global_options("/Users/yuazajcev/Documents/ffmpegParser/lib/src/global_options"); // uploading global options from file
//    if (!std::filesystem::exists(global_options)) {
//        std::cerr << "No global options file\n";
//        return 1;
//    }
//    std::ifstream options_stream(global_options, std::ios_base::in | std::ios_base::binary);
//    if (options_stream.fail()) {
//        std::cerr << "Unable to open global options file\n";
//        return 1;
//    }
//    while (std::getline(options_stream, cur_line)) {
//        std::stringstream ss(cur_line); // to allow some text after option
//        std::string option;
//        ss >> option;
//        std::cerr << "\"" <<  option << "\",\n";
//    }


    return 0;
}