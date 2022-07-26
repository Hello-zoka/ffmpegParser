#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "lib/include/parser.h"

void upload_options() {
    std::filesystem::path bad_options_file(
        "lib/src/bad_parameter_config");  // uploading bad options from file
    if (!std::filesystem::exists(bad_options_file)) {
        std::cerr << "No options file\n";
        return;
    }
    std::ifstream options_stream(bad_options_file,
                                 std::ios_base::in | std::ios_base::binary);
    if (options_stream.fail()) {
        std::cerr << "Unable to open options file\n";
        return;
    }
    std::string cur_line;
    int cnt = 0;
    ffmpeg_parse::dot_parsing = false;
    while (std::getline(options_stream, cur_line)) {
        std::stringstream ss(cur_line);  // to allow some comments after option
        std::string option;
        ss >> option;
        if (option == "dot_parse")
            ffmpeg_parse::dot_parsing = true;
        else
            ffmpeg_parse::bad_options.insert(option);
        cnt++;
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
    ffmpeg_parse::refactor_graph(result);

    std::cout << ffmpeg_parse::convert_graph(result);
    return 0;
}
