#include "MST.h"

#include <iterator>
#include <utility>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, property<edge_weight_t, Weight>>;
using B_Edge = graph_traits<Graph>::edge_descriptor;
using Vertex = graph_traits<Graph>::vertex_descriptor;
using E = std::pair<size_t, size_t>;

std::vector<Edge> MinimumSpanningTree(const std::vector<Edge>& graph, size_t vertices, const std::vector<Weight>& weights) {
    auto edgeVec = std::vector<E>();
    for (const auto& edge : graph) {
        edgeVec.push_back({edge.v1, edge.v2});
    }
    auto g = Graph(edgeVec.begin(), edgeVec.end(), weights.begin(), vertices);

    auto spanning_tree = std::vector<B_Edge>();

    kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));

    auto edges = std::vector<Edge>();

    for (const auto& edge : spanning_tree) {
        edges.push_back(Edge{source(edge, g), target(edge, g)});
    }

    return edges;
}
