#include "../include/parser.h"

namespace ffmpeg_parse {
    std::string convert_graph(const graph &graph) { // converting to graphviz
        std::string result = "digraph {\n  rankdir=LR;\n";
        for (std::size_t ind = 0; ind < graph.names.size(); ind++) {
            std::string shape, color = "black";
            switch (graph.vertex_type[ind]) {
                case 0:
                case 1:
                    shape = "circle";
                    break;
                case 3:
                case 4:
                    shape = "circle";
                    color = "blue";
                    break;
                case 2:
                    shape = "rectangle";
                    break;
                default:
                    throw incorrect_vertex_type(graph.names[ind], graph.vertex_type[ind]);
            }
            result += "  \"" + graph.names[ind]+ "\" [shape=" + shape + ", color=" + color + "]\n";
        }
        for (const edge &cur_edg: graph.edges) {
            result += "  \"" + graph.names[cur_edg.from] + "\" -> \"" + graph.names[cur_edg.to] + "\" [label=\"" + cur_edg.label +
                      "\"]\n";
        }
        result += "}\n";
        return result;
    }
}