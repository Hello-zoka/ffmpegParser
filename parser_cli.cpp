#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "lib/include/parser.h"

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

    ffmpeg_parse::parse_to_graph(query, result);
    try {
        ffmpeg_parse::refactor_graph(result);
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    std::cout << ffmpeg_parse::convert_graph(result);

    return 0;
}