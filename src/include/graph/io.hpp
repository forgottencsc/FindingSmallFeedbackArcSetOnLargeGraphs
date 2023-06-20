#ifndef GRAPH_IO_HPP
#define GRAPH_IO_HPP
#include <iostream>
#include <vector>
#include <graph/common.hpp>

namespace graph {

using std::istream;
using std::string;
using std::vector;

using std::max;

//  Simple weighted digraph
//  n m
//  u1 v1 w1
//  u2 v2 w2
//  ...
template<class Graph>
pair<vertex_id_t<Graph>, vector<edge_t<Graph>>> read_graph_w(istream& is) {
    vertex_id_t<Graph> n;
    typename Graph::edge_id_type m;
    is >> n >> m;

    vector<edge_t<Graph>> es;
    es.reserve(m);
    for (typename Graph::edge_id_type i = 0; i < m; ++i) {
        vertex_id_t<Graph> u, v;
        edge_weight_t<Graph> w;
        is >> u >> v >> w;
        if (w == 0) continue;
        es.emplace_back(u, v, w);
        n = max(n, max(u, v) + 1);
    }
    return { n, es };
}

//  Simple unweighted digraph
//  n m
//  u1 v1
//  u2 v2
//  ...
template<class Graph>
pair<vertex_id_t<Graph>, vector<edge_t<Graph>>> read_graph_u(istream& is) {
    vertex_id_t<Graph> n;
    typename Graph::edge_id_type m;
    is >> n >> m;

    vector<edge_t<Graph>> es; es.reserve(m);
    for (typename Graph::edge_id_type i = 0; i < m; ++i) {
        vertex_id_t<Graph> u, v;
        is >> u >> v;
        es.emplace_back(u, v, 1);
        n = max(n, max(u, v) + 1);
    }
    return { n, es };
}

// Example: s27.d from ISCAS dataset
// p <name> <n> <m>
// a <u> <v> <w> <t>
// a <u> <v> <w> <t>
// ...
template<class Graph>
pair<vertex_id_t<Graph>, vector<edge_t<Graph>>> read_graph_dw(istream& is) {
    string token;
    is >> token;
    is >> token;
    vertex_id_t<Graph> n;
    typename Graph::edge_id_type m;
    is >> n >> m;

    vector<edge_t<Graph>> es; es.reserve(m);
    for (typename Graph::edge_id_type i = 0; i < m; ++i) {
        is >> token;
        vertex_id_t<Graph> u, v;
        edge_weight_t<Graph> w, t;
        is >> u >> v >> w >> t;
        u--; v--;
        es.emplace_back(u, v, w);
        n = max(n, max(u, v) + 1);
    }
    return { n, es };
}


// Example: s27.d from ISCAS dataset
// Ignore weights
// p <name> <n> <m>
// a <u> <v> <w> <t>
// a <u> <v> <w> <t>
// ...
template<class Graph>
pair<vertex_id_t<Graph>, vector<edge_t<Graph>>> read_graph_du(istream& is) {
    string token;
    is >> token;
    is >> token;
    vertex_id_t<Graph> n;
    typename Graph::edge_id_type m;
    is >> n >> m;

    vector<edge_t<Graph>> es; es.reserve(m);
    for (typename Graph::edge_id_type i = 0; i < m; ++i) {
        is >> token;
        vertex_id_t<Graph> u, v;
        edge_weight_t<Graph> w, t;
        is >> u >> v >> w >> t;
        u--; v--;
        es.emplace_back(u, v, 1);
        n = max(n, max(u, v) + 1);
    }
    // for (const edge_t<Graph>& e : es)
    //     assert(0 <= (e.src) && (e.src) < n && 0 <= (e.dst) && (e.dst) < n);
    return { n, es };
}

// Default SNAP Format
// Example(first 4 lines in soc-Epinions1.txt):
// # Directed graph (each unordered pair of nodes is saved once): soc-Epinions1.txt 
// # Directed Epinions social network
// # Nodes: 75879 Edges: 508837
// # FromNodeId	ToNodeId
template<class Graph>
pair<vertex_id_t<Graph>, vector<edge_t<Graph>>> read_graph_su(istream& is) {
    string token;
    getline(is, token);
    getline(is, token);
    vertex_id_t<Graph> n;
    typename Graph::edge_id_type m;
    is >> token >> token >> n >> token >> m;
    getline(is, token);
    getline(is, token);


    vector<edge_t<Graph>> es; es.reserve(m);
    for (typename Graph::edge_id_type i = 0; i < m; ++i) {
        vertex_id_t<Graph> u, v;
        // edge_weight_t<Graph> w;
        is >> u >> v;
        es.emplace_back(u, v, 1);
        n = max(n, max(u, v) + 1);
    }
    return { n, es };
}

//  Simple unweighted digraph without n&m
//  u1 v1
//  u2 v2
//  ...
template<class Graph>
pair<vertex_id_t<Graph>, vector<edge_t<Graph>>> read_graph_lu(istream& is) {
    vector<edge_t<Graph>> es;
    vertex_id_t<Graph> u, v, n = 0;
    while (is >> u >> v) {
        es.emplace_back(u, v, 1);
        n = max(n, max(u, v) + 1);
    }
    return { n, es };
}

template<class Graph>
pair<vertex_id_t<Graph>, vector<edge_t<Graph>>> read_graph_lo(istream& is) {
    vertex_id_t<Graph> n;
    is >> n;
    vector<edge_t<Graph>> es; es.reserve(n * n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            vertex_id_t<Graph> w; is >> w;
            es.emplace_back(i, j, w);
        }
    return { n, es };
}

template<class Graph>
pair<vertex_id_t<Graph>, vector<edge_t<Graph>>> read_graph_impl(istream& is, const string& t) {
    if (t == "U")
        return read_graph_u<Graph>(is);
    else if (t == "W")
        return read_graph_w<Graph>(is);
    else if (t == "DW")
        return read_graph_dw<Graph>(is);
    else if (t == "DU")
        return read_graph_du<Graph>(is);
    else if (t == "SU")
        return read_graph_su<Graph>(is);
    else if (t == "LU")
        return read_graph_lu<Graph>(is);    
    else if (t == "LO")
        return read_graph_lo<Graph>(is);
    else
        return {};
}

template<class Graph, class Fn>
Graph read_graph(istream& is, const string& t, Fn fn) {
    auto res = read_graph_impl<Graph>(is, t);
    for (edge_t<Graph>& e : res.second)
        fn(e);
    return Graph(res.first, res.second.begin(), res.second.end());
}

template<class Graph>
Graph read_graph(istream& is, const string& t) {
    auto res = read_graph_impl<Graph>(is, t);
    return Graph(res.first, res.second.begin(), res.second.end());
}

template<class Graph>
void write_graph_w(const Graph& g, std::ostream& os) {
    os << g.num_vertices() << ' ' << g.num_edges() << std::endl;
    for (const edge_t<Graph>& e : g.edges())
        os << e.src << ' ' << e.dst << ' ' << e.weight << std::endl;
}

template<class Graph>
void write_graph_u(const Graph& g, std::ostream& os) {
    os << g.num_vertices() << ' ' << g.num_edges() << std::endl;
    for (const edge_t<Graph>& e : g.edges())
        os << e.src << ' ' << e.dst << std::endl;
}

}

#endif