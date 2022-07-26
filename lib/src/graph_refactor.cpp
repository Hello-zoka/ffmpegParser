#include <iostream>
#include "../include/parser.h"

namespace ffmpeg_parse {
enum VertexStatus { unused, active, visited, error };

void dfs(std::size_t vertex,
         std::vector<VertexStatus> &used,
         const std::vector<std::vector<std::size_t>> &adj_list,
         int &cycle_found) {
    used[vertex] = active;
    for (std::size_t to : adj_list[vertex]) {
        if (used[to] == active) {
            std::cerr << "Found a cycle\n";
            cycle_found = 1;
            used[vertex] = error;
            return;
        }
        if (used[to] == unused) {
            dfs(to, used, adj_list, cycle_found);
            if (cycle_found != 0) {
                if (used[vertex] == error) {
                    cycle_found = 2;  // end of cycle coloring
                }
                if (cycle_found == 1) {  // inside cycle
                    used[vertex] = error;
                }
                return;
            }
        }
    }
    used[vertex] = visited;
}

void refactor_graph(graph &graph) {
    std::vector<std::vector<std::size_t>> adj_list(
        graph.names.size());  // adjacency list

    for (const auto &cur_edg : graph.edges) {  // converting to adj list
        if (graph.type == graph::chain &&
            graph.vertex_type[cur_edg.from] == filter &&
            graph.vertex_type[cur_edg.to] ==
                mapped_output) {  // mapping banned at filter chain
            graph.vertex_correctness[cur_edg.to] = false;
            std::cerr << "Mapping of middle files are not allowed at filter "
                         "chains graph\n";
        }
        adj_list[cur_edg.from].push_back(cur_edg.to);
    }
    for (const auto &cur_edg : graph.edges) {
        if (cur_edg.label == "-map" && adj_list[cur_edg.from].size() != 1 &&
            graph.vertex_type[cur_edg.from] != input) {
            graph.vertex_correctness[cur_edg.from] = false;
            graph.vertex_correctness[cur_edg.to] = false;
            std::cerr << "Mapping not leaf file\n";
        }
    }

    for (std::size_t ind = 0; ind < graph.names.size();
         ind++) {  // auto deducting of output
        if (((graph.type == graph::chain && graph.vertex_type[ind] == middle) ||
             (graph.type == graph::complex &&
              graph.vertex_type[ind] == filter)) &&
            adj_list[ind].size() == 0) {
            if (graph.gl_out_pos.empty()) {
                graph.vertex_correctness[ind] = false;
                std::cerr << "No output files for filter chain\n";
            }
            graph.edges.push_back({ind, graph.gl_out_pos.front(), "auto"});
            adj_list[ind].push_back(graph.gl_out_pos.front());
            break;
        }
    }

    std::vector<VertexStatus> used(graph.names.size(), unused);
    int leafs_cnt = 0;

    for (std::size_t inp = 0; inp < graph.input_amount; inp++) {
        if (graph.vertex_type[inp] == middle && adj_list[inp].empty()) {
            leafs_cnt++;
        }
        if (used[inp] != unused) {
            graph.vertex_correctness[inp] = false;
            std::cerr << "Incoming edges in input vertex\n";
        }
        int cycle_found = 0;
        dfs(inp, used, adj_list, cycle_found);
        if (cycle_found != 0) {  // marking cycle as an error
            for (std::size_t ind = 0; ind < graph.input_amount; inp++) {
                if (used[ind] == error) {
                    graph.vertex_correctness[inp] = false;
                }
            }
            break;
        }
    }
    for (std::size_t vertex = 0; vertex < graph.names.size(); vertex++) {
        if (used[vertex] == unused) {
            graph.vertex_correctness[vertex] = false;
            std::cerr << "Unused vertex with name " + graph.names[vertex]
                      << "\n";
        }
        if (adj_list[vertex].size() == 0 &&
            graph.vertex_type[vertex] < 3) {  // not output files
            graph.vertex_correctness[vertex] = false;
            std::cerr << "All graph's leafs have to be output files\n";
        }
    }
    if (graph.type == 0 && leafs_cnt > 1) {
        for (std::size_t inp = 0; inp < graph.input_amount; inp++) {
            if (graph.vertex_type[inp] == middle && adj_list[inp].empty()) {
                graph.vertex_correctness[inp] = false;
            }
        }
        std::cerr << "Filter chain graph can't have more than 1 leaf\n";
    }
}
}  // namespace ffmpeg_parse
