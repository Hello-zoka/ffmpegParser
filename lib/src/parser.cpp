#include  "../include/parser.h"
#include <string>
#include <iostream>

namespace ffmpeg_parse {
#define RETURN_ON_ERROR(x)            \
    exit_code = x;                    \
    if (exit_code != 0) { \
        return exit_code;             \


    void skip_spaces(parse_context &cur_context) {
        while (!cur_context.empty() && isspace(cur_context.command[cur_context.pos])) {
            cur_context.pos++;
        }
    }


    void parse_token(parse_context &cur_context, std::string &result) {
        skip_spaces(cur_context);
        char ch;
        while (!cur_context.empty() && !isspace(ch = cur_context.get_char())) {
            result += ch;
        }
    }

    bool parse_option(parse_context &cur_context, std::size_t pos, const std::string option) {
        skip_spaces(cur_context);
        if (cur_context.command.size() - pos + 1 < option.size()) {
            return false;
        }
        if (cur_context.command.compare(pos, option.size(), option) == 0) {
            std::string null;
            cur_context.pos = pos;
            parse_token(cur_context, null);
            return true;
        }
        return false;
    }

    void parse_input(parse_context &cur_context, parse_result &result) {
        for (std::size_t ind = cur_context.pos; ind < cur_context.command.size(); ind++) {
            if (parse_option(cur_context, ind, "-i")) {
                std::string file_name, null;
                parse_token(cur_context, file_name);
                if (file_name.empty()) {
                    throw expected_filename(cur_context.pos);
                }
                result.input_names.push_back(file_name);
            }
        }
    }

    void parse_names(parse_context &cur_context, parse_result &result, std::vector<std::size_t> &id) {
        skip_spaces(cur_context);
        while (!cur_context.empty() && cur_context.check_char() == '[') {
            std::size_t open_paren_pos = cur_context.pos;
            cur_context.pos++;
            char ch;
            std::string name;
            do {
                if (cur_context.empty()) {
                    throw no_closed_paren(open_paren_pos);
                }
                ch = cur_context.get_char();
                name += ch;
            } while (ch != ']');
            name.pop_back();
            if (result.graph.index_by_name.count(name) == 0) {
                result.graph.index_by_name[name] = result.graph.names.size();
                result.graph.names.push_back(name);
            }
            id.push_back(result.graph.index_by_name[name]);
            skip_spaces(cur_context);
        }
    }

    void parse_filter(parse_context &cur_context, parse_result &result) {
        std::vector<std::size_t> inputs, outputs;
        parse_names(cur_context, result, inputs);
        while (cur_context.check_char() != '[' && cur_context.check_char() != ';' && cur_context.check_char() != '\"') {
            cur_context.get_char();
        }
        parse_names(cur_context, result, outputs);

        for (std::size_t from : inputs) {
            for (std::size_t to : outputs) {
                result.graph.edges.emplace_back(from, to);
            }
        }
    }

    void parse_filter_graph(parse_context &cur_context, parse_result &result) {
        skip_spaces(cur_context);
        char ch = 'a', quote = cur_context.get_char();
        if (quote != '\'' && quote != '\"') {
            throw expected_quote(cur_context.pos);
        }

        while (ch != '\"') {
            parse_filter(cur_context, result);
            if (cur_context.empty()) {
                throw expected_quote(cur_context.pos);
            }
            ch = cur_context.get_char();
            switch (ch) {
                case ';':
                case '\"':
                    break;
                default:
                    throw unexpected_char_after_filter(cur_context.pos);
            }
        }

    }

    void parse_filter_chain(parse_context &cur_context, parse_result &result) {
        // TODO
    }

    int parse_to_graph(std::string &command, parse_result &result) {
        parse_context cur_context{command, 0};
        try {
            parse_input(cur_context, result);
            if (parse_option(cur_context, cur_context.pos, "-filter_complex")) {
                parse_filter_graph(cur_context, result);
            } else {  // TODO wtf is going on in other cases?????
                parse_filter_chain(cur_context, result);
            }
        } catch (std::exception &e) {
            std::cerr << e.what();
            return 1;
        }
        return 0;
    }



    //  exceptions

    expected_filename::expected_filename(std::size_t error_pos)
            : std::runtime_error("Expected file name after '-i' on position " + std::to_string(error_pos) + "\n") {
    }

    unexpected_option_prefix::unexpected_option_prefix(std::size_t error_pos)
            : std::runtime_error("Parsing error at position " + std::to_string(error_pos) +
                                 ". Expected that '-i' prefix reserved for input file options\n") {
    }

    expected_quote::expected_quote(std::size_t error_pos)
            : std::runtime_error("Expected quotation mark at position " + std::to_string(error_pos) + "\n") {
    }

    no_closed_paren::no_closed_paren(std::size_t error_pos)
            : std::runtime_error(
            "Expected closed paren. Last open found on position " + std::to_string(error_pos) + "\n") {
    }

    unexpected_end_of_command::unexpected_end_of_command() : std::runtime_error("Unexpected end of command\n") {
    }

    unexpected_char_after_filter::unexpected_char_after_filter(std::size_t error_pos)
            : std::runtime_error(
            "Unexpected character after filter att position " + std::to_string(error_pos) + "\n") {
    }
}