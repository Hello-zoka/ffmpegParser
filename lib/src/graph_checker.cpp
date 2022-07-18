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


    void check_graph(const graph &graph) {
        std::vector<std::vector<std::size_t>> adj_list(graph.names.size()); // adjacency list

        for (const auto &cur_edg: graph.edges) { // converting to adj list
            adj_list[cur_edg.from].push_back(cur_edg.to);
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