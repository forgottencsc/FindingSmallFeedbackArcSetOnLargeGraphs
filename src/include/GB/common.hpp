#ifndef FAS_GB_COMMON_HPP
#define FAS_GB_COMMON_HPP
#include <stack>
#include <functional>
#include <util/random.hpp>
#include <graph/common.hpp>

namespace GB {

using namespace graph;
using util::rnd;

typedef vector<char> bisection;

template<class Graph>
using solver = std::function<bisection(const Graph&)>;

template<class Graph>
bisection random_bisection(const Graph& g) {
    const vertex_id_t<Graph> n = g.num_vertices();
    vector<vertex_id_t<Graph>> v = util::rnd_perm(n);
    bisection b(n, 0);
    for (vertex_id_t<Graph> i = 0; i < n / 2; ++i)
        b[v[i]] = 1;
    return b;
}

}

#endif