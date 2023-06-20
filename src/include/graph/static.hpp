#ifndef GRAPH_STATIC_HPP
#define GRAPH_STATIC_HPP

#include <graph/common.hpp>
#include <util/tagged_tuple.hpp>
#include <util/histogram_map.hpp>

namespace graph {

using util::tagged_tuple;
using util::get;
using util::histogram_map;

using util::property::src;
using util::property::dst;
using util::property::weight;

template<class VertexSizeType, class EdgeSizeType, class EdgeWeightType, bool SortedEdges>
struct static_graph_traits {
    
    typedef VertexSizeType vertex_id_type;

    typedef EdgeSizeType edge_id_type;

    typedef EdgeWeightType edge_weight_type;

    typedef tagged_tuple<src<vertex_id_type>, dst<vertex_id_type>, weight<edge_weight_type>> edge_type;
    typedef tagged_tuple<src<vertex_id_type>, weight<edge_weight_type>> in_edge_type;
    typedef tagged_tuple<dst<vertex_id_type>, weight<edge_weight_type>> out_edge_type;
    
    typedef histogram_map<src<vertex_id_type>, 
        std::conditional_t<SortedEdges, dst<vertex_id_type>, void>,
        out_edge_type, edge_id_type> out_edge_container;
    typedef histogram_map<dst<vertex_id_type>,
        std::conditional_t<SortedEdges, src<vertex_id_type>, void>,
        in_edge_type, edge_id_type> in_edge_container;

};

template<class VertexSizeType, class EdgeSizeType, class EdgeWeightType, bool SortedEdges = true>
class static_graph {
public:
    typedef static_graph<VertexSizeType, EdgeSizeType, EdgeWeightType, SortedEdges> this_type;

    typedef static_graph_traits<VertexSizeType, EdgeSizeType, EdgeWeightType, SortedEdges> traits;

    typedef typename traits::vertex_id_type vertex_id_type;
    typedef typename traits::edge_id_type edge_id_type;

    typedef typename traits::edge_weight_type edge_weight_type;

    typedef typename traits::edge_type edge_type;

    typedef typename traits::out_edge_container out_edge_container;
    typedef typename traits::in_edge_container in_edge_container;

    typedef typename out_edge_container::value_type out_edge_type;
    typedef typename in_edge_container::value_type in_edge_type;

    typedef typename out_edge_container::slice out_adjacency_container;
    typedef typename in_edge_container::slice in_adjacency_container;

    typedef typename out_edge_container::const_iterator out_edge_iterator;
    typedef typename in_edge_container::const_iterator in_edge_iterator;

private:
    vertex_id_type m_n;
    in_edge_container m_in_edges;
    out_edge_container m_out_edges;

public:
    static_graph() : m_n(0) {}
    static_graph(const this_type&) = default;
    static_graph(this_type&&) noexcept = default;

    this_type& operator=(const this_type&) = default;
    this_type& operator=(this_type&&) = default;

    ~static_graph() = default;

    template<class FwdIt>
    static_graph(vertex_id_type n, FwdIt begin_, FwdIt end_) :
        m_n(n),
        m_in_edges(n, begin_, end_, util::elements_are_unsorted),
        m_out_edges(n, begin_, end_, util::elements_are_unsorted)
        {}

    vertex_id_type num_vertices() const { return m_n; }

    edge_id_type num_edges() const { return m_out_edges.size(); }

    const out_edge_container& edges() const {
        return m_out_edges;
    }

    in_adjacency_container in_edges(vertex_id_type v) const {
        return m_in_edges[v];
    }

    out_adjacency_container out_edges(vertex_id_type v) const {
        return m_out_edges[v];
    }

};


}


#endif