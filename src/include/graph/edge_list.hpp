#ifndef GRAPH_EDGE_LIST_HPP
#define GRAPH_EDGE_LIST_HPP

#include <graph/common.hpp>
#include <util/tagged_tuple.hpp>

namespace graph {

namespace property = util::property;
using util::tagged_tuple;

template<class VertexID, class EdgeID, class EdgeWeight>
struct edge_list_graph {
    typedef VertexID vertex_id_type;
    typedef EdgeID edge_id_type;
    typedef EdgeWeight edge_weight_type;
    typedef tagged_tuple<property::src<vertex_id_type>, property::dst<vertex_id_type>, property::weight<edge_weight_type>> edge_type;

    vertex_id_type n;
    vector<edge_type> m_edges;

    edge_list_graph() = default;

    template<class FwdIt>
    edge_list_graph(vertex_id_type n_, FwdIt first, FwdIt last) : n(n_), m_edges(first, last) {}

    vertex_id_type num_vertices() const {
        return n;
    }

    edge_id_type num_edges() const {
        return m_edges.size();
    }

    const auto& edges() const { return m_edges; }

    auto& edges() { return m_edges; }

};

}

#endif