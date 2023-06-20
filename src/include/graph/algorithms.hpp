#ifndef GRAPH_ALGORITHMS_HPP
#define GRAPH_ALGORITHMS_HPP
#include <algorithm>
#include <vector>
#include <graph/common.hpp>
#include <util/histogram_sort.hpp>

// #include <iostream>
// using namespace std;
// template<class Graph>
// void print_graph(const Graph& g) {
//     cout << g.num_vertices() << ' ' << g.num_edges() << endl;
//     for (auto e : g.edges()) {
//         cout << e.src << ' ' << e.dst << ' ' << e.weight << endl;
//     }
//     cout << endl;
// }

namespace graph {

using std::forward;
using std::pair;
using std::vector;

template<class Graph>
Graph reorder(const Graph& g, const vector<vertex_id_t<Graph>>& id) {
    typedef edge_t<Graph> edge_type;
    typedef vertex_id_t<Graph> vertex_id;
    vector<edge_t<Graph>> ev(g.edges().begin(), g.edges().end());
    for (edge_t<Graph>& e : ev) {
        e.src = id[e.src];
        e.dst = id[e.dst];
    }
    return Graph(g.num_vertices(), ev.begin(), ev.end());
}

template<class RndIt, class Fn>
RndIt merge_parallel_edges(RndIt begin_, RndIt end_, Fn&& fn) {
    if (begin_ == end_) return begin_;
    typedef typename std::iterator_traits<RndIt>::value_type value_type;
    util::histogram_sort(begin_, end_, src<>(), dst<>());
    return std::unique(begin_, end_, forward<Fn>(fn));
}

template<class T>
auto weight_add = [] (T& t1, T& t2) -> bool {
    if (t1.src != t2.src || t1.dst != t2.dst)
        return false;
    t1.weight += t2.weight;
    return true;
};


}

#endif