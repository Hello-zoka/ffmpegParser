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

    struct parse_result {
        struct graph {
            std::unordered_map<std::string, std::size_t> index_by_name = {{"Input", 0}};
            std::vector<std::string> names = {"Input"};
            std::vector<std::pair<int, int>> edges;
        } graph;

        std::vector<std::pair<std::string, std::string>> outputs;
        std::vector<std::string> input_names;
    };

    int parse_to_graph(std::string &command, parse_result &result);


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

    struct unexpected_end_of_command : std::runtime_error{
        explicit unexpected_end_of_command();
    };

    struct unexpected_char_after_filter : std::runtime_error{
        explicit unexpected_char_after_filter(std::size_t error_pos);
    };
}

#endif //FFMPEGPARSER_PARSER_H
