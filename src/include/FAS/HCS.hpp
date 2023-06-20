#ifndef FAS_HCS_HPP
#define FAS_HCS_HPP
#include <list>
#include <queue>
#include <ext/pb_ds/assoc_container.hpp>
#include <graph/scc.hpp>
#include <graph/algorithms.hpp>
#include <FAS/common.hpp>

namespace FAS {
    
using std::exchange;
using std::queue;
using std::list;
using std::priority_queue;

template<class Graph>
struct HCS {

    typedef vertex_id_t<Graph> vertex_id;
    typedef edge_id_t<Graph> edge_id;
    typedef edge_weight_t<Graph> edge_weight;
    typedef edge_t<Graph> edge;

    typedef __gnu_pbds::gp_hash_table<vertex_id, edge_weight> edge_container;

    enum stat_t { normal = 0, inqueue = 1, deleted = 2 };

    vertex_id n;
    vector<char> stat;

    vector<vertex_id> s, id;
    typedef pair<typename vector<vertex_id>::iterator, typename vector<vertex_id>::iterator> pii;
    list<pii> k;
    vector<list<vertex_id>> l;

    vector<edge_container> g, h;

    typedef pair<vertex_id, edge_weight> pvw;

    void del_edge(vertex_id u, vertex_id v) {
        g[u].erase(v);
        h[v].erase(u);
    }

    void add_edge(vertex_id u, vertex_id v, edge_weight w) {
        if (u == v) return;
        auto res = g[u].insert({ v, w });
        if (res.second)                 //  Parallel arc not exist
            h[v].insert({ u, w });
        else {                          //  Parallel arc exists
            res.first->second += w;
            h[v][u] += w;
        }
    }

    void reduction(pii& x) {
        bool f = true;
        while (f) {
            f = false;
            for (auto it = x.first; it != x.second; ++it) {
                vertex_id u = *it;
                if (g[u].size() == 1 && h[u].size() == 1) {
                    vertex_id pred = h[u].begin()->first;
                    vertex_id succ = g[u].begin()->first;
                    edge_weight w = min(h[u].begin()->second, g[u].begin()->second);
                    if (pred == u && succ == u)
                        continue;
                    if (pred == succ)
                        continue;
                    f = true;   //  reduction made
                    stat[u] = deleted;
                    if (h[u].begin()->second < g[u].begin()->second)
                        l[succ].splice(l[succ].begin(), l[u]);
                    else 
                        l[pred].splice(l[pred].end(), l[u]);
                    del_edge(pred, u);
                    del_edge(u, succ);
                    add_edge(pred, succ, w);
                }
                else if (g[u].size() == 1) {
                    vertex_id v = g[u].begin()->first;
                    edge_weight w2 = g[u].begin()->second;
                    auto it = g[v].find(u);
                    if (it != g[v].end()) {
                        edge_weight w1 = it->second;
                        if (w2 <= w1) {
                            f = true;   //  reduction made
                            del_edge(u, v);
                        }
                    }
                }
                else if (h[u].size() == 1) {
                    vertex_id v = h[u].begin()->first;
                    edge_weight w1 = h[u].begin()->second;
                    auto it = h[v].find(u);
                    if (it != h[v].end()) {
                        edge_weight w2 = it->second;
                        if (w1 <= w2) {
                            f = true;   //  reduction made
                            del_edge(v, u);
                        }
                    }
                }
            }
        }
        x.second = remove_if(x.first, x.second, [&](vertex_id v) { return stat[v] == deleted; });
    }


    vertex_id color_num, dfc;
    vector<vertex_id> dfn, low, color, stk, inst;
    vector<pair<vertex_id, typename edge_container::const_iterator>> ctx;
    void scc_dfs(vertex_id x) {
        ctx.emplace_back(x, g[x].begin());
        stk.push_back(x); inst[x] = 1;
        dfn[x] = low[x] = dfc++;
        while (!ctx.empty()) {
            vertex_id u = ctx.back().first;
            typename edge_container::const_iterator it = ctx.back().second, it_end = g[u].end();
            ctx.pop_back();
            if (it != it_end) {
                vertex_id v = it->first;
                ++it;
                ctx.emplace_back(u, it);
                if (dfn[v] == n) {
                    ctx.emplace_back(v, g[v].begin());
                    stk.push_back(v); inst[v] = 1;
                    // vis[v] = 1;
                    dfn[v] = low[v] = dfc++;
                }
                else if (inst[v])
                    low[u] = std::min(low[u], dfn[v]);
            }
            else {
                if (!ctx.empty())
                    low[ctx.back().first] = std::min(low[ctx.back().first], low[u]);
                if (low[u] == dfn[u]) {
                    vertex_id v;
                    do {
                        v = stk.back();
                        stk.pop_back();
                        inst[v] = 0;
                        color[v] = color_num;
                    } while (v != u);
                    ++color_num;
                }
            }
        }
    }

    void scc_decomp(const pii& x) {
        color_num = dfc = 0;
        for (auto it = x.first; it != x.second; ++it)
            dfn[*it] = n;
        for (auto it = x.first; it != x.second; ++it)
            if (dfn[*it] == n)
                scc_dfs(*it);
        for (auto it = x.first; it != x.second; ++it)
            color[*it] = color_num - color[*it] - 1;
    }

    vector<pair<vertex_id, vertex_id>> par_es;
    vector<pii> partition(const pii& x) {
        // vertex_id n2 = x.second - x.first;
        sort(x.first, x.second, [&](vertex_id u, vertex_id v) { return color[u] < color[v]; });
        vector<pii> res;
        res.reserve(color_num);
        
        for (auto it = x.first; it != x.second; ++it) {
            vertex_id u = *it;
            if (res.empty())
                res.emplace_back(it, it);
            else if (color[it[-1]] != color[u]) {
                res.back().second = it;
                res.emplace_back(it, it);
            }
            for (auto e : g[u]) 
                if (color[u] != color[e.first])
                    par_es.emplace_back(u, e.first);
        }
        res.back().second = x.second;
        for (auto e : par_es)
            del_edge(e.first, e.second);
        par_es.clear();
        return res;
    }

    solution<Graph> solve(const Graph& g0, solver<Graph> sol) {
        n = g0.num_vertices();
        id.resize(n, n);
        g.resize(n);
        h.resize(n);
        stat.resize(n, normal);

        dfn.resize(n, n);
        low.resize(n, n);
        inst.resize(n, 0);
        color.resize(n, n);

        s.assign(n, 0);
        iota(s.begin(), s.end(), 0);
        k.emplace_back(s.begin(), s.end());

        l.resize(n);
        for (vertex_id u = 0; u < n; ++u)
            l[u].push_back(u);

        for (const auto& e : g0.edges())
            add_edge(e.src, e.dst, e.weight);
        
        vertex_id p = 0;
        solution<Graph> res(n);
        auto it = k.begin();
        while (it != k.end()) {         
            if (it->second == it->first + 1) {
                for (vertex_id v : l[it->first[0]])
                    res[v] = p++;
                ++it;
            }
            else {
                pii x = *it;
                reduction(x);
                scc_decomp(x);
                vector<pii> scc_res = partition(x);
                if (color_num > 1) {
                    auto it2 = next(it);
                    for (const pii& y : scc_res)
                        k.emplace(it2, y);
                    it = k.erase(it);
                }
                else {
                    vertex_id n2 = x.second - x.first;
                    edge_id m2 = 0;
                    for (auto it = x.first; it != x.second; ++it) {
                        vertex_id u = *it;
                        m2 += g[u].size();
                        id[u] = it - x.first;
                    }
                    vector<edge_t<Graph>> es; es.reserve(m2);
                    for (auto it = x.first; it != x.second; ++it) {
                        vertex_id u = *it;
                        for (auto e0 : g[u]) {
                            vertex_id v = e0.first;
                            edge e;
                            e.src = id[u];
                            e.dst = id[v];
                            e.weight = e0.second;
                            es.push_back(e);
                        }
                    }
                    solution<Graph> pos = inv<Graph>(sol(Graph(n2, es.begin(), es.end())));
                    for (vertex_id u : pos)
                        for (vertex_id v : l[x.first[u]])
                            res[v] = p++;
                    ++it;
                }
            }
        }
        
        return res;
    }
};

}

#endif