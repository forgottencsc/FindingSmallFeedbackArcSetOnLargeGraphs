#ifndef FAS_IO_HPP
#define FAS_IO_HPP
#include <algorithm>
#include <iterator>
#include <iostream>
#include <FAS/common.hpp>

namespace FAS {

    template<class Graph>
    solution<Graph> read_solution(std::istream& is) {
        typedef graph::vertex_id_t<Graph> vertex_id;
        solution<Graph> rk;
        std::copy(std::istream_iterator<vertex_id>(is), std::istream_iterator<vertex_id>(), std::back_inserter(rk));
        vector<char> flg(rk.size(), 0);
        for (int i : rk) {
            if (i >= rk.size() || flg[i])
                return solution<Graph>(1, 0);
            else {
                flg[i] = 1;
            }
        }
        return rk;
    }

}

#endif