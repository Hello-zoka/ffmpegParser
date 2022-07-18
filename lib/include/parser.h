#ifndef FFMPEGPARSER_PARSER_H
#define FFMPEGPARSER_PARSER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>


namespace ffmpeg_parse {

    struct parse_context {
        const std::string command;
        std::size_t pos;


        bool empty() const {
            return pos >= command.size();
        }

        char get_char() {
            assert(!empty());
            return command[pos++];
        }

        char check_char() const {
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

    struct graph {
        std::unordered_map<std::string, std::size_t> index_by_name;
        std::vector<std::string> names;
        std::vector<int> vertex_type;
        std::vector<edge> edges;
        std::size_t input_amount = 0;
        std::size_t output_amount = 0;
    };


    int parse_to_graph(const std::string &command, graph &result);

    void check_graph(const graph &graph);

    std::string convert_graph(const graph &graph);



    // exceptions

    struct unexpected_option_prefix : std::runtime_error {
        explicit unexpected_option_prefix(std::size_t error_pos);
    };

    struct expected_filename : std::runtime_error {
        explicit expected_filename(std::size_t error_pos);
    };

    struct expected_quote : std::runtime_error {
        explicit expected_quote(std::size_t error_pos);
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
        explicit incorrect_vertex_type(std::string vertex_name, std::size_t vertex_type);
    };


    struct incorrect_graph : std::runtime_error {
        explicit incorrect_graph(std::string error_message);
    };
}

#endif //FFMPEGPARSER_PARSER_H
