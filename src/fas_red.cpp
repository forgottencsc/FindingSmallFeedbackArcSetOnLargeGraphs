#include <bits/stdc++.h>
#include <graph/static.hpp>
#include <graph/edge_list.hpp>
#include <graph/io.hpp>
#include <FAS/common.hpp>
#include <FAS/greedy.hpp>
#include <FAS/reduction.hpp>
#include <FAS/HCS.hpp>
using namespace std;

using namespace graph;
using namespace util;
using namespace FAS;

template<class Graph>
solution<Graph> count_edges(const Graph& g, edge_id_t<Graph>& m, edge_weight_t<Graph>& w) {
    m += g.num_edges();
    w += get_edge_weight_sum(g);
    return trivial_solver(g);
}

template<class Graph, class Solver>
solution<Graph> solve_after_2cyc_SCC(const Graph& g, Solver s) {
    vector<edge_t<Graph>> es;
    es.reserve(g.num_edges());
    for (const edge_t<Graph>& e : g.edges()) {
        if ((e.src) < (e.dst))
            es.push_back(e);
        else if ((e.src) > (e.dst))
            es.push_back(edge_t<Graph>{ e.dst, e.src, -e.weight });
    }

    es.erase(merge_parallel_edges(es.begin(), es.end(), weight_add<edge_t<Graph>>), es.end());
    es.erase(remove_if(es.begin(), es.end(), [](const edge_t<Graph>& e) { return e.weight == 0; }), es.end());
    for (edge_t<Graph>& e : es)
        if ((e.weight) < 0) {
            swap(e.src, e.dst);
            e.weight = -e.weight;
        }

    Graph h(g.num_vertices(), es.begin(), es.end());
    coloring_result<Graph> color = strongly_connected_components(h);
    es.clear();
    for (auto e : g.edges())
        if (color.second[e.src] == color.second[e.dst])
            es.push_back(e);
    return s(Graph(g.num_vertices(), es.begin(), es.end()));
}

//  usage: fas_red InputGraphPath InputGraphType Algname [-r Seed] [-p] [-w MaxWeight]
int main(int argc, char** argv) {
    typedef static_graph<int, int, long long> graph;
    typedef vertex_id_t<graph> vertex_id;
    typedef edge_weight_t<graph> edge_weight;
    if (argc < 4) {
        fprintf(stderr, "Not enough arguments!\n Usage: fas_red InputGraphPath InputGraphType Algname [-r Seed] [-p] [-w MaxWeight]");
        exit(-1);
    }
    string alg_name = argv[3], graph_path = argv[1], graph_type = argv[2];
    
    size_t pos_sec = graph_path.find_last_of("/\\");

    string graph_name;
    if (pos_sec != string::npos)
        graph_name = graph_path.substr(pos_sec + 1);
    else 
        graph_name = graph_path;

    int maxw = 1;
    long long seed = 0;
    bool seed_provided = false;
    bool do_shuffle = false;
    {
        int i = 4;
        while (i < argc) {
            if (argv[i] == "-r"s) {
                ++i;
                if (i >= argc) {
                    fprintf(stderr, "Error: Random seed must not be empty!\n");
                    return -1;
                }
                seed = atoll(argv[i]);
                ++i;
                seed_provided = true;
            }
            else if (argv[i] == "-p"s) {
                do_shuffle = true;
                ++i;
            }
            else if (argv[i] == "-w"s) {
                ++i;
                if (i >= argc) {
                    fprintf(stderr, "Error: Maximum weight must not be empty!\n");
                    return -1;
                }
                maxw = atoi(argv[i]);
                ++i;
            }
            else {
                fprintf(stderr, "Error: Unrecognized option %s\n", argv[i]);
                return -1;
            }
        }
    }

    vector<int> vertex_map;

    graph g;
    {
        ifstream ifs(graph_path);
        auto res = read_graph_impl<graph>(ifs, graph_type);
        vertex_map.assign(res.first, 0);
        iota(vertex_map.begin(), vertex_map.end(), 0);

        
        if (!seed_provided && (do_shuffle || maxw > 1)) {
            fprintf(stderr, "Warning: You should provide a random seed with parameter -r when using parameter -p or -w! Use default seed %lld instead.\n", seed);
        }

        mt19937_64 mt(seed);

        if (maxw > 1) {
            uniform_int_distribution<edge_weight> uid(1, maxw);
            for (graph::edge_type& e : res.second)
                e.weight = uid(mt);
        }
        
        if (do_shuffle) {
            shuffle(res.second.begin(), res.second.end(), mt);
            shuffle(vertex_map.begin(), vertex_map.end(), mt);
            for (graph::edge_type& e : res.second) {
                e.src = vertex_map[e.src];
                e.dst = vertex_map[e.dst];
            }
        }

        g = graph(res.first, res.second.begin(), res.second.end());
    }
    string task_name = graph_name + "_" + alg_name + "_" + to_string(maxw) + "_" + to_string(seed);
    // fprintf(stderr, "Task name: %s\n", task_name.c_str());
    // fprintf(stderr, "Read complete! %d vertices and %d edges\n", g.num_vertices(), g.num_edges());

    edge_id_t<graph> res_m = 0;
    edge_weight_t<graph> res_w = 0;
    chrono::system_clock::time_point t_start = chrono::system_clock::now();
    if (alg_name == "HCS") {
        HCS<graph> red;
        red.solve(g, bind(count_edges<graph>, _1, ref(res_m), ref(res_w)));
    }
    else if (alg_name == "loops") {
        res_w = get_edge_weight_sum(g);
        res_m = g.num_edges();
        for (const edge_t<graph>& e : g.edges())
            if (e.src == e.dst) {
                res_m--;
                res_w -= e.weight;
            }
        
    }
    else if (alg_name == "scc") {
        auto color = strongly_connected_components(g);
        for (auto e : g.edges())
            if (color.second[e.src] == color.second[e.dst]) {
                res_m++;
                res_w += e.weight;
            }
    }
    else if (alg_name == "scc+2cyc") {
        solve_after_2cyc_SCC(g, bind(count_edges<graph>, _1, ref(res_m), ref(res_w)));
    }
    else if (alg_name == "scc+2cyc+chain") {
        fastred<graph, false, true, true> red;
        red.solve(g, bind(count_edges<graph>, _1, ref(res_m), ref(res_w)));
    }
    else if (alg_name == "RED") {
        fastred<graph, false, true, false> red;
        red.solve(g, bind(count_edges<graph>, _1, ref(res_m), ref(res_w)));
    }
    else if (alg_name == "REDstar") {
        fastred<graph, true, true, false> red;
        red.solve(g, bind(count_edges<graph>, _1, ref(res_m), ref(res_w)));
    }
    else {
        fprintf(stderr, "Invalid algorithm name.\n");
        exit(-1);
    }
    chrono::system_clock::time_point t_finish = chrono::system_clock::now();

    chrono::milliseconds time_elapsed = chrono::duration_cast<chrono::milliseconds>(t_finish-t_start);

    printf("%s,%d,%lld,%lld\n", task_name.c_str(), res_m, res_w, time_elapsed.count());

    return 0;
}