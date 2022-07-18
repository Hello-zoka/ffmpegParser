#include "../include/parser.h"

namespace ffmpeg_parse {
    std::string convert_graph(const graph &graph) { // converting to graphviz
        std::string result = "digraph {\n";
        for (std::size_t ind = 0; ind < graph.names.size(); ind++) {
            std::string shape;
            switch (graph.vertex_type[ind]) {
                case 0:
                case 1:
                case 3:
                    shape = "circle";
                    break;
                case 2:
                    shape = "rectangle";
                    break;
                default:
                    throw incorrect_vertex_type(graph.names[ind], graph.vertex_type[ind]);
            }
            result += "  \"" + graph.names[ind]+ "\" [shape=" + shape + "]\n";
        }
        for (const edge &cur_edg: graph.edges) {
            result += "  \"" + graph.names[cur_edg.from] + "\" -> \"" + graph.names[cur_edg.to] + "\" [label=\"" + cur_edg.label +
                      "\"]\n";
        }
        result += "}\n";
        return result;
    }
}