#ifndef UTIL_HISTOGRAM_SORT_HPP
#define UTIL_HISTOGRAM_SORT_HPP
#include <algorithm>

namespace util {

/*  TODO:
    Replace new[] and delete[] with allocators.
    Optimize the value domain.
*/

namespace property {

template<class T>
struct reversed {};

}

template<class RndIt, class S, class ...Ts>
class histogram_sorter;

template<class RndIt, class S>
class histogram_sorter<RndIt, S> {
public:
    S n;
    RndIt p;
    S* ptr[2];
    S* cnt;

    histogram_sorter(S n_, S m, RndIt p_) : n(n_), p(p_) {
        ptr[0] = new S[n];
        ptr[1] = new S[n];
        cnt = new S[m];
        std::iota(ptr[0], ptr[0] + n, 0);
    }

    ~histogram_sorter() {
        for (S i = 0; i < n; ++i)
            ptr[1][ptr[0][i]] = i;
        for (S i = 0; i < n; ++i) {
            S j = ptr[0][i], k = ptr[1][i];
            if (j != i) {
                std::swap(p[i], p[j]);
                ptr[0][i] = i;
                ptr[1][i] = i;
                ptr[0][k] = j;
                ptr[1][j] = k;
            }
        }
        delete[] ptr[0];
        delete[] ptr[1];
        delete[] cnt;
    }
};

template<class RndIt, class S, template<class>class P, class K, class ...Ts>
class histogram_sorter<RndIt, S, P<K>, Ts...> : public histogram_sorter<RndIt, S, Ts...> {
    typedef histogram_sorter<RndIt, S, Ts...> base_type;
    S get_max(S n, S m, RndIt p) {
        m_ = 0;
        for (S i = 0; i < n; ++i)
            m_ = std::max(m_, (S)p[i].get(P<void>()));
        m_++;
        return std::max(m, m_);
    }
    S m_;
protected:
    using base_type::ptr;
    using base_type::cnt;
    
public:
    histogram_sorter(S n, S m, RndIt p_) : base_type(n, get_max(n, m, p_), p_) {
        std::fill_n(cnt, m_, 0);
        for (S i = 0; i < n; ++i)
            cnt[p_[i].get(P<void>())]++;
        for (S i = 0; i + 1 < m_; ++i)
            cnt[i + 1] += cnt[i];
        for (S i = 0; i < n; ++i)
            ptr[1][--cnt[p_[ptr[0][n - i - 1]].get(P<void>())]] = ptr[0][n - i - 1];
        std::swap(ptr[0], ptr[1]);
    }
};

template<class RndIt, class S, template<class>class P, class K, class ...Ts>
class histogram_sorter<RndIt, S, property::reversed<P<K>>, Ts...> : public histogram_sorter<RndIt, S, Ts...> {
    typedef histogram_sorter<RndIt, S, Ts...> base_type;
    S get_max(S n, S m, RndIt p) {
        m_ = 0;
        for (S i = 0; i < n; ++i)
            m_ = std::max(m_, (S)p[i].get(P<void>()));
        m_++;
        return std::max(m, m_);
    }
    S m_;
protected:
    using base_type::ptr;
    using base_type::cnt;
    
public:
    histogram_sorter(S n, S m, RndIt p_) : base_type(n, get_max(n, m, p_), p_) {
        std::fill_n(cnt, m_, 0);
        for (S i = 0; i < n; ++i)
            cnt[m_ - p_[i].get(P<void>()) - 1]++;
        for (S i = 0; i + 1 < m_; ++i)
            cnt[i + 1] += cnt[i];
        for (S i = 0; i < n; ++i)
            ptr[1][--cnt[m_ - p_[ptr[0][n - i - 1]].get(P<void>()) - 1]] = ptr[0][n - i - 1];
        std::swap(ptr[0], ptr[1]);
    }
};

template<class RndIt, class ...Ts>
void histogram_sort(RndIt begin, RndIt end, Ts...) {
    if (begin == end) return ;
    histogram_sorter<RndIt, size_t, Ts...>(distance(begin, end), 0, begin);
}

}


#endif