#include "Delaunay3D.h"

#include <utility>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Delaunay_triangulation_cell_base_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

#include "../Assert.h"

#include <iostream>

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Vb = CGAL::Triangulation_vertex_base_with_info_3<size_t, K>;
using Cb = CGAL::Delaunay_triangulation_cell_base_3<K>;
using Tds = CGAL::Triangulation_data_structure_3<Vb, Cb>;
// Use the Fast_location tag. Default or Compact_location works too.
using Delaunay = CGAL::Delaunay_triangulation_3<K, Tds, CGAL::Fast_location>;
using Point = Delaunay::Point;

std::vector<Edge> Delaunay3D(const std::vector<glm::vec3>& pointsVec) {
    auto points = std::vector<std::pair<Point, size_t>>();
    for (size_t i = 0; i < pointsVec.size(); ++i) {
        const auto& point = pointsVec[i];
        points.push_back(std::make_pair(Point(point.x, point.y, point.z), i));
    }

    auto T = Delaunay(points.begin(), points.end());
    LOG_ASSERT(T.number_of_vertices() == pointsVec.size());
    LOG_ASSERT(T.is_valid());

    for (const auto& v : T.finite_vertex_handles()) {
        LOG_ASSERT(points[v->info()].first == v->point());
    }

    auto edges = std::vector<Edge>();
    for (const auto& e : T.finite_edges()) {
        const auto v1 = e.first->vertex(e.second)->info();
        const auto v2 = e.first->vertex(e.third)->info();

        edges.push_back({v1, v2});
    }

    return edges;
}

bool Edge::operator==(const Edge& other) const {
    return v1 == other.v1 && v2 == other.v2;
}

size_t Edge::HashFunction::operator()(const Edge& edge) const {
    size_t v1Hash = std::hash<size_t>()(edge.v1);
    size_t v2Hash = std::hash<size_t>()(edge.v2) << 1;
    return v1Hash ^ v2Hash;
}
