#ifndef GRAPH_SCC_HPP
#define GRAPH_SCC_HPP
#include <vector>

namespace graph {

using std::vector;

template<class Graph>
struct strongly_connected_components_impl {
    const Graph& g;
    typedef vertex_id_t<Graph> vertex_id;
    typedef typename Graph::out_edge_type out_edge_type;

    vertex_id dfc, scc;
    vector<char> vis, inst;
    vector<vertex_id> dfn, low, bel, stk;

    vertex_id dfs1(vertex_id u) {
        stk.push_back(u); inst[u] = 1;
        vis[u] = 1;
        dfn[u] = low[u] = dfc++;
        for (const out_edge_type& e : g.out_edges(u)) {
            vertex_id v = e.dst;
            if (vis[v] == 0)
                low[u] = std::min(low[u], dfs1(v));
            else if (inst[v])
                low[u] = std::min(low[u], dfn[v]);
        }
        if (low[u] == dfn[u]) {
            vertex_id v;
            do {
                v = stk.back();
                stk.pop_back();
                inst[v] = 0;
                bel[v] = scc;
            } while (v != u);
            ++scc;
        }
        vis[u] = 2;
        return low[u];
    }

    vector<pair<vertex_id, typename Graph::out_edge_iterator>> s;
    void dfs2(vertex_id x) {
        s.push_back({ x, g.out_edges(x).begin() });
        stk.push_back(x); inst[x] = 1;
        vis[x] = 1;
        dfn[x] = low[x] = dfc++;
        while (!s.empty()) {
            vertex_id u = s.back().first;
            typename Graph::out_edge_iterator it = s.back().second;
            s.pop_back();
            typename Graph::out_edge_iterator it_end = g.out_edges(u).end();
            if (it != it_end) {
                vertex_id v = it->dst;
                ++it;
                s.push_back({ u, it });
                if (vis[v] == 0) {
                    s.push_back({ v, g.out_edges(v).begin() });
                    stk.push_back(v); inst[v] = 1;
                    vis[v] = 1;
                    dfn[v] = low[v] = dfc++;
                }
                else if (inst[v])
                    low[u] = std::min(low[u], dfn[v]);
            }
            else {
                if (!s.empty())
                    low[s.back().first] = std::min(low[s.back().first], low[u]);
                if (low[u] == dfn[u]) {
                    vertex_id v;
                    do {
                        v = stk.back();
                        stk.pop_back();
                        inst[v] = 0;
                        bel[v] = scc;
                    } while (v != u);
                    ++scc;
                }
            }
        }
    }

    strongly_connected_components_impl(const Graph& g_) : g(g_), dfc(0), scc(0) {
        const vertex_id n = g.num_vertices();
        vis.assign(n, 0);
        dfn.assign(n, 0);
        low.assign(n, 0);
        bel.assign(n, 0);
        inst.assign(n, 0);
        for (vertex_id i = 0; i < n; ++i) {
            if (vis[i] == 0)
                dfs2(i);
        }
        for (vertex_id i = 0; i < n; ++i)
            bel[i] = scc - bel[i] - 1;
    }

};

template<class Graph>
coloring_result<Graph> strongly_connected_components(const Graph& g) {
    strongly_connected_components_impl impl(g);
    return { impl.scc, impl.bel };
}

}

#endif