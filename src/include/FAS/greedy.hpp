#ifndef FAS_SOLVER_GREEDY_HPP
#define FAS_SOLVER_GREEDY_HPP
#include <vector>
#include <util/histogram_heap.hpp>
#include <FAS/common.hpp>
#include <FAS/recursive.hpp>
#include <GB/common.hpp>

namespace FAS {

//  Greedy: O(n+m)
//  P. Eades, X. Lin, W. F. Smyth. (1993).
//  A fast and effective heuristic for the feedback arc set problem.
//  Information Processing Letters, 47(6), 319-323.

//  GreedyAbs: O(n+m)
//  T. Coleman, A. Wirth. (2010). 
//  Ranking tournaments: Local search and a new algorithm. 
//  Journal of Experimental Algorithmics, 14, 2-6.
template<class Graph, bool useAbs = false>
class greedy_impl {
public:
    typedef vertex_id_t<Graph> vertex_id;
    typedef edge_weight_t<Graph> edge_weight_type;
    typedef edge_t<Graph> edge_type;

    typedef util::static_histogram_max_heap<edge_weight_type, vertex_id> heap;

    const Graph& g;
    vector<char> del;
    vector<edge_weight_type> iws, ows, vio, voi;
    vector<vertex_id> source, sink;
    edge_weight_type mw;
    heap hio, hoi;

    int lp, rp;
    vector<vertex_id> rk;

    void remove_vertex(vertex_id u) {
        del[u] = 1;
        for (const edge_type& e : g.in_edges(u)) {
            int v = e.src;
            if (del[v]) continue;
            ows[v] -= e.weight;
            if (ows[v] == 0)
                sink.push_back(v);
            if constexpr (useAbs) {
                vio[v] = mw + iws[v] - ows[v];
                hio.modify_key(v, vio[v]);
            }
            voi[v] = mw + ows[v] - iws[v];
            hoi.modify_key(v, voi[v]);
        }
        for (const edge_type& e : g.out_edges(u)) {
            int v = e.dst;
            if (del[v]) continue;
            iws[v] -= e.weight;
            if (iws[v] == 0)
                source.push_back(v);
            if constexpr (useAbs) {
                vio[v] = mw + iws[v] - ows[v];
                hio.modify_key(v, vio[v]);
            }
            voi[v] = mw + ows[v] - iws[v];
            hoi.modify_key(v, voi[v]);
        }
    }

    greedy_impl(const Graph& g_) : g(g_) {
        const vertex_id n = g.num_vertices();
        if (n == 1) {
            rk.assign(1, 0);
            return;
        }
        del.assign(n, 0);
        iws.assign(n, 0);
        ows.assign(n, 0);
        if constexpr (useAbs) vio.assign(n, 0);
        voi.assign(n, 0);
        for (const edge_type& e : g.edges()) {
            iws[e.dst] += e.weight;
            ows[e.src] += e.weight;
        }

        mw = 0;
        for (vertex_id i = 0; i < n; ++i)
            mw = max({ mw, iws[i], ows[i] });
        for (vertex_id i = 0; i < n; ++i) {
            if constexpr (useAbs)
                vio[i] =  mw + iws[i] - ows[i];
            voi[i] = mw + ows[i] - iws[i];
        }
        if constexpr (useAbs)
            hio = heap(2 * mw + 1, vio.begin(), vio.end());
        hoi = heap(2 * mw + 1, voi.begin(), voi.end());

        for (vertex_id i = 0; i < n; ++i) {
            if (ows[i] == 0)
                sink.push_back(i);
            if (iws[i] == 0)
                source.push_back(i);
        }

        rk.assign(n, 0);
        lp = 0;
        rp = n;
        while (lp != rp) {
            while (!sink.empty()) {
                int u = sink.back();
                sink.pop_back();
                if (del[u]) continue;
                remove_vertex(u);
                rk[u] = --rp;
            }
            while (!source.empty()) {
                int u = source.back();
                source.pop_back();
                if (del[u]) continue;
                remove_vertex(u);
                rk[u] = lp++;
            }
            if (lp == rp) break;
            int ul, ur;
            if constexpr (useAbs) while (del[ur = hio.top()]) hio.pop();
            while (del[ul = hoi.top()]) hoi.pop();
            if (!useAbs || voi[ul] > vio[ur]) {
                rk[ul] = lp++;
                remove_vertex(ul);
            }
            else {
                rk[ur] = --rp;
                remove_vertex(ur);
            }
        }
    }

};

template<class Graph>
solution<Graph> greedy(const Graph& g) {
    greedy_impl<Graph,true> impl(g);
    // vector<vertex_id_t<Graph>> rk = trivial_solver(g);
    return impl.rk;
    // return get_fas_weight(g, rk) < get_fas_weight(g, impl.rk) ? rk : impl.rk;
}


template<class Graph>
struct decomposer_greedy {

    double alpha;
    decomposer_greedy(double alpha_ = (2. / 3)) : alpha(alpha_) {}

    partition_result<Graph> decompose(const Graph& g) {
        typedef vertex_id_t<Graph> vertex_id;
        const vertex_id n = g.num_vertices();
        greedy_impl<Graph, true> impl(g);
        vector<vertex_id> color(n); 
        vertex_id t = min(max(impl.lp, (vertex_id)ceil((1 - alpha) * n)),  (vertex_id)floor(alpha * n));
        for (vertex_id i = 0; i < n; ++i)
            color[i] = impl.rk[i] >= t;
        return partition(g, coloring_result<Graph>(2, color));
    }

    solution<Graph> assemble(const Graph& g, const partition_result<Graph>& dres, const vector<solution<Graph>>& sres) {
        return hybrid_assemble(g, dres, sres, solver<Graph>(trivial_solver<Graph>));
    }

};

template<class Graph>
solver<Graph> solver_greedy() { return greedy<Graph>; }

template<class Graph>
solver<Graph> solver_greedyDC() {
    return [=](const Graph& g) {
        return recurse<Graph>(g,
            trivial_solver<Graph>, 1,
            std::make_tuple(decomposer_scc<Graph>(), decomposer_greedy<Graph>()));
    };
}

}   //  namespace FAS

#endif