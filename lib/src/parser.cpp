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
        result = "";
        char ch;
        while (!cur_context.empty() && !isspace(ch = cur_context.get_char())) {
            result += ch;
        }
    }

    void check_token(parse_context &cur_context, std::string &result) {
        std::size_t pos = cur_context.pos;
        result = "";
        while (pos < cur_context.command.size() && isspace(cur_context.command[pos])) {
            pos++;
        }
        while (pos < cur_context.command.size() && !isspace(cur_context.command[pos])) {
            result += cur_context.command[pos++];
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
            return cur_context.command.size() <= pos + option.size() ||
                   isspace(cur_context.command[pos + option.size()]); // check space after option
        }
        return false;
    }

    void add_vertex(parse_result &result, std::string &name, int type) {
        if (result.graph.index_by_name.count(name) == 0) {
            result.graph.index_by_name[name] = result.graph.names.size();
            result.graph.names.push_back(name);
            result.graph.vertex_type.push_back(type);
        }
    }

    void parse_input(parse_context &cur_context, parse_result &result) {
        for (std::size_t ind = cur_context.pos; ind < cur_context.command.size(); ind++) {
            if (parse_option(cur_context, ind, "-i")) {
                std::string file_name, null;
                parse_token(cur_context, file_name);
                if (file_name.empty()) {
                    throw expected_filename(cur_context.pos);
                }
                add_vertex(result, file_name, 0);
                result.input_amount++;
            }
        }
    }

    void parse_reference(parse_context &cur_context, parse_result &result, std::string &reference) {
        int first_colon_pos = reference.size();
        for (std::size_t ind = 0; ind < reference.size(); ind++) {
            if (reference[ind] == ':') {
                first_colon_pos = ind;
                break;
            }
        }
        while (reference.size() > first_colon_pos) {
            reference.pop_back();
        }
        int index;
        try {
            index = std::stoi(reference);
            if (index >= result.input_amount) {
                throw incorrect_reference(cur_context.pos);
            }
            reference = result.graph.names.at(index);
        }
        catch (...) {
            throw incorrect_reference(cur_context.pos);
        }
    }

    void parse_names(parse_context &cur_context, parse_result &result, std::vector<std::size_t> &id) {
        // only add to id's vector
        skip_spaces(cur_context);
        //  TODO parse quotes
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
            if (name.empty()) {
                throw empty_name(cur_context.pos);
            }
            if (isdigit(name[0])) {
                parse_reference(cur_context, result, name);
            }
            add_vertex(result, name, 1);
            id.push_back(result.graph.index_by_name[name]);
            skip_spaces(cur_context);
        }
    }

    void parse_filter(parse_context &cur_context, parse_result &result) {
        std::vector<std::size_t> inputs, outputs;
        std::string command;
        parse_names(cur_context, result, inputs);
        while (cur_context.check_char() != '[' && cur_context.check_char() != ';' && cur_context.check_char() != '\"') {
            command += cur_context.get_char();
        }
        parse_names(cur_context, result, outputs);


        add_vertex(result, command, 2);
        std::size_t command_id = result.graph.index_by_name[command];

        if (inputs.empty()) {
            // TODO refactor
            inputs.push_back(0);
        }
        if (outputs.empty()) {
            // TODO
            // TODO
        }

        for (std::size_t from: inputs) {
            result.graph.edges.push_back({from, command_id, ""});
        }
        for (std::size_t to: outputs) {
            result.graph.edges.push_back({command_id, to, ""});
        }
    }

    void parse_filter_graph(parse_context &cur_context, parse_result &result) {
        skip_spaces(cur_context);
        char ch = 'a', quote = cur_context.get_char();
        if (quote != '\'' && quote != '\"') { // TODO parse without quotes, using  space as separator
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

    bool is_out_name(const std::string &name) {
        int count_dots = 0;
        for (std::size_t ind = 0; ind < name.size(); ind++) {
            if (name[ind] == '.') {
                count_dots++;
            }
        }
        return count_dots == 1;
    }

    bool parse_output_name(parse_context &cur_context, std::string &out_name) {
        std::string token;
        check_token(cur_context, token);
        if (token == "-f") {
            parse_token(cur_context, token); // -f flag
            parse_token(cur_context, token); // format option
            parse_token(cur_context, out_name); // out name
            if (!is_out_name(token)) {
                throw incorrect_output_name(cur_context.pos);
            }
            return true;
        }
        if (is_out_name(token)) {
            parse_token(cur_context, token);
            out_name = token;
            return true;
        }
        return false;
    }

    void parse_mapping(parse_context &cur_context, parse_result &result) {
        std::string token;
        check_token(cur_context, token);
        std::vector<std::size_t> mapped_id;
        std::vector<std::size_t> out_ids;
        while (!cur_context.empty() && token == "-map") {
            parse_token(cur_context, token); // skip -map option
            std::size_t prev_sz = mapped_id.size();
            parse_names(cur_context, result, mapped_id); // only push back new ids
            if (mapped_id.size() == prev_sz) {
                throw expected_stream_name(cur_context.pos);
            }
            std::string out_name;
            if (cur_context.empty()) {
                throw unexpected_end_of_command();
            }

            check_token(cur_context, token);
            while (token != "-map") {
                if (parse_output_name(cur_context, out_name)) {
                    add_vertex(result, out_name, 3);  // 3 -- output vertex
                    result.output_amount++;
                    out_ids.push_back(result.graph.index_by_name[out_name]);
                } else { // TODO  parse global options after mapping
                    parse_token(cur_context, token); // to pass through options
                }
                if (cur_context.empty()) break;
                check_token(cur_context, token); // to check if -map found
            }
            if (!out_ids.empty()) {
                if (out_ids.size() != 1) {
                    std::cerr << "Hard case. -map option with more than 1 outputs. Using only last one\n";

                    for (std::size_t inp_id: mapped_id) {
                        result.graph.edges.push_back({inp_id, out_ids.back(), ""});
                    }
                    mapped_id.clear();
                    out_ids.clear();
                }
            }
        }
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
            parse_mapping(cur_context, result);
        } catch (std::exception &e) {
            std::cerr << e.what();
            return 1;
        }
        return 0;
    }



    //  exceptions

    expected_filename::expected_filename(std::size_t
                                         error_pos)
            : std::runtime_error("Expected file name after '-i' on position " + std::to_string(error_pos) + "\n") {
    }

    unexpected_option_prefix::unexpected_option_prefix(std::size_t
                                                       error_pos)
            : std::runtime_error("Parsing error at position " + std::to_string(error_pos) +
                                 ". Expected that '-i' prefix reserved for input file options\n") {
    }

    expected_quote::expected_quote(std::size_t
                                   error_pos)
            : std::runtime_error("Expected quotation mark at position " + std::to_string(error_pos) + "\n") {
    }

    no_closed_paren::no_closed_paren(std::size_t
                                     error_pos)
            : std::runtime_error(
            "Expected closed paren. Last open found on position " + std::to_string(error_pos) + "\n") {
    }

    unexpected_end_of_command::unexpected_end_of_command() : std::runtime_error("Unexpected end of command\n") {
    }

    unexpected_char_after_filter::unexpected_char_after_filter(std::size_t
                                                               error_pos)
            : std::runtime_error(
            "Unexpected character after filter att position " + std::to_string(error_pos) + "\n") {
    }

    empty_name::empty_name(std::size_t
                           error_pos) : std::runtime_error(
            "Name can't be empty. Error at position " + std::to_string(error_pos) + "\n") {
    }

    incorrect_reference::incorrect_reference(std::size_t
                                             error_pos) : std::runtime_error(
            "Incorrect reference at position " + std::to_string(error_pos) + "\n") {
    }

    incorrect_output_name::incorrect_output_name(std::size_t
                                                 error_pos) : std::runtime_error(
            "Incorrect output file name at position " + std::to_string(error_pos) + "\n") {
    }

    expected_stream_name::expected_stream_name(std::size_t
                                               error_pos) : std::runtime_error(
            "Expected stream name in brackets after '-map' option at position " + std::to_string(error_pos) + "\n") {
    }


}