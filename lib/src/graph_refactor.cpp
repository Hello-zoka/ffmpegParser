#include "../include/parser.h"

namespace ffmpeg_parse {
    void dfs(std::size_t vertex, std::vector<int> &used, const std::vector<std::vector<std::size_t>> &adj_list) {
        used[vertex] = 1;
        for (std::size_t to: adj_list[vertex]) {
            if (used[to] == 1) {
                throw incorrect_graph("Found a cycle");
            }
            if (used[to] == 0) {
                dfs(to, used, adj_list);
            }
        }
        used[vertex] = 2;
    }


    void refactor_graph(graph &graph) {
        std::vector<std::vector<std::size_t>> adj_list(graph.names.size()); // adjacency list

        for (const auto &cur_edg: graph.edges) { // converting to adj list
            if (graph.type == 0 && graph.vertex_type[cur_edg.from] == 2 &&
                graph.vertex_type[cur_edg.to] == 3) { // mapping banned at filter chain
                throw incorrect_graph("Mapping of middle files are not allowed at filter chains graph");
            }
            adj_list[cur_edg.from].push_back(cur_edg.to);
        }
        for (const auto &cur_edg: graph.edges) {
            if (cur_edg.label == "-map" && adj_list[cur_edg.from].size() != 1) {
                throw incorrect_graph("Mapping not leaf file");
            }
        }

        for (std::size_t ind = 0; ind < graph.names.size(); ind++) { // auto deducting of output
            if (graph.type == 0 && graph.vertex_type[ind] == 1 && adj_list[ind].size() == 0) {
                if (graph.gl_out_pos.empty()) {
                    throw incorrect_graph("No output files for filter chain");
                }
                graph.edges.push_back({ind, graph.gl_out_pos.front(), "auto"});
                break;
            }
            if (graph.type == 1 && graph.vertex_type[ind] == 2 && adj_list[ind].size() == 0) {
                if (graph.gl_out_pos.empty()) {
                    throw incorrect_graph("No output files for one of filters");
                }
                graph.edges.push_back({ind, graph.gl_out_pos.front(), "auto"});
                break;
            }
        }

        std::vector<int> used(graph.names.size(), 0);
        int leafs_cnt = 0;

        for (std::size_t inp = 0; inp < graph.input_amount; inp++) {
            if (graph.vertex_type[inp] == 1 && adj_list[inp].empty()) {
                leafs_cnt++;
            }
            if (used[inp] != 0) {
                throw incorrect_graph("Incoming edges in input vertex");
            }
            dfs(inp, used, adj_list);
        }
        for (std::size_t vertex = 0; vertex < graph.names.size(); vertex++) {
            if (used[vertex] == 0) {
                throw incorrect_graph("Unused vertex with name" + graph.names[vertex]);
            }
            if (adj_list[vertex].size() == 0 && graph.vertex_type[vertex] != 3) {
                throw incorrect_graph("All graph's leafs have to be output files");
            }
        }
        if (graph.type == 0 && leafs_cnt > 1) {
            throw incorrect_graph("Filter chain graph can't have more than 1 leaf");
        }
    }
}
