#include <bits/stdc++.h>
#include <graph/static.hpp>
#include <graph/edge_list.hpp>
#include <graph/io.hpp>
#include <FAS/common.hpp>
#include <FAS/greedy.hpp>
#include <FAS/reduction.hpp>
#include <FAS/hybrid.hpp>
#include <FAS/HCS.hpp>
using namespace std;

using namespace graph;
using namespace util;
using namespace FAS;

//  usage: fas_alg InputGraphPath InputGraphType Algname  [-r Seed] [-p] [-w MaxWeight] [-o OutputSolutionPath]
int main(int argc, char** argv) {
    typedef static_graph<int, int, long long> graph;
    typedef vertex_id_t<graph> vertex_id;
    typedef edge_weight_t<graph> edge_weight;
    if (argc < 4) {
        fprintf(stderr, "Not enough arguments!\n Usage: fas_alg InputGraphPath InputGraphType Algname [-r Seed] [-p] [-w MaxWeight] [-o OutputSolutionPath]");
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
    string opath;
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
            else if (argv[i] == "-o"s) {
                ++i;
                if (i >= argc) {
                    fprintf(stderr, "Error: Output path must not be empty!\n");
                    return -1;
                }
                opath = argv[i];
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

    solution<graph> rk;
    chrono::system_clock::time_point t_start = chrono::system_clock::now();
    if (alg_name == "Greedy")
        rk = greedy(g);
    else if (alg_name == "scc+Greedy") {
        coloring_result<graph> color = strongly_connected_components(g);
        partition_result<graph> par = partition(g, color);
        vector<solution<graph>> sv;
        for (const auto& x : par.second)
            sv.emplace_back(greedy(x.second));
        rk = default_assemble(g.num_vertices(), par, sv);
    }
    else if (alg_name == "scc+2cyc+Greedy") {
        fastred<graph, false, false, false> red;
        rk = red.solve(g, greedy<graph>);
    }
    else if (alg_name == "scc+2cyc+chain+Greedy") {
        fastred<graph, false, true, true> red;
        rk = red.solve(g, greedy<graph>);
    }
    else if (alg_name == "RED+Greedy") {
        fastred<graph, false> red;
        rk = red.solve(g, greedy<graph>);
    }
    else if (alg_name == "REDstar+Greedy") {
        fastred<graph, true> red;
        rk = red.solve(g, greedy<graph>);
    }
    else if (alg_name == "HCS+Greedy") {
        HCS<graph> red;
        rk = red.solve(g, greedy<graph>);
    }
    else if (alg_name == "RAS2") {
        rk = hybrid<graph, false, 2, true>(g);
    }
    else if (alg_name == "RAS3") {
        rk = hybrid<graph, false, 3, true>(g);
    }
    else if (alg_name == "RAS4") {
        rk = hybrid<graph, false, 4, true>(g);
    }
    else if (alg_name == "RAS6") {
        rk = hybrid<graph, false, 6, true>(g);
    }
    else if (alg_name == "RAS8") {
        rk = hybrid<graph, false, 8, true>(g);
    }
    else if (alg_name == "RAS12") {
        rk = hybrid<graph, false, 12, true>(g);
    }
    else if (alg_name == "RAS16") {
        rk = hybrid<graph, false, 16, true>(g);
    }
    else if (alg_name == "RAS24") {
        rk = hybrid<graph, false, 24, true>(g);
    }
    else if (alg_name == "RAS32") {
        rk = hybrid<graph, false, 32, true>(g);
    }
    else if (alg_name == "RAS48") {
        rk = hybrid<graph, false, 48, true>(g);
    }
    else if (alg_name == "RASstar") {
        rk = hybrid<graph, true, 3, true>(g);
    }
    else {
        fprintf(stderr, "Invalid algorithm name.\n");
        exit(-1);
    }
    chrono::system_clock::time_point t_finish = chrono::system_clock::now();

    chrono::milliseconds time_elapsed = chrono::duration_cast<chrono::milliseconds>(t_finish-t_start);

    printf("%s,%lld,%lld\n", task_name.c_str(), get_fas_weight(g, rk), time_elapsed.count());

    if (!opath.empty()) {
        ofstream ofs(opath);
        for (vertex_id i = 0; i < g.num_vertices(); ++i)
            ofs << rk[vertex_map[i]] << " \n"[i + 1 == g.num_vertices()];
    }

    return 0;
}