#include "doctest.h"
#include "lib/include/parser.h"
#include  <iostream>
#include "lib/src/parser.cpp"

TEST_CASE("Input files") {
    std::vector<std::string> expected_input_names, expected_names = {"Input"};
    ffmpeg_parse::parse_result result;
    std::string command;
    SUBCASE("Solo file") {
        expected_input_names = {"C.mkv"};
        command = "ffmpeg -i C.mkv out1.mkv -c:s dvdsub -an out2.mkv";
    }
    SUBCASE("Multiple files") {
        expected_input_names = {"A.avi", "C.mkv", "B.mp4"};
        command = "ffmpeg -i A.avi -i C.mkv -i B.mp4 -filter_complex \"overlay\" out1.mp4 out2.srt";
    }
    ffmpeg_parse::parse_to_graph(command, result);
    CHECK(result.graph.names == expected_names);
    CHECK(result.input_names == expected_input_names);
    CHECK(!result.graph.names.empty());
}

TEST_CASE("Name parse") {
    ffmpeg_parse::parse_result result;
    std::string command;
    std::size_t expected_pos;
    std::vector<std::string> expected_names;
    SUBCASE("Solo file") {
        command = "[A.jpg:0:a]sldf";
        expected_names = {"Input", "A.jpg:0:a"};
        expected_pos = 11;
    }
    SUBCASE("Multiple files") {
        command = "[0:as:vgm][asd][file]tr.rash";
        expected_names = {"Input", "0:as:vgm", "asd", "file"};
        expected_pos = 21;
    }
    SUBCASE("Spaces") {
        command = " [0 :as:vgm] [asd] [file] tr.rash";
        expected_names = {"Input", "0 :as:vgm", "asd", "file"};
        expected_pos = 26;
    }
    SUBCASE("Empty") {
        command = " []";
        expected_names = {"Input", ""};
        expected_pos = 3;
    }
    ffmpeg_parse::parse_context cur_context{command, 0};
    std::vector<std::size_t> ids;
    ffmpeg_parse::parse_names(cur_context, result, ids);
    CHECK(result.graph.names == expected_names);
    CHECK(cur_context.pos == expected_pos);
}


TEST_CASE("Edges parse") {
    // TODO
}