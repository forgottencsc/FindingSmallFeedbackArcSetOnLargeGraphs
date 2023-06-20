#ifndef UTIL_RANDOM_HPP
#define UTIL_RANDOM_HPP
#include <random>
#include <type_traits>

namespace util {

std::mt19937_64 rng;

template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
T rnd(T l, T r) {
    return std::uniform_int_distribution<T>(l, r)(rng);
}

template<class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
T rnd(T l, T r) {
    return std::uniform_real_distribution<T>(l, r)(rng);
}

template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::vector<T> rnd_perm(T n) {
    std::vector<T> a(n);
    std::iota(a.begin(), a.end(), 0);
    for (T i = 0; i < n; ++i)
        std::swap(a[i], a[rnd(i, n - 1)]);
    return a;
}

}

#endif