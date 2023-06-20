#ifndef UTIL_HISTOGRAM_HEAP_HPP
#define UTIL_HISTOGRAM_HEAP_HPP
#include <vector>
#include <list>
#include <type_traits>

/*  TODO:
    Replace vectors with raw pointers
*/

namespace util {

using std::size_t;
using std::vector;
using std::list;
using std::forward;
using std::move;
using std::swap;
using std::min;
using std::max;
using std::is_integral_v;

//  Max-heap containing n elements with key range [0, m]
template<class K, class I = size_t>
class static_histogram_max_heap : public vector<K> {
public:

    typedef K key_type;
    typedef I size_type;
    typedef vector<key_type> base_type;
    typedef static_histogram_max_heap<K, I> this_type;

    static_assert(is_integral_v<K>, "Key type must be integral.");
    static_assert(is_integral_v<I>, "Index type must be integral.");

    key_type m;             //  All key value lies in range [0, m)
    vector<size_type> a;   //  a[i] = i-th element sorted by key value.
    vector<size_type> c;   //  c[i] = number of elements with key < i.
    vector<size_type> p;   //  p[i] = position of element i in sequence.

    static_histogram_max_heap() : m(0) {}
    static_histogram_max_heap(const this_type&) = default;
    static_histogram_max_heap(this_type&&) = default;

    this_type& operator=(const this_type&) = default;
    this_type& operator=(this_type&&) = default;

    ~static_histogram_max_heap() = default;


    template<class ...Args>
    static_histogram_max_heap(key_type m_, Args&& ...args) :
        base_type(forward<Args>(args)...), 
        m(m_), a(base_type::size()), c(size_t(m)+1), p(size()) {
        for (const key_type& k : *this) {
            // assert(0 <= k);
            // assert(k <= m);
            c[k]++;
        }
        for (key_type i = 1; i <= m; ++i)
            c[i] += c[i - 1];
        for (size_type i = 0; i < size(); ++i) {
            p[i] = --c[key(i)];
            a[p[i]] = i;
        }
    }

    template<class ...Args>
    void assign(key_type m_, Args&& ...args) {
        base_type::assign(forward<Args>(args)...);
        m = m_;
        a.resize(base_type::size());
        c.resize(size_t(m)+1);
        p.resize(size());
        fill(c.begin(), c.end(), 0);
        for (const key_type& k : *this)
            c[k]++;
        for (key_type i = 1; i <= m; ++i)
            c[i] += c[i - 1];
        for (size_type i = 0; i < size(); ++i) {
            p[i] = --c[key(i)];
            a[p[i]] = i;
        }
    }



    size_type size() const {
        return a.size();
    }

    size_type top() {
        return a.back();
    }

    void pop() {
        a.pop_back();
    }

    key_type key(size_type i) const {
        return this->at(i);
    }

    //  Increase key value of element i by 1
    void increase_key(size_type i) {
        // assert(i < p.size());     //  Ensure index in range [0, n)
        // assert(key(i) + 1 <= m);  //  Ensure key in range [0, m)

        //  Swap element i to the last element with key at(i)
        size_type j = a[key(i) == m ? size() - 1 : min(size(), c[key(i) + 1]) - 1];
        if (i != j) {
            swap(p[i], p[j]);
            a[p[i]] = i;
            a[p[j]] = j;
        }

        this->at(i)++;
        c[key(i)] = p[i];
    }

    //  Decrease key value of element i by 1
    void decrease_key(size_type i) {
        // assert(i < p.size());     //  Ensure index in range [0, n)
        // assert(key(i) > 0);  //  Ensure key in range [0, m)
        
        //  Swap element i to the first element with key k[i]
        size_type j = a[c[key(i)]];
        if (i != j) {
            swap(p[i], p[j]);
            a[p[i]] = i;
            a[p[j]] = j;
        }

        c[key(i)]++;
        this->at(i)--;
    }

    //  Modify the key value of element i to k
    void modify_key(size_t i, key_type k) {
        // assert(0 <= k && k <= m);
        while (key(i) < k) increase_key(i);
        while (key(i) > k) decrease_key(i);
    }
};

//  Min-heap containing n elements with key range [0, m)
template<class K, class I = size_t> 
class static_histogram_min_heap {
public:
    typedef static_histogram_max_heap<K, I> base_type;
    typedef static_histogram_min_heap<K, I> this_type;
    typedef typename base_type::key_type key_type;
    typedef typename base_type::size_type size_type;

private:
    static_histogram_max_heap<K, I> h;

    template<class ...Args>
    static vector<key_type> translate(key_type m, Args&& ...args) {
        vector<key_type> v(forward<Args>(args)...);
        for (key_type& k : v) k = m - k;
        return v;
    }
public:

    static_histogram_min_heap() {}
    static_histogram_min_heap(const this_type&) = default;
    static_histogram_min_heap(this_type&&) = default;

    this_type& operator=(const this_type&) = default;
    this_type& operator=(this_type&&) = default;

    ~static_histogram_min_heap() = default;


    template<class ...Args>
    static_histogram_min_heap(key_type m_, Args&& ...args)
        : h(translate(m_, forward<Args>(args)...)) {}

    key_type key(size_type i) { return h.m - h.key(i); }

    size_type size() const { return h.size(); }

    size_type top() { return h.top(); }

    void pop() { h.pop(); }
    
    void increase_key(size_type i) {
        // assert(key(i) + 1 <= this->m);
        h.decrease_key(i);
    }

    void decrease_key(size_type i) {
        // assert(key(i) > 0);
        h.increase_key(i);
    }

    void modify_key(size_type i, key_type k) {
        h.modify_key(i, this->m - k);
    }

};

template<class K, class I = size_t>
class dynamic_histogram_heap {
    typedef dynamic_histogram_heap<K, I> this_type;
public:
    typedef K key_type;
    typedef I size_type;
    typedef typename list<size_type>::iterator iterator;
    typedef typename list<size_type>::const_iterator const_iterator;

    size_type n;
    key_type ml, mr, maxv, minv;
    vector<K> v;
    vector<list<size_type>> c;
    vector<iterator> p;

    void rebuild_p() {
        for (key_type i = ml; i < mr; ++i)
            for (auto it = c[i - ml].begin(); it != c[i - ml].end(); ++it)
                p[*it] = it;
    }

    key_type key(size_type i) const { return v.at(i); }

    size_type size() const { return n; }
    
    bool empty() const { return n == 0; }

    void insert(size_type i, key_type k) {
        n++;
        v.at(i) = k;
        p[i] = c[k - ml].insert(c[k - ml].end(), i);
        maxv = max(maxv, k);
        minv = min(minv, k);
    }

    void remove(size_type i) {
        n--;
        c[key(i) - ml].erase(p[i]);
        p[i] = c[0].end();
    }

    void increase_key(size_type i, key_type k) {
        key_type old_k = key(i);
        remove(i);
        insert(i, old_k + k);
    }

    void decrease_key(size_type i, key_type k) {
        key_type old_k = key(i);
        remove(i);
        insert(i, old_k - k);
    }

    void modify_key(size_type i, key_type k) {
        key_type old_k = key(i);
        if (old_k < k)
            increase_key(i, k - old_k);
        else if (old_k > k)
            decrease_key(i, old_k - k);
    }

    size_type find_min() {
        while (minv < mr && c[minv - ml].empty()) ++minv;
        // assert(minv < mr);
        return c[minv - ml].front();
    }

    size_type find_max() {
        while (maxv >= ml && c[maxv - ml].empty()) --maxv;
        // assert(maxv >= ml);
        return c[maxv - ml].front();
    }

    template<class ...Args>
    dynamic_histogram_heap(K m, Args&& ...args)
        : this_type(std::make_pair<K, K>(0, m), forward<Args>(args)...) {
    }

    template<class ...Args>
    dynamic_histogram_heap(std::pair<K, K> m, Args&& ...args)
        : v(forward<Args>(args)...), ml(m.first), mr(m.second), c(mr - ml), n(0), maxv(ml), minv(mr) {
        p.assign(v.size(), c[0].end());
        for (size_type i = 0; i < v.size(); ++i)
            insert(i, v.at(i));
    }

    // dynamic_histogram_heap(const this_type& arg) : n(arg.n), n0(arg.n0), ml(arg.ml), mr(arg.mr), minv(arg.minv), maxv(arg.maxv), c(arg.c) {
    //     rebuild_p();
    // }

    // dynamic_histogram_heap(this_type&& arg) : n(arg.n), ml(arg.ml), mr(arg.mr), minv(arg.minv), maxv(arg.maxv), c(move(arg.c)) {

    // }

    void debug() {
        std::cerr << "n=" << n << " m=[" << ml << "," << mr << ") maxv=" << maxv << " minv=" << minv << std::endl;
        for (key_type i = 0; i < mr - ml; ++i) {
            std::cerr << i + ml << ":";
            for (size_type x : c[i])
                std::cerr << " " << x;
            std::cerr << std::endl;
        }
        std::cerr << std::endl;
    }

};

}

#endif
