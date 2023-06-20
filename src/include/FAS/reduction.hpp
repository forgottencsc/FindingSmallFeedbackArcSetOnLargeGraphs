#ifndef FAS_REDUCTION_HPP
#define FAS_REDUCTION_HPP
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

template<class Graph, bool RecSCC = true, bool ArcContraction = true, bool ContractChainOnly = false>
struct fastred {

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
    vector<edge_weight> gs, hs;

    typedef pair<vertex_id, edge_weight> pvw;
    typedef pair<edge_weight, vertex_id> pwv;
    queue<vertex_id> q;
    vector<priority_queue<pwv>> gq, hq;
    vector<pwv> gc, hc;
    const pwv nil = { 0, 0 };

    //  Try to get the only outgoing arc of vertex u
    pwv getg(vertex_id u) {
        return g[u].size() == 1 ? pwv(g[u].begin()->second, g[u].begin()->first) : nil;
    }

    //  Try to get the only incoming arc of vertex u
    pwv geth(vertex_id u) {
        return h[u].size() == 1 ? pwv(h[u].begin()->second, h[u].begin()->first) : nil;
    }

    void enqueue(vertex_id u) {
        if (stat[u] != normal) return;
        q.push(u);
        stat[u] = inqueue;
    }

    //  Check if it's possible contract(u, v) where (u, v) is the only outgoing arc of vertex u
    bool checkg(vertex_id u) {
        pwv p = getg(u);
        return p != nil && p.first >= min(hs[u], gs[p.second]);
    }

    //  Check if it's possible contract(v, u) where (v, u) is the only incoming arc of vertex u
    bool checkh(vertex_id u) {
        pwv p = geth(u);
        return p != nil && p.first >= min(gs[u], hs[p.second]);
    }

    //  Update vertex u
    void checkc(vertex_id u) {
        pwv ngc = getg(u), nhc = geth(u);
        pwv ogc = exchange(gc[u], ngc), ohc = exchange(hc[u], nhc);
        if (ngc != nil && ogc != ngc)
            gq[ngc.second].push(pwv(ngc.first, u));
        if (nhc != nil && ohc != nhc)
            hq[nhc.second].push(pwv(nhc.first, u));
    }

    //  Find new contractible arcs
    void checkq(vertex_id u) {
        if (checkg(u) || checkh(u))
            enqueue(u);
        while (!gq[u].empty()) {
            pwv p = gq[u].top();
            vertex_id v = p.second;
            edge_weight w = p.first;
            if (stat[v] == normal && getg(v) == gc[v]) {
                if (w < gs[u])
                    break;
                else
                    enqueue(v);
            }
            gq[u].pop();
            gc[v] = nil;
        }
        while (!hq[u].empty()) {
            pwv p = hq[u].top();
            vertex_id v = p.second;
            edge_weight w = p.first;
            if (stat[v] == normal && geth(v) == hc[v]) {
                if (w < hs[u])
                    break;
                else
                    enqueue(v);
            }
            hq[u].pop();
            hc[v] = nil;
        }
    }

    void del_edge(vertex_id u, vertex_id v) {
        auto it = g[u].find(v);
        gs[u] -= it->second;
        hs[v] -= it->second;
        g[u].erase(v);
        h[v].erase(u);
    }

    void add_edge(vertex_id u, vertex_id v, edge_weight w) {
        if (u == v) return;
        auto it = g[v].find(u);         //  Find reverse arc
        if (it == g[v].end()) {         //  Reverse arc not exist
            auto res = g[u].insert({ v, w });
            gs[u] += w;
            hs[v] += w;
            if (res.second)                 //  Parallel arc not exist
                h[v].insert({ u, w });
            else {                          //  Parallel arc exists
                res.first->second += w;
                h[v][u] += w;
            }
        }
        else {                              //  Reverse arc exists
            if (it->second > w) {           //  Decrease its weight
                it->second -= w;     
                h[u][v] -= w;
                gs[v] -= w;
                hs[u] -= w;
            }
            else {                          //  Remove reverse arc 
                w -= it->second;
                gs[v] -= it->second;
                hs[u] -= it->second;
                g[v].erase(u);
                h[u].erase(v);
                if (w != 0) {
                    g[u].insert({ v, w });
                    h[v].insert({ u, w });
                    gs[u] += w;
                    hs[v] += w;
                }
            }
        }
    }

    void contract(vertex_id u, vertex_id v) {
        del_edge(u, v);
        //  Merge vertex with smaller degree into vertex with larger degree
        if (g[u].size() + h[u].size() > g[v].size() + h[v].size()) {
            l[u].splice(l[u].end(), l[v]);
            std::swap(u, v);
        }
        else 
            l[v].splice(l[v].begin(), l[u]);
        
        vector<pvw> gu(g[u].begin(), g[u].end()), hu(h[u].begin(), h[u].end());
        for (const pvw& e : gu) {
            del_edge(u, e.first);
            add_edge(v, e.first, e.second);
        }
        for (const pvw& e : hu) {
            del_edge(e.first, u);
            add_edge(e.first, v, e.second);
        }
        for (const pvw& e : gu) checkc(e.first);
        for (const pvw& e : hu) checkc(e.first);
        checkc(v);
        for (const pvw& e : gu) checkq(e.first);
        for (const pvw& e : hu) checkq(e.first);
        checkq(v);
        g[u].clear();
        h[u].clear();
        gq[u] = priority_queue<pwv>();
        hq[u] = priority_queue<pwv>();
        stat[u] = deleted;
    }

    void reduction(pii& x) {
        for (auto it = x.first; it != x.second; ++it)
            checkc(*it);
        for (auto it = x.first; it != x.second; ++it)
            checkq(*it);
        while (!q.empty()) {
            vertex_id u = q.front(); q.pop();
            if (stat[u] == deleted) continue;
            stat[u] = normal;
            if (checkh(u))
                contract(geth(u).second, u);
            else if (checkg(u))
                contract(u, getg(u).second);
            else
                checkc(u);
        }
        x.second = remove_if(x.first, x.second, [&](vertex_id v) { return stat[v] == deleted; });
    }

    void contract_chain(pii& x) {
        for (auto it = x.first; it != x.second; ++it) {
            vertex_id u = *it;
            pwv pg = getg(u), ph = geth(u);
            if (pg != nil && ph != nil) {
                vertex_id vl = ph.second, vr = pg.second;
                del_edge(vl, u);
                del_edge(u, vr);
                add_edge(vl, vr, min(ph.first, pg.first));
                if (ph.first < pg.first)
                    l[vr].splice(l[vr].begin(), l[u]);
                else
                    l[vl].splice(l[vl].end(), l[u]);
                stat[u] = deleted;
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
        gs.resize(n, 0);
        hs.resize(n, 0);
        gq.resize(n);
        hq.resize(n);
        gc.resize(n, nil);
        hc.resize(n, nil);
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
        solution<Graph> res(n, n);
        auto it = k.begin();
        while (it != k.end()) {
            pii x = *it;
            if constexpr (ArcContraction) {
                if constexpr (ContractChainOnly)
                    contract_chain(x);
                else
                    reduction(x);
            }
            scc_decomp(x);
            vector<pii> scc_res = partition(x);
            auto it2 = next(it);
            if (RecSCC && color_num > 1) {
                for (const pii& y : scc_res)
                    k.emplace(it2, y);
            }
            else {
                for (pii& y : scc_res) {
                    vertex_id n2 = y.second - y.first;
                    edge_id m2 = 0;
                    for (auto it3 = y.first; it3 != y.second; ++it3) {
                        vertex_id u = *it3;
                        m2 += g[u].size();
                        id[u] = it3 - y.first;
                    }
                    vector<edge_t<Graph>> es; es.reserve(m2);
                    for (auto it3 = y.first; it3 != y.second; ++it3) {
                        vertex_id u = *it3;
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
                        for (vertex_id v : l[y.first[u]])
                            res[v] = p++;
                }
            }
            it = k.erase(it);
        }
        return res;
    }
};

}

#endif