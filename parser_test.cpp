#include "lib/include/parser.h"
#include <iostream>
#include "doctest.h"
#include "lib/src/graph_convert.cpp"
#include "lib/src/graph_refactor.cpp"
#include "lib/src/parser.cpp"

using namespace ffmpeg_parse;

TEST_CASE("Input files") {
    std::vector<std::string> expected_names;
    graph result;
    std::string command;
    SUBCASE("Solo file") {
        expected_names = {"C.mkv"};
        command = "ffmpeg -i C.mkv outmkv -c:s dvdsub -an out2,mkv";
    }
    SUBCASE("Multiple files") {
        expected_names = {"A.avi", "C.mkv", "B.mp4"};
        command = "ffmpeg -i A.avi -i C.mkv -i    B.mp4 ";
    }
    parse_to_graph(command, result);

    CHECK(result.names == expected_names);
    CHECK(!result.names.empty());
}

TEST_CASE("Name parse") {
    graph result;
    std::string command;
    std::size_t expected_pos;
    std::vector<std::string> expected_names;
    SUBCASE("Solo file") {
        command = "[A.jpg:0:a]sldf";
        expected_names = {"A.jpg:0:a"};
        expected_pos = 11;
    }
    SUBCASE("Multiple files") {
        command = "[a:as:vgm][asd][file]tr.rash";
        expected_names = {"a:as:vgm", "asd", "file"};
        expected_pos = 21;
    }
    SUBCASE("Spaces") {
        command = " [a :as:vgm] [asd] [file] tr.rash";
        expected_names = {"a :as:vgm", "asd", "file"};
        expected_pos = 26;
    }
    parse_context cur_context{command, 0};
    std::vector<std::size_t> ids;
    parse_names(cur_context, result, ids);
    CHECK(result.names == expected_names);
    CHECK(cur_context.pos == expected_pos);
}

bool edge::operator==(const edge &rhs) const {
    return from == rhs.from && to == rhs.to && label == rhs.label;
}

bool edge::operator!=(const edge &rhs) const {
    return !(rhs == *this);
}

TEST_CASE("Edges parse") {
    std::string command;
    std::vector<std::string> expected_names;
    std::vector<VertexType> expected_vertex_type;
    std::vector<edge> expected_edges;
    SUBCASE("Simple") {
        command =
            "ffmpeg\n"
            "-copytb 1 -copyts -vsync 0 -i "
            "file:///slot/sandbox/nv_tmpfs/j/downloaded_video/"
            "input_video_0.mov\n"
            "-itsoffset 82.0 -i file:///slot/sandbox/nv_tmpfs/j/logo\n"
            "-filter_complex\n"
            "\"[0][1]overlay=eof_action=repeat:x=0:y=0[s0];\n"
            "[s0]select=between(pts\\,983040\\,1080832)[s1]\"";
        expected_names = {
            "file:///slot/sandbox/nv_tmpfs/j/downloaded_video/"
            "input_video_0.mov",
            "file:///slot/sandbox/nv_tmpfs/j/logo",
            "s0",
            "overlay=eof_action=repeat:x=0:y=0 (3)",
            "s1",
            "select=between(pts\\,983040\\,1080832) (5)"};
        expected_edges = {
            {0, 3, ""}, {1, 3, ""}, {3, 2, ""}, {2, 5, ""}, {5, 4, ""}};
        expected_vertex_type = {input, input, middle, filter, middle, filter};
    }
    SUBCASE("Complicated") {
        command =
            "ffmpeg -i A.avi -i B.mp4 -i C.mkv -filter_complex "
            "\"[1:v]hue=s=0,split=2[outv1][outv2];overlay;aresample\"";
        expected_names = {"A.avi",       "B.mp4",        "C.mkv",
                          "outv1",       "outv2",        "hue=s=0,split=2 (5)",
                          "overlay (6)", "aresample (7)"};
        expected_edges = {
            {1, 5, ""}, {5, 3, ""}, {5, 4, ""}, {0, 6, ""}, {0, 7, ""}};
        expected_vertex_type = {input,  input,  input,  middle,
                                middle, filter, filter, filter};
    }
    SUBCASE("Spaces") {
        command =
            "ffmpeg -i A.avi -i B.mp4 -i C.mkv -filter_complex "
            "[1:v]hue=s=0,split=2[outv1][outv2];overlay;aresample";
        expected_names = {"A.avi",       "B.mp4",        "C.mkv",
                          "outv1",       "outv2",        "hue=s=0,split=2 (5)",
                          "overlay (6)", "aresample (7)"};
        expected_edges = {
            {1, 5, ""}, {5, 3, ""}, {5, 4, ""}, {0, 6, ""}, {0, 7, ""}};
        expected_vertex_type = {input,  input,  input,  middle,
                                middle, filter, filter, filter};
    }

    graph result;
    parse_to_graph(command, result);
    CHECK(result.names == expected_names);
    CHECK(result.edges == expected_edges);
    CHECK(result.vertex_type == expected_vertex_type);
}

TEST_CASE("Filter chain") {
    std::string command;
    std::vector<std::string> expected_names;
    std::vector<VertexType> expected_vertex_type;
    std::vector<edge> expected_edges;
    SUBCASE("Simple") {
        command = "ffmpeg -i input -vf scale=iw/2:-1 output.mp4";
        expected_names = {"input", "scale=iw/2:-1 (1)", "output.mp4"};
        expected_edges = {{0, 1, ""}};
        expected_vertex_type = {input, filter, unlinked_output};
    }
    SUBCASE("Intermediate files") {
        command =
            "ffmpeg -i input -vf "
            "[in]yadif=0:0:0[middle];[middle]scale=iw/2:-1[out] output.mp4";
        expected_names = {"input",           "in",  "middle",
                          "yadif=0:0:0 (3)", "out", "scale=iw/2:-1 (5)",
                          "output.mp4"};
        expected_edges = {{1, 3, ""}, {3, 2, ""}, {2, 5, ""}, {5, 4, ""}};
        expected_vertex_type = {input,  middle, middle,         filter,
                                middle, filter, unlinked_output};
    }

    graph result;
    parse_to_graph(command, result);
    CHECK(result.names == expected_names);
    CHECK(result.edges == expected_edges);
    CHECK(result.vertex_type == expected_vertex_type);
}

TEST_CASE("Mapping") {
    std::string command;
    std::vector<std::string> expected_names;
    std::vector<VertexType> expected_vertex_type;
    std::vector<edge> expected_edges;
    std::size_t expected_input_amount, expected_output_amount;

    SUBCASE("Simple mapping") {
        command =
            "ffmpeg -i A.avi -i B.mp4 -i C.mkv -filter_complex "
            "\"[1:v]hue=s=0,split=2[outv1][outv2]\" -map [outv1] -an out1.mp4 "
            "-map [outv2] out2.mp4";
        expected_names = {"A.avi",    "B.mp4",   "C.mkv",
                          "outv1",    "outv2",   "hue=s=0,split=2 (5)",
                          "out1.mp4", "out2.mp4"};
        expected_edges = {
            {1, 5, ""}, {5, 3, ""}, {5, 4, ""}, {3, 6, "-map"}, {4, 7, "-map"}};
        expected_vertex_type = {input,  input,  input,         middle,
                                middle, filter, mapped_output, mapped_output};
        expected_input_amount = 3;
        expected_output_amount = 2;
    }

    SUBCASE("Unexpected end of command") {  // Check std::cerr "unexpected end
                                            // of command"
        command =
            "ffmpeg -i A.avi -i B.mp4 -i C.mkv -filter_complex "
            "\"[1:v]hue=s=0,split=2[outv1][outv2];overlay;aresample\"\n"
            "        -map [outv1] -an        out1.mp4\n"
            "        -map [outv2] -map [1:a:0]";
        expected_names = {"A.avi",       "B.mp4",         "C.mkv",
                          "outv1",       "outv2",         "hue=s=0,split=2 (5)",
                          "overlay (6)", "aresample (7)", "out1.mp4"};
        expected_edges = {{1, 5, ""}, {5, 3, ""}, {5, 4, ""},
                          {0, 6, ""}, {0, 7, ""}, {3, 8, "-map"}};
        expected_vertex_type = {input,  input,  input,  middle,       middle,
                                filter, filter, filter, mapped_output};
        expected_input_amount = 3;
        expected_output_amount = 1;
    }

    SUBCASE("Hard case") {  // Check std::cerr "Hard case"
        SUBCASE("Quotes") {
            command =
                "ffmpeg -i A.avi -i B.mp4 -i C.mkv -filter_complex "
                "\"[1:v]hue=s=0,split=2[outv1]  [outv2] ;overlay;aresample\"\n"
                "        -map [outv1] -an        out1.mp4\n"
                "                                  out2.mkv\n"
                "        -map [outv2] -map [1:a:0] out3.mkv";
        }
        SUBCASE("Spaces") {
            command =
                "ffmpeg -i A.avi -i B.mp4 -i C.mkv -filter_complex "
                "[1:v]hue=s=0,split=2[outv1][outv2];overlay;aresample\n"  // \n
                                                                          // is_space!!
                "        -map [outv1] -an        out1.mp4\n"
                "                                  out2.mkv\n"
                "        -map [outv2] -map [1:a:0] out3.mkv";
        }
        expected_names = {"A.avi",       "B.mp4",         "C.mkv",
                          "outv1",       "outv2",         "hue=s=0,split=2 (5)",
                          "overlay (6)", "aresample (7)", "out1.mp4",
                          "out2.mkv",    "out3.mkv"};
        expected_edges = {{1, 5, ""},      {5, 3, ""},     {5, 4, ""},
                          {0, 6, ""},      {0, 7, ""},     {3, 8, "-map"},
                          {4, 10, "-map"}, {1, 10, "-map"}};
        expected_vertex_type = {input,           input,        input,
                                middle,          middle,       filter,
                                filter,          filter,       mapped_output,
                                unlinked_output, mapped_output};
        expected_input_amount = 3;
        expected_output_amount = 2;
    }

    graph result;
    try {
        parse_to_graph(command, result);
    } catch (...) {
    }
    CHECK(result.names == expected_names);
    CHECK(result.edges == expected_edges);
    CHECK(result.vertex_type == expected_vertex_type);
    CHECK(expected_input_amount == result.input_amount);
    CHECK(expected_output_amount == result.output_amount);
}

TEST_CASE("Graph viz") {
    std::string command;
    SUBCASE("Hard case") {
        command =
            "ffmpeg -i A.avi -i B.mp4 -i C.mkv -filter_complex "
            "\"[1:v]hue=s=0,split=2[outv1][outv2];overlay;aresample\"\n"
            "        -map [outv1] -an        out1.mp4\n"
            "                                  out2.mkv\n"
            "        -map [outv2] -map [1:a:0] out3.mkv";
    }
    SUBCASE("Simple") {
        command =
            "ffmpeg -i A.avi -i B.mp4 out1.mkv out2.wav -map [1:a] -c:a copy "
            "out3.mov";
    }
    SUBCASE("Filter chain") {
        command =
            "ffmpeg -i input -vf "
            "yadif=0:0:0[middle];[middle]scale=iw/2:-1[out] output.mp4";
    }
    graph result;

    parse_to_graph(command, result);
    try {
        refactor_graph(result);
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    std::cerr << convert_graph(result);
}