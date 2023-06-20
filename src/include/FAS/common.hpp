#ifndef FAS_COMMON_HPP
#define FAS_COMMON_HPP
#include <vector>
#include <functional>
#include <graph/common.hpp>

namespace FAS {

using namespace graph;

using std::forward;
using std::move;
using std::get;
using std::swap;
using std::max;
using std::min;
using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using std::function;
using std::vector;

template<class Graph>
using solution = vector<vertex_id_t<Graph>>;

template<class Graph>
using solver = function<solution<Graph>(const Graph&)>;    

template<class Graph>
using improver = function<solution<Graph>(const Graph& g, const solution<Graph>&)>;

template<class Graph>
using perturber = function<solution<Graph>(const Graph& g, const solution<Graph>&)>;

template<class Graph>
using crossover = function<solution<Graph>(const Graph& g, const solution<Graph>&, const solution<Graph>&)>;

template<class Graph>
solution<Graph> inv(const solution<Graph>& a) {
    const vertex_id_t<Graph> n = a.size();
    solution<Graph> b(n);
    for (vertex_id_t<Graph> i = 0; i < n; ++i)
        b[a[i]] = i;
    return b;
}

template<class Graph>
edge_weight_t<Graph> get_edge_weight_sum(const Graph& g) {
    edge_weight_t<Graph> wsum = 0;
    for (const edge_t<Graph>& e : g.edges())
        wsum += e.weight;
    return wsum;
}

template<class Graph>
edge_weight_t<Graph> get_lo_weight(const Graph& g, const solution<Graph>& rk) {
    edge_weight_t<Graph> wsum = 0;
    for (const edge_t<Graph>& e : g.edges())
        if (rk[e.src] < rk[e.dst])
            wsum += e.weight;
    return wsum;
}

template<class Graph>
edge_weight_t<Graph> get_fas_weight(const Graph& g, const solution<Graph>& rk) {
    if (rk.size() != (size_t)g.num_vertices())
        return -1;
    edge_weight_t<Graph> wsum = 0;
    for (const edge_t<Graph>& e : g.edges())
        if (rk[e.src] >= rk[e.dst])
            wsum += e.weight;
    return wsum;
}


template<class Graph>
solution<Graph> trivial_solver(const Graph& g) {
    typedef vertex_id_t<Graph> vertex_id;
    vertex_id n = g.num_vertices();
    solution<Graph> rk1(n), rk2(n);
    for (vertex_id i = 0; i < n; ++i)
        rk1[i] = i, rk2[i] = n - i - 1;
    return get_fas_weight(g, rk1) < get_fas_weight(g, rk2) ? rk1 : rk2;
}
    
}

#endif