#include "../include/parser.h"

namespace ffmpeg_parse {
std::string convert_graph(graph &graph) {  // converting to graphviz
    std::string result = "digraph {\n  rankdir=LR;\n";
    for (std::size_t ind = 0; ind < graph.names.size(); ind++) {
        std::string shape, color = "black";
        switch (graph.vertex_type[ind]) {
            case input:
                color = "red";
            case middle:
                shape = "ellipse";
                break;
            case mapped_output:
            case unlinked_output:
                shape = "ellipse";
                color = "blue";
                break;
            case filter:
                shape = "rectangle";
                break;
            default:
                throw incorrect_vertex_type(graph.names[ind],
                                            graph.vertex_type[ind]);
        }
        if (graph.names[ind][0] == '\'' || graph.names[ind][0] == '\"') {
            graph.names[ind].pop_back();
            graph.names[ind][0] = ' ';
        }
        result += "  \"" + graph.names[ind] + "\" [shape=" + shape +
                  ", color=" + color +
                  (graph.vertex_correctness[ind]
                       ? "]\n"
                       : ", fillcolor=red, style=filled]\n");
    }
    for (const edge &cur_edg : graph.edges) {
        result += "  \"" + graph.names[cur_edg.from] + "\" -> \"" +
                  graph.names[cur_edg.to] + "\" [label=\"" + cur_edg.label +
                  "\"" +
                  (!graph.vertex_correctness[cur_edg.from] &&
                           !graph.vertex_correctness[cur_edg.to]
                       ? ", color=red]\n"
                       : "]\n");
    }
    result += "}\n";
    return result;
}
}  // namespace ffmpeg_parse
