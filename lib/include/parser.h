#ifndef FFMPEGPARSER_PARSER_H
#define FFMPEGPARSER_PARSER_H

#include <cassert>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ffmpeg_parse {

inline std::unordered_set<std::string> bad_options, log_options;
inline bool dot_parsing;
inline constexpr char file_option[] = "-f";
inline constexpr char map_option[] = "-map";
inline constexpr char input_option[] = "-i";
inline const std::set<std::string> filter_tags = {"-filter_complex", "-vf",
                                                  "-af", "-sf", "-df"};

struct parse_context {
    const std::string command;
    std::size_t pos;

    [[nodiscard]] bool empty() const {
        return pos >= command.size();
    }

    char get_char() {
        assert(!empty());
        return command[pos++];
    }

    [[nodiscard]] char check_char() const {
        assert(!empty());
        return command[pos];
    }
};

struct edge {
    bool operator==(const edge &rhs) const;

    bool operator!=(const edge &rhs) const;

    std::size_t from, to;
    std::string label;
};

enum VertexType { input, middle, filter, mapped_output, unlinked_output };

struct graph {
    std::unordered_map<std::string, std::size_t> index_by_name;
    std::vector<std::string> names;
    std::vector<VertexType> vertex_type;
    std::vector<bool> vertex_correctness;
    std::vector<std::size_t> gl_out_pos;
    std::vector<edge> edges;
    std::size_t input_amount = 0;
    std::size_t output_amount = 0;
    enum Type { chain, complex } type;
};

void parse_to_graph(const std::string &command, graph &result);

void refactor_graph(graph &graph);

std::string convert_graph(graph &graph);

// exceptions

struct expected_filename : std::runtime_error {
    explicit expected_filename(std::size_t error_pos);
};

struct no_closed_paren : std::runtime_error {
    explicit no_closed_paren(std::size_t error_pos);
};

struct unexpected_end_of_command : std::runtime_error {
    explicit unexpected_end_of_command();
};

struct unexpected_char_after_filter : std::runtime_error {
    explicit unexpected_char_after_filter(std::size_t error_pos);
};

struct empty_name : std::runtime_error {
    explicit empty_name(std::size_t error_pos);
};

struct incorrect_reference : std::runtime_error {
    explicit incorrect_reference(std::size_t error_pos);
};

struct incorrect_output_name : std::runtime_error {
    explicit incorrect_output_name(std::size_t error_pos);
};

struct expected_stream_name : std::runtime_error {
    explicit expected_stream_name(std::size_t error_pos);
};

struct incorrect_vertex_type : std::runtime_error {
    explicit incorrect_vertex_type(const std::string &vertex_name,
                                   std::size_t vertex_type);
};
}  // namespace ffmpeg_parse

#endif  // FFMPEGPARSER_PARSER_H
