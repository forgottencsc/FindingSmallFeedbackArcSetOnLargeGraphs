#ifndef GRAPH_COMMON_HPP
#define GRAPH_COMMON_HPP
#include <vector>
#include <utility>
#include <util/tagged_tuple.hpp>

namespace graph {
    using std::pair;
    using std::vector;

    using util::property::src;
    using util::property::dst;

    template<class Graph>
    using vertex_id_t = typename Graph::vertex_id_type;

    template<class Graph>
    using edge_id_t = typename Graph::edge_id_type;

    template<class Graph>
    using edge_weight_t = typename Graph::edge_weight_type;

    template<class Graph>
    using edge_t = typename Graph::edge_type;

    //  (color_num, color of each vertex)
    template<class Graph>
    using coloring_result = pair<vertex_id_t<Graph>, vector<vertex_id_t<Graph>>>;

}

#endif