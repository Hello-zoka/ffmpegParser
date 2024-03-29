#include "../include/parser.h"
#include <fstream>
#include <iostream>
#include <string>

namespace ffmpeg_parse {

void skip_spaces(parse_context &cur_context) {
    while (!cur_context.empty() &&
           isspace(cur_context.command[cur_context.pos])) {
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
    while (pos < cur_context.command.size() &&
           isspace(cur_context.command[pos])) {
        pos++;
    }
    while (pos < cur_context.command.size() &&
           !isspace(cur_context.command[pos])) {
        result += cur_context.command[pos++];
    }
}

bool parse_option(parse_context &cur_context,
                  std::size_t pos,
                  const std::string &option,
                  bool separated = true) {  // checking token at pos index
    skip_spaces(cur_context);
    if (cur_context.command.size() - pos + 1 < option.size()) {
        return false;
    }
    if (cur_context.command.compare(pos, option.size(), option) == 0) {
        std::string null;
        cur_context.pos = pos + option.size();
        return !separated ||
               cur_context.command.size() <= pos + option.size() ||
               isspace(cur_context
                           .command[pos + option.size()]);  // check space after
                                                            // separated option
    }
    return false;
}

void add_vertex(graph &result, std::string &name, VertexType type) {
    if (type == 2) {  // making all filters node different
        name += " (" + std::to_string(result.names.size()) + ")";
    }
    if (result.index_by_name.count(name) == 0) {
        result.index_by_name[name] = result.names.size();
        result.names.push_back(name);
        result.vertex_type.push_back(type);
        result.vertex_correctness.push_back(true);
    }
}

void parse_inputs(parse_context &cur_context, graph &result) {
    for (std::size_t ind = cur_context.pos; ind < cur_context.command.size();
         ind++) {
        if (parse_option(cur_context, ind, input_option)) {
            std::string file_name, null;
            parse_token(cur_context, file_name);
            if (file_name.empty()) {
                throw expected_filename(cur_context.pos);
            }
            std::ifstream check_file(file_name.c_str());
            if (!check_file.good()) {
                std::cerr << "Can't find such file as " + file_name << '\n';
            }
            add_vertex(result, file_name, input);
            result.input_amount++;
        }
    }
}

void parse_reference(parse_context &cur_context,
                     graph &result,
                     std::string &reference) {
    std::size_t first_colon_pos = reference.size();
    for (std::size_t ind = 0; ind < reference.size(); ind++) {
        if (reference[ind] == ':') {
            first_colon_pos = ind;
            break;
        }
    }
    while (static_cast<int>(reference.size()) > first_colon_pos) {
        reference.pop_back();
    }
    std::size_t index;
    try {
        index = std::stoi(reference);
        if (index >= result.input_amount) {
            throw incorrect_reference(cur_context.pos);
        }
        reference = result.names.at(index);
    } catch (...) {
        throw incorrect_reference(cur_context.pos);
    }
}

void parse_names(parse_context &cur_context,
                 graph &result,
                 std::vector<std::size_t> &id) {
    // only add to id's vector
    skip_spaces(cur_context);
    //  TODO parse quotes around brackets
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
        add_vertex(result, name, middle);
        id.push_back(result.index_by_name[name]);
        skip_spaces(cur_context);
    }
}

void parse_filter(parse_context &cur_context, graph &result) {
    std::vector<std::size_t> inputs, outputs, logs;
    std::vector<std::string> logs_label;
    std::string command, log_file, log_file_option;
    bool log_file_state = false;
    parse_names(cur_context, result, inputs);
    while (!cur_context.empty() && cur_context.check_char() != '[' &&
           cur_context.check_char() != ';') {
        for (auto &log_option : log_options) {
            if (parse_option(cur_context, cur_context.pos, log_option + '=',
                             false)) {
                log_file_state = true;
                log_file_option = log_option;
                command += log_option + '=';
            }
        }
        if (cur_context.empty()) {  // after success option parse should be more
                                    // than 0 chars
            throw unexpected_end_of_command();
        }

        if (cur_context.check_char() == ',' ||
            cur_context.check_char() == ':') {  // end of flag
            if (log_file_state) {
                logs.push_back(result.names.size());
                add_vertex(result, log_file, mapped_output);
                logs_label.push_back(log_file_option);
            }
            log_file_state = false;
            log_file_option = "";
            log_file = "";
        }
        if (log_file_state) {
            log_file += cur_context.check_char();
        }
        command += cur_context.get_char();
    }
    if (log_file_state) {
        logs.push_back(result.names.size());
        add_vertex(result, log_file, mapped_output);
        logs_label.push_back(log_file_option);
    }
    parse_names(cur_context, result, outputs);

    add_vertex(result, command, filter);
    std::size_t command_id = result.index_by_name[command];

    if (inputs.empty()) {  // You can add here some more complicated logic
        inputs.push_back(0);
    }

    for (std::size_t ind = 0; ind < logs.size(); ind++) {
        result.edges.push_back({command_id, logs[ind], logs_label[ind]});
    }

    for (std::size_t from : inputs) {
        result.edges.push_back({from, command_id, ""});
    }
    for (std::size_t to : outputs) {
        result.edges.push_back({command_id, to, ""});
    }
}

void parse_graph(parse_context &cur_filter, graph &result) {
    skip_spaces(cur_filter);
    while (!cur_filter.empty()) {
        parse_filter(cur_filter, result);
        if (cur_filter.empty()) {
            break;
        }
        if (cur_filter.get_char() != ';') {
            throw unexpected_char_after_filter(cur_filter.pos);
        }
    }
}

void parse_quotes_filter(parse_context &cur_context,
                         std::string &result) {  // parsing filters by separator
    skip_spaces(cur_context);
    if (cur_context.empty())
        return;
    char sep = ' ', ch;
    if (cur_context.check_char() == '\"') {
        sep = '\"';
        cur_context.get_char();
    }
    while (!cur_context.empty()) {
        ch = cur_context.get_char();
        if ((sep == ' ' && isspace(ch)) || sep == ch)
            break;
        result += ch;
    }
}

bool is_out_name(const std::string &name) {  // checking '.' in name
    int count_dots = 0;
    for (char letter : name) {
        if (letter == '.') {
            count_dots++;
        }
    }
    return count_dots == 1;
}

bool parse_output_name(parse_context &cur_context, std::string &out_name) {
    std::string token;
    check_token(cur_context, token);
    if (token == file_option) {
        parse_token(cur_context, token);     // -f flag
        parse_token(cur_context, token);     // format option
        parse_token(cur_context, out_name);  // out name
        if (out_name.empty()) {
            throw incorrect_output_name(cur_context.pos);
        }
        if (out_name[0] == '-' && out_name.size() != 1) {
            std::cerr << "Warning! File name should be just after '-f' option. "
                         "Found another option\n";
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

void relax_bad_flag(
    bool &prev_bad,
    const std::string &token) {  // checking if option needs parameter with '.'
    if (bad_options.find(token) != bad_options.end() && dot_parsing) {
        prev_bad = true;
    } else {
        prev_bad = false;
    }
}

void parse_mapping(parse_context &cur_context, graph &result) {
    std::string token;
    std::vector<std::size_t> mapped_id;
    std::size_t out_id;
    bool prev_bad = false;
    while (!cur_context.empty()) {
        std::string out_name;
        if (!prev_bad && parse_output_name(cur_context, out_name)) {
            result.gl_out_pos.push_back(result.names.size());
            add_vertex(result, out_name, unlinked_output);
            continue;
        }
        parse_token(cur_context, token);  // skip option
        relax_bad_flag(prev_bad, token);

        if (token == map_option) {
            prev_bad = false;
            std::size_t prev_sz = mapped_id.size(),
                        prev_vertex_sz = result.names.size();
            parse_names(cur_context, result,
                        mapped_id);  // only push back new ids
            if (mapped_id.size() == prev_sz) {
                throw expected_stream_name(cur_context.pos);
            }
            if (result.names.size() != prev_vertex_sz) {
                throw incorrect_reference(cur_context.pos);
            }

            if (cur_context.empty()) {
                throw unexpected_end_of_command();
            }
            check_token(cur_context, token);
            while (token != map_option) {
                if (!prev_bad &&
                    parse_output_name(cur_context,
                                      out_name)) {  // found out file
                    add_vertex(result, out_name, mapped_output);
                    result.output_amount++;
                    out_id = result.index_by_name[out_name];
                    for (std::size_t inp_id : mapped_id) {
                        result.edges.push_back(
                            {inp_id, static_cast<std::size_t>(out_id),
                             map_option});
                    }
                    mapped_id.clear();
                    prev_bad = false;
                    break;
                } else {
                    parse_token(cur_context, token);  // to pass through options
                    relax_bad_flag(prev_bad, token);
                }
                if (cur_context.empty())
                    break;
                check_token(cur_context, token);  // to check next option
            }
        }
    }
}

void parse_to_graph(const std::string &command,
                    graph &result) {  // can throw exceptions
    parse_context cur_context{command, 0};

    parse_inputs(cur_context, result);
    std::string filters, token;
    check_token(cur_context, token);
    if (filter_tags.find(token) != filter_tags.end()) {
        parse_token(cur_context, token);
        parse_quotes_filter(cur_context, filters);
        parse_context filter_context{filters, 0};
        parse_graph(filter_context, result);
        if (token == "-filter_complex") {
            result.type = graph::complex;
        } else {
            result.type = graph::chain;
        }
    }
    parse_mapping(cur_context, result);
}

//  exceptions

expected_filename::expected_filename(std::size_t error_pos)
    : std::runtime_error("Expected file name after '-i' on position " +
                         std::to_string(error_pos) + "\n") {
}

no_closed_paren::no_closed_paren(std::size_t error_pos)
    : std::runtime_error("Expected closed paren. Last open found on position " +
                         std::to_string(error_pos) + "\n") {
}

unexpected_end_of_command::unexpected_end_of_command()
    : std::runtime_error("Unexpected end of command\n") {
}

unexpected_char_after_filter::unexpected_char_after_filter(
    std::size_t error_pos)
    : std::runtime_error("Unexpected character after filter att position " +
                         std::to_string(error_pos) + "\n") {
}

empty_name::empty_name(std::size_t error_pos)
    : std::runtime_error("Name can't be empty. Error at position " +
                         std::to_string(error_pos) + "\n") {
}

incorrect_reference::incorrect_reference(std::size_t error_pos)
    : std::runtime_error("Incorrect reference at position " +
                         std::to_string(error_pos) + "\n") {
}

incorrect_output_name::incorrect_output_name(std::size_t error_pos)
    : std::runtime_error("Incorrect output file name at position " +
                         std::to_string(error_pos) + "\n") {
}

expected_stream_name::expected_stream_name(std::size_t error_pos)
    : std::runtime_error(
          "Expected stream name in brackets after '-map' option at position " +
          std::to_string(error_pos) + "\n") {
}

incorrect_vertex_type::incorrect_vertex_type(const std::string &vertex_name,
                                             std::size_t vertex_type)
    : std::runtime_error("Incorrect vertex type " +
                         std::to_string(vertex_type) + " of vertex '" +
                         vertex_name + "'\n") {
}
}  // namespace ffmpeg_parse
