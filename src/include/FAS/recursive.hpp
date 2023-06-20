#ifndef FAS_SOLVER_RECURSIVE_HPP
#define FAS_SOLVER_RECURSIVE_HPP
#include <tuple>
#include <graph/algorithms.hpp>
#include <FAS/common.hpp>

namespace FAS {

using std::get;
using std::decay_t;
using std::integral_constant;
using std::tuple;

template<class Graph>
using partition_result = pair<Graph, vector<pair<vector<vertex_id_t<Graph>>, Graph>>>;

template<class Graph>
Graph coalease(const Graph& g, const coloring_result<Graph>& c) {
    vector<edge_t<Graph>> es;
    for (const edge_t<Graph>& e : g.edges()) {
        vertex_id_t<Graph> u = c.second[e.src], v = c.second[e.dst];
        if (u != v) es.emplace_back(u, v, e.weight);
    }
    return Graph(c.first, es.begin(), merge_parallel_edges(es.begin(), es.end(), weight_add<edge_t<Graph>>));
}

template<class Graph>
partition_result<Graph> partition(const Graph& g, const coloring_result<Graph>& c) {
    typedef edge_t<Graph> edge_type;
    typedef vertex_id_t<Graph> vertex_id;
    const vertex_id n = g.num_vertices();
    vector<edge_type> outer_edges(g.edges().begin(), g.edges().end());
    vertex_id n_color = c.first;
    const vector<vertex_id>& color = c.second;
    auto it = std::partition(outer_edges.begin(), outer_edges.end(), [&](edge_type e) { return color[e.src] != color[e.dst]; });

    vector<vertex_id> cnt(n_color, 0);
    vector<vertex_id> id(n);
    vector<vector<vertex_id>> e_id(n_color);
    for (vertex_id i = 0; i < n; ++i) {
        id[i] = e_id[color[i]].size();
        e_id[color[i]].push_back(i);
    }
    
    vector<vector<edge_type>> inner_edges(n_color);
    for (auto it2 = outer_edges.begin(); it2 != it; ++it2) {
        it2->src = color[it2->src];
        it2->dst = color[it2->dst];
    }

    for (auto it2 = it; it2 != outer_edges.end(); ++it2)
        inner_edges[color[it2->src]].emplace_back(id[it2->src], id[it2->dst], it2->weight);

    vector<pair<vector<vertex_id>, Graph>> res;
    for (vertex_id i = 0; i < n_color; ++i)
        res.emplace_back(e_id[i], Graph(e_id[i].size(), inner_edges[i].begin(), merge_parallel_edges(inner_edges[i].begin(), inner_edges[i].end(), weight_add<edge_t<Graph>>)));

    Graph h(n_color, outer_edges.begin(), merge_parallel_edges(outer_edges.begin(), it, weight_add<edge_t<Graph>>));
    return { move(h), move(res) };    
}

template<class Graph>
solution<Graph> default_assemble(const vertex_id_t<Graph>& n,
    const partition_result<Graph>& p,
    const vector<solution<Graph>>& sv) {
    typedef vertex_id_t<Graph> vertex_id;
    const vertex_id k = p.first.num_vertices();
    vertex_id c = 0;
    solution<Graph> sg(n);
    for (vertex_id i = 0; i < k; ++i) {
        const Graph& h = p.second[i].second;
        const vector<vertex_id>& id_map = p.second[i].first;
        const solution<Graph>& sh = sv[i];
        for (vertex_id j = 0; j < h.num_vertices(); ++j)
            sg[id_map[j]] = c + sh[j];
        c += h.num_vertices();
    }
    return sg;
}

template<class Graph>
solution<Graph> default_assemble(const Graph& g,
    const partition_result<Graph>& p,
    const vector<solution<Graph>>& sv) {
    return default_assemble(g.num_vertices(), p, sv);
}

template<class Graph>
solution<Graph> hybrid_assemble(const Graph& g,
    const partition_result<Graph>& p,
    const vector<solution<Graph>>& sv,
    const solver<Graph>& s) {
    typedef vertex_id_t<Graph> vertex_id;
    const vertex_id n = g.num_vertices(), k = p.first.num_vertices();
    solution<Graph> sc = s(p.first);
    vector<vertex_id> offset(k, 0), sg(n);
    for (vertex_id i = 0; i < k; ++i)
        if (sc[i] != k - 1)
            offset[sc[i] + 1] = p.second[i].second.num_vertices();
    for (vertex_id i = 1; i < k; ++i)
        offset[i] += offset[i - 1];

    for (vertex_id i = 0; i < k; ++i) {
        const Graph& h = p.second[i].second;
        const vector<vertex_id>& id_map = p.second[i].first;
        const solution<Graph>& sh = sv[i];
        for (vertex_id j = 0; j < h.num_vertices(); ++j)
            sg[id_map[j]] = offset[sc[i]] + sh[j];
    }
    return sg;
}

template<class Graph>
solution<Graph> dp_assemble(const Graph& g,
    const partition_result<Graph>& p,
    const vector<solution<Graph>>& sv,
    const solver<Graph>& s) {
    typedef vertex_id_t<Graph> vertex_id;
    
    typedef edge_weight_t<Graph> edge_weight;

    const Graph& h = p.first;
    const vertex_id k = h.num_vertices(), n = g.num_vertices();
    solution<Graph> seqh(k), rkh = s(h);
    for (vertex_id i = 0; i < k; ++i)
        seqh[rkh[i]] = i;

    solution<Graph> cur;
    for (vertex_id i = 0; i < k; ++i) {
        vertex_id id = seqh[i];
        const vector<vertex_id> id_map = p.second[id].first;
        const Graph& gi = p.second[id].second;
        const solution<Graph>& sgi = sv[id];
        vertex_id n1 = cur.size(), n2 = sgi.size();
        solution<Graph> tmp(n2);
        for (vertex_id j = 0; j < n2; ++j)
            tmp[sgi[j]] = id_map[j];
        vector<vector<edge_weight>> dp(n1 + 1, vector<edge_weight>(n2 + 1, 0));
        vector<vector<edge_weight>> sum1(n1 + 1, vector<edge_weight>(n2 + 1, 0));
        vector<vector<edge_weight>> sum2(n2 + 1, vector<edge_weight>(n1 + 1, 0));
        
        vector<vertex_id> rk1(n, n), rk2(n, n);
        for (vertex_id i1 = 0; i1 < n1; ++i1)
            rk1[cur[i1]] = i1;
        for (vertex_id i2 = 0; i2 < n2; ++i2)
            rk2[tmp[i2]] = i2;

        for (vertex_id i1 = 0; i1 < n1; ++i1)
            for (const edge_t<Graph>& e : g.out_edges(cur[i1]))
                if (rk2[e.dst] != n)
                    sum1[i1 + 1][rk2[e.dst] + 1] += e.weight;

        for (vertex_id i2 = 0; i2 < n2; ++i2)
            for (const edge_t<Graph>& e : g.out_edges(tmp[i2]))
                if (rk1[e.dst] != n)
                    sum2[i2 + 1][rk1[e.dst] + 1] += e.weight;

        for (vertex_id i1 = 1; i1 <= n1; ++i1)
            for (vertex_id i2 = 1; i2 <= n2; ++i2)
                sum1[i1][i2] += sum1[i1][i2 - 1];
        
        for (vertex_id i2 = 1; i2 <= n2; ++i2)
            for (vertex_id i1 = 1; i1 <= n1; ++i1)
                sum2[i2][i1] += sum2[i2][i1 - 1];
        
        for (vertex_id i1 = 1; i1 <= n1; ++i1)
            for (vertex_id i2 = 1; i2 <= n2; ++i2)
                dp[i1][i2] = min(dp[i1 - 1][i2] + sum1[i1][i2], dp[i1][i2 - 1] + sum2[i2][i1]);

        solution<Graph> n_cur;
        vertex_id i1 = n1, i2 = n2;
        while (i1 != 0 || i2 != 0) {
            if (i2 == 0)
                n_cur.push_back(cur[--i1]);
            else if (i1 == 0)
                n_cur.push_back(tmp[--i2]);
            else {
                if (dp[i1][i2] == dp[i1 - 1][i2] + sum1[i1][i2])
                    n_cur.push_back(cur[--i1]);
                else 
                    n_cur.push_back(tmp[--i2]);
            }
        }
        reverse(n_cur.begin(), n_cur.end());
        cur = move(n_cur);
    }
    solution<Graph> rk(n);
    for (vertex_id i = 0; i < n; ++i)
        rk[cur[i]] = i;
    return rk;
}

template<class Graph, class Tpl, size_t I>
solution<Graph> recurse_impl(const Graph& g,
    const solver<Graph>& s, vertex_id_t<Graph> t,
    Tpl&& tpl, integral_constant<size_t, I>
    ) {
    if (g.num_vertices() <= t)
        return s(g);

    decay_t<std::tuple_element_t<I, decay_t<Tpl>>> d(get<I>(tpl));  //  Use copy construction, no move.
    partition_result<Graph> dres = d.decompose(g);
    vector<solution<Graph>> sres;
    for (const auto& h : dres.second)
        sres.push_back(recurse_impl(h.second, s, t, forward<Tpl>(tpl),
        integral_constant<size_t, (I + 1) % std::tuple_size_v<decay_t<Tpl>>>()));
    return d.assemble(g, dres, sres);
}

template<class Graph, class Tpl>
solution<Graph> recurse(const Graph& g,
    const solver<Graph>& s, vertex_id_t<Graph> t,
    Tpl&& tpl) {
    return recurse_impl(g, s, t, forward<Tpl>(tpl), integral_constant<size_t, 0>());
}

template<class Graph>
struct decomposer_scc {

    partition_result<Graph> decompose(const Graph& g) {
        return partition(g, strongly_connected_components(g));
    }

    solution<Graph> assemble(const Graph& g, const partition_result<Graph>& dres, const vector<solution<Graph>>& sres) {
        return default_assemble(g, dres, sres);
        //return hybrid_assemble(g, dres, sres, solver<graph>(greedy<graph>));
    }

};


}

#endif