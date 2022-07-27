#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "lib/include/parser.h"

void upload_options(const std::string &config_filename) {
    std::filesystem::path bad_options_file(
        config_filename);  // uploading bad options from file
    if (!std::filesystem::exists(bad_options_file)) {
        std::cerr << "No config file\n";
        return;
    }
    std::ifstream options_stream(bad_options_file,
                                 std::ios_base::in | std::ios_base::binary);
    if (options_stream.fail()) {
        std::cerr << "Unable to open config file\n";
        return;
    }
    std::string cur_line;
    ffmpeg_parse::dot_parsing = false;
    while (std::getline(options_stream, cur_line)) {
        std::stringstream ss(cur_line);  // to allow some comments after option
        std::string option;
        ss >> option;
        if (option == "mode:") {
            ss >> option;
            if (option == "dot_parse")
                ffmpeg_parse::dot_parsing = true;
        } else if (option == "bad_options:") {
            while (ss >> option) {
                ffmpeg_parse::bad_options.insert(option);
            }
        } else if (option == "log_options:") {
            while (ss >> option) {
                ffmpeg_parse::log_options.insert(option);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Using " << argv[0] << " <file name> <config file name>\n";
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

    upload_options(argv[2]);

    ffmpeg_parse::parse_to_graph(query, result);
    ffmpeg_parse::refactor_graph(result);

    std::cout << ffmpeg_parse::convert_graph(result);
    return 0;
}
