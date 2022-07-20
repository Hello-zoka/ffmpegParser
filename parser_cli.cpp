#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "lib/include/parser.h"

void upload_options() {
    std::filesystem::path bad_options_file("lib/src/bad_parameter_options"); // uploading global options from file
    if (!std::filesystem::exists(bad_options_file)) {
        std::cerr << "No options file\n";
    }
    std::ifstream options_stream(bad_options_file, std::ios_base::in | std::ios_base::binary);
    if (options_stream.fail()) {
        std::cerr << "Unable to open options file\n";
    }
    std::string cur_line;
    while (std::getline(options_stream, cur_line)) {
        std::stringstream ss(cur_line); // to allow some comments after option
        std::string option;
        ss >> option;
        ffmpeg_parse::bad_options.insert(option);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Using " << argv[0] << " <file name>\n";
        return 1;
    }
    std::filesystem::path filename(argv[1]);
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
    ffmpeg_parse::graph result;

    upload_options();

    ffmpeg_parse::parse_to_graph(query, result);
    try {
        ffmpeg_parse::refactor_graph(result);
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
    }

    std::freopen("out.txt", "w", stdout);
    std::cout << ffmpeg_parse::convert_graph(result);
    fclose(stdout);
    return 0;
}