#ifndef UTIL_HISTOGRAM_MAP_HPP
#define UTIL_HISTOGRAM_MAP_HPP
#include <algorithm>
#include <memory>
#include <util/tagged_tuple.hpp>
#include <util/histogram_sort.hpp>
#include <util/iterator_range.hpp>
#include <util/pointer_proxy.hpp>

namespace util {

enum elements_are_sorted_t { elements_are_sorted };
enum elements_are_unsorted_t { elements_are_unsorted };

template<class Traits>
class histogram_map_base;

template<class Traits>
class histogram_map_iterator;

template<class Traits>
class histogram_map_const_iterator;

template<class K, class V, class V2, class S = std::size_t, class D = std::ptrdiff_t>
class histogram_map;

template<class Traits>
class histogram_map_iterator {
    typedef histogram_map_iterator<Traits> this_type;
    typedef typename Traits::container_base_type container_base_type;
    typedef typename Traits::partial_reference partial_reference;
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::difference_type difference_type;
    typedef typename Traits::reference reference;
    typedef typename Traits::pointer pointer;

private:
    typedef typename Traits::primary_key_type primary_key_type;
    typedef typename Traits::size_type size_type;
    typedef typename Traits::primary_key_tuple primary_key_tuple;

    container_base_type* c;
    mutable primary_key_type k;
    size_type p;
    
    void locate() const {
        if (k < 0 || k >= c->m || (c->offsets[k] <= p && p < c->offsets[k + 1]))
            return;
            
        while (p < c->offsets[k]) --k;
        while (p >= c->offsets[k + 1]) ++k;
    }

    histogram_map_iterator(container_base_type* c_, size_type k_, size_type p_)
        : c(c_), k(k_), p(p_) {
    }

public:

    template<class K, class K2, class V, class S, class D>
    friend class histogram_map;

    template<class Traits2>
    friend class histogram_map_base;

    template<class Traits2, class>
    friend class histogram_map_slice;

    histogram_map_iterator() = default;
    histogram_map_iterator(const this_type&) = default;
    histogram_map_iterator(this_type&&) = default;

    this_type& operator=(const this_type&) = default;
    this_type& operator=(this_type&&) = default;
    
    friend bool operator==(const this_type& i1, const this_type& i2) {
        return i1.c == i2.c && i1.p == i2.p;
    }
    friend bool operator!=(const this_type& i1, const this_type& i2) {
        return !(i1 == i2);
    }

    reference operator*() const {
        locate();
        return tagged_tuple_cat(primary_key_tuple(k), partial_reference(c->values[p]));
    }

    pointer operator->() const { return pointer(**this); }

    this_type& operator++() {
        p++;
        return *this;
    }

    this_type& operator--() {
        p--;
        return *this;
    }

    this_type operator++(int) {
        this_type tmp = *this;
        ++*this;
        return tmp;
    }

    this_type operator--(int) {
        this_type tmp = *this;
        --*this;
        return tmp;
    }

    friend this_type operator+(const this_type& i1, difference_type d) {
        return this_type(i1.c, i1.k, i1.p + d);
    }

    friend this_type operator-(const this_type& i1, difference_type d) {
        return this_type(i1.c, i1.k, i1.p - d);
    }

    friend difference_type operator-(const this_type& i1, const this_type& i2) {
        return i1.p - i2.p;
    }

    this_type& operator+=(difference_type d) {
        *this = *this + d;
        return *this;
    }

    this_type& operator-=(difference_type d) {
        *this = *this - d;
        return *this;
    }

    reference operator[](difference_type d) {
        return *(*this + d);        
    }

        
    friend bool operator<(const histogram_map_iterator<Traits>& it1, const histogram_map_iterator<Traits>& it2) {
        return it1.p < it2.p;
    }

};

template<class Traits>
class histogram_map_const_iterator {
    typedef histogram_map_const_iterator<Traits> this_type;
    typedef typename Traits::container_base_type container_base_type;
    typedef typename Traits::partial_const_reference partial_reference;
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::difference_type difference_type;
    typedef typename Traits::const_reference reference;
    typedef typename Traits::const_pointer pointer;

private:
    typedef typename Traits::primary_key_type primary_key_type;
    typedef typename Traits::size_type size_type;
    typedef typename Traits::primary_key_tuple primary_key_tuple;
 
    const container_base_type* c;
    primary_key_type k;
    size_type p;
    
    void locate() {
        if (k < 0 || k >= c->m || (c->offsets[k] <= p && p < c->offsets[k + 1]))
            return;
        while (p < c->offsets[k]) --k;
        while (p >= c->offsets[k + 1]) ++k;
    }

    histogram_map_const_iterator(const container_base_type* c_, size_type k_, size_type p_)
        : c(c_), k(k_), p(p_) {
    }

public:

    template<class K, class K2, class V, class S, class D>
    friend class histogram_map;

    template<class Traits2>
    friend class histogram_map_base;

    template<class Traits2, class>
    friend class histogram_map_slice;

    histogram_map_const_iterator() = default;
    histogram_map_const_iterator(const this_type&) = default;
    histogram_map_const_iterator(this_type&&) = default;

    histogram_map_const_iterator(histogram_map_iterator<Traits> it)
        : c(it.c), k(it.k), p(it.p) {}

    this_type& operator=(const this_type&) = default;
    this_type& operator=(this_type&&) = default;
    
    friend bool operator==(const this_type& i1, const this_type& i2) {
        return i1.c == i2.c && i1.p == i2.p;
    }
    friend bool operator!=(const this_type& i1, const this_type& i2) {
        return !(i1 == i2);
    }

    reference operator*() {
        locate();
        return tagged_tuple_cat(primary_key_tuple(k), partial_reference(c->values[p]));
    }

    pointer operator->()  { return pointer(**this); }

    this_type& operator++() {
        p++;
        return *this;
    }

    this_type& operator--() {
        p--;
        return *this;
    }

    this_type operator++(int) {
        this_type tmp = *this;
        ++*this;
        return tmp;
    }

    this_type operator--(int) {
        this_type tmp = *this;
        --*this;
        return tmp;
    }

    friend this_type operator+(const this_type& i1, difference_type d) {
        return this_type(i1.c, i1.k, i1.p + d);
    }

    friend this_type operator-(const this_type& i1, difference_type d) {
        return this_type(i1.c, i1.k, i1.p - d);
    }

    friend difference_type operator-(const this_type& i1, const this_type& i2) {
        return i1.p - i2.p;
    }

    this_type& operator+=(difference_type d) {
        *this = *this + d;
        return *this;
    }

    this_type& operator-=(difference_type d) {
        *this = *this - d;
        return *this;
    }

    reference operator[](difference_type d) {
        return *(*this + d);        
    }
        
    friend bool operator<(const histogram_map_const_iterator<Traits>& it1, const histogram_map_const_iterator<Traits>& it2) {
        return it1.p < it2.p;
    }
};





template<class K1, class K2, class V, class S, class D>
struct histogram_map_traits;


template<template<class> class P1, class T1, class ...Ts, class SizeType, class DiffType>
struct histogram_map_traits<P1<T1>, void, tagged_tuple<Ts...>, SizeType, DiffType> {

    typedef histogram_map_traits<P1<T1>, void, tagged_tuple<Ts...>, SizeType, DiffType> this_type;
    typedef histogram_map_base<this_type> container_base_type;

    typedef T1 primary_key_type;
    typedef P1<void> primary_key_tag;
    typedef P1<T1> primary_key;
    typedef void secondary_key;
    typedef tagged_tuple<P1<T1>> primary_key_tuple;
    typedef tagged_tuple<P1<const T1>> const_primary_key_tuple;
    typedef tagged_tuple<Ts...> partial_value_type;

    typedef tagged_tuple<P1<T1>, Ts...> value_type;
    typedef SizeType size_type;
    typedef DiffType difference_type;
    typedef add_qualifier_t<partial_value_type, std::add_lvalue_reference> partial_reference;
    typedef add_qualifier_t<partial_value_type, std::add_lvalue_reference, std::add_const> partial_const_reference;
    typedef tagged_tuple_cat_result_t<const_primary_key_tuple, partial_reference> reference;
    typedef tagged_tuple_cat_result_t<const_primary_key_tuple, partial_const_reference> const_reference;

    typedef histogram_map_iterator<this_type> iterator;
    typedef histogram_map_const_iterator<this_type> const_iterator;

    template<class Ref>
    struct pointer_proxy {
        Ref ref;
        template<class ...Args>
        pointer_proxy(Args&& ...args) : ref(std::forward<Args>(args)...) {}

        Ref* operator->() { return &ref; }
    };

    typedef pointer_proxy<reference> pointer;
    typedef pointer_proxy<const_reference> const_pointer;

};

template<template<class> class P1, class T1, template<class>class P2, class T2, class ...Ts, class SizeType, class DiffType>
struct histogram_map_traits<P1<T1>, P2<T2>, tagged_tuple<P2<T2>, Ts...>, SizeType, DiffType> {

    typedef histogram_map_traits<P1<T1>, P2<T2>, tagged_tuple<P2<T2>, Ts...>, SizeType, DiffType> this_type;
    typedef histogram_map_base<this_type> container_base_type;

    typedef T1 primary_key_type;
    typedef P1<void> primary_key_tag;
    typedef P1<T1> primary_key;
    typedef T2 secondary_key_type;
    typedef P2<void> secondary_key_tag;
    typedef P2<T2> secondary_key;
    
    typedef tagged_tuple<P1<T1>> primary_key_tuple;
    typedef tagged_tuple<P2<T2>> secondary_key_tuple;
    typedef tagged_tuple<P1<const T1>> const_primary_key_tuple;
    typedef tagged_tuple<P2<const T2>> const_secondary_key_tuple;
    typedef tagged_tuple<P2<T2>, Ts...> partial_value_type;

    typedef tagged_tuple<P1<T1>, P2<T2>, Ts...> value_type;
    typedef SizeType size_type;
    typedef DiffType difference_type;
    typedef add_qualifier_t<partial_value_type, std::add_lvalue_reference> partial_reference;
    typedef add_qualifier_t<partial_value_type, std::add_lvalue_reference, std::add_const> partial_const_reference;
    typedef tagged_tuple_cat_result_t<const_primary_key_tuple, partial_reference> reference;
    typedef tagged_tuple_cat_result_t<const_primary_key_tuple, partial_const_reference> const_reference;

    typedef tagged_tuple_lower_bound_cmp<std::less<P2<T2>>, const_reference> secondary_key_lower_bound_cmp;
    typedef tagged_tuple_upper_bound_cmp<std::less<P2<T2>>, const_reference> secondary_key_upper_bound_cmp;

    typedef histogram_map_iterator<this_type> iterator;
    typedef histogram_map_const_iterator<this_type> const_iterator;

    typedef pointer_proxy<reference> pointer;
    typedef pointer_proxy<const_reference> const_pointer;

};

template<class Traits, class SecondaryKey>
class histogram_map_slice {
public:
    typedef typename Traits::container_base_type container_base_type;

    typedef typename Traits::value_type value_type;
    typedef typename Traits::size_type size_type;
    typedef typename Traits::difference_type difference_type;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_reference const_reference;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::const_pointer const_pointer;

    typedef typename Traits::primary_key_type primary_key_type;
    typedef typename Traits::primary_key_tag primary_key_tag;
    typedef typename Traits::secondary_key_type secondary_key_type;
    typedef typename Traits::secondary_key_tag secondary_key_tag;
    
    typedef typename Traits::const_iterator iterator;
    typedef typename Traits::const_iterator const_iterator;

    const container_base_type* p;
    const primary_key_type k;

    histogram_map_slice(const container_base_type* p_, primary_key_type k_) : p(p_), k(k_) {}

    iterator begin() const {
        return iterator{ p, k, p->offsets[k] };
    }

    iterator end() const {
        return iterator{ p, k + 1, p->offsets[k + 1] };
    }

    iterator begin(secondary_key_type k2) const {
        return lower_bound(begin(), end(), k2, typename Traits::secondary_key_lower_bound_cmp());
    }

    iterator end(secondary_key_type k2) const {
        return upper_bound(begin(), end(), k2, typename Traits::secondary_key_upper_bound_cmp());
    }

    iterator_range<iterator> operator()(secondary_key_type k2) const {

        return iterator_range<iterator>(begin(k2), end(k2));
    }

    iterator_range<iterator> operator()(secondary_key_type k2l, secondary_key_type k2r) const {
        return iterator_range<iterator>(begin(k2l), end(k2r));
    }

    size_type size() const { return p->offsets[k + 1] - p->offsets[k]; }

    bool empty() const { return size() == 0; }

};

template<class Traits>
class histogram_map_slice<Traits, void> {
public:
    typedef typename Traits::container_base_type container_base_type;

    typedef typename Traits::value_type value_type;
    typedef typename Traits::size_type size_type;
    typedef typename Traits::difference_type difference_type;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_reference const_reference;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::const_pointer const_pointer;

    typedef typename Traits::primary_key_type primary_key_type;
    typedef typename Traits::primary_key_tag primary_key_tag;
    
    typedef typename Traits::const_iterator iterator;
    typedef typename Traits::const_iterator const_iterator;

    const container_base_type* p;
    const primary_key_type k;

    histogram_map_slice(const container_base_type* p_, primary_key_type k_) : p(p_), k(k_) {}

    iterator begin() const {
        return iterator{ p, k, p->offsets[k] };
    }

    iterator end() const {
        return iterator{ p, k + 1, p->offsets[k + 1] };
    }

    size_type size() const { return p->offsets[k + 1] - p->offsets[k]; }

    bool empty() const { return size() == 0; }

};

template<class Traits>
class histogram_map_base {
    typedef histogram_map_base<Traits> this_type;
public:
    typedef typename Traits::partial_value_type partial_value_type;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::size_type size_type;
    typedef typename Traits::difference_type difference_type;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_reference const_reference;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::const_pointer const_pointer;

    typedef typename Traits::primary_key_type primary_key_type;
    typedef typename Traits::primary_key_tag primary_key_tag;

    typedef histogram_map_iterator<Traits> iterator;
    typedef histogram_map_const_iterator<Traits> const_iterator;
    typedef histogram_map_slice<Traits, typename Traits::secondary_key> slice;

    typedef std::allocator<size_type> offset_alloc_type;
    typedef std::allocator<partial_value_type> value_alloc_type;

    template<class Traits2, class>
    friend class histogram_map_slice;

    template<class Traits2>
    friend class histogram_map_iterator;

    template<class Traits2>
    friend class histogram_map_const_iterator;

protected:
    offset_alloc_type offset_alloc;
    value_alloc_type value_alloc;

    size_type m;
    size_type* offsets;
    partial_value_type* values;

    primary_key_type locate(size_type s) const {
        return std::upper_bound(offsets, offsets + m, s) - offsets - 1;
    }

    void copy_from(const this_type& x) {
        m = x.m;
        if (m == 0) {
            offsets = nullptr;
            values = nullptr;
            return;
        } 
        offsets = offset_alloc.allocate(m + 1);
        for (size_type i = 0; i < m + 1; ++i)
            offset_alloc.construct(offsets + i, x.offsets[i]);
        size_type n = offsets[m];
        values = value_alloc.allocate(n);
        for (size_type i = 0; i < n; ++i)
            value_alloc.construct(values + i, x.values[i]);
    }

    void move_from(this_type&& x) noexcept {
        m = x.m;
        if (m == 0) {
            offsets = nullptr;
            values = nullptr;
            return;
        } 
        offsets = x.offsets;
        values = x.values;
        x.m = 0;
        x.offsets = nullptr;
        x.values = nullptr;
    }

    void clean() {
        if (m == 0) return;
        for (size_type i = 0; i < size(); ++i)
            value_alloc.destroy(values + i);
        value_alloc.deallocate(values, size());
        offset_alloc.deallocate(offsets, m + 1);
        m = 0;
        values = nullptr;
        offsets = nullptr;
    }

public:

    histogram_map_base() noexcept : m(0), offsets(nullptr), values(nullptr) {}

    size_type size() const { return m == 0 ? 0 : offsets[m]; }

    bool empty() const { return size() == 0; }
    
    iterator begin() {
        return iterator{ this, 0, 0 };
    }

    iterator end() {
        return iterator{ this, m, size() };
    }
    
    const_iterator begin() const  {
        return const_iterator{ this, 0, 0 };
    }

    const_iterator end() const {
        return const_iterator{ this, m, size() };
    }

};


template<template<class> class P, class T, class ...Ts, class SizeType, class DiffType>
class histogram_map<P<T>, void, tagged_tuple<Ts...>, SizeType, DiffType>
    : public histogram_map_base<histogram_map_traits<P<T>, void, tagged_tuple<Ts...>, SizeType, DiffType>> {
    typedef histogram_map<P<T>, void, tagged_tuple<Ts...>, SizeType, DiffType> this_type;
    typedef histogram_map_traits<P<T>, void, tagged_tuple<Ts...>, SizeType, DiffType> traits;
    typedef histogram_map_base<traits> base_type;

    using base_type::m;
    using base_type::values;
    using base_type::offsets;
    using base_type::value_alloc;
    using base_type::offset_alloc;

    using base_type::clean;
    using base_type::copy_from;
    using base_type::move_from;

public:
    typedef typename traits::partial_value_type partial_value_type;
    typedef typename traits::value_type value_type;
    typedef typename traits::size_type size_type;
    typedef typename traits::difference_type difference_type;
    typedef typename traits::reference reference;
    typedef typename traits::const_reference const_reference;
    typedef typename traits::pointer pointer;
    typedef typename traits::const_pointer const_pointer;

    typedef typename traits::primary_key_type primary_key_type;
    typedef typename traits::primary_key_tag primary_key_tag;

    typedef histogram_map_iterator<traits> iterator;
    typedef histogram_map_const_iterator<traits> const_iterator;
    typedef histogram_map_slice<traits, void> slice;

    typedef std::allocator<size_type> offset_alloc_type;
    typedef std::allocator<partial_value_type> value_alloc_type;

    using base_type::size;
    using base_type::empty;
    using base_type::begin;
    using base_type::end;

    template<class FwdIt>
    void assign(size_type m_, FwdIt first, FwdIt last, elements_are_unsorted_t) {
        clean();
        m = m_;
        if (m == 0) return;
        offsets = offset_alloc.allocate(m + 1);
        std::fill_n(offsets, m + 1, 0);
        for (FwdIt it = first; it != last; ++it)
            offsets[it->get(primary_key_tag())]++;
        for (size_type i = 0; i < m; ++i)
            offsets[i + 1] += offsets[i];
        
        size_type n = offsets[m];
        values = value_alloc.allocate(n);
        for (FwdIt it = first; it != last; ++it) {
            size_type pos = --offsets[it->get(primary_key_tag())];
            value_alloc.construct(values + pos, *it);
        }
    }

    template<class FwdIt>
    void assign(size_type m_, size_type n_, FwdIt first, FwdIt last, elements_are_sorted_t)  {
        clean();
        m = m_;
        if (m == 0) return;
        offsets = offset_alloc.allocate(m);
        values = value_alloc.allocate(n_);
        primary_key_type k_prev;
        FwdIt it = first;
        for (size_type i = 0; i < n_; ++i, ++it) {
            primary_key_type k_cur = it->get(primary_key_tag());
            if (i == 0 || k_prev != k_cur) {
                for (primary_key_type j = i ? k_prev + 1 : 0; ; ++j) {
                    offset_alloc.construct(offsets + j, i);
                    if (j == k_cur) break;
                }
                k_prev = k_cur;
            }
            value_alloc.construct(values + i, *it);
        }
        for (primary_key_type j = n_ ? k_prev + 1 : 0; ; ++j) {
            offset_alloc.construct(offsets + j, n_);
            if (j == m) break;
        }
    }

    template<class FwdIt>
    void assign(size_type m_, FwdIt first, FwdIt last, elements_are_sorted_t)  {
        assign(m_, distance(first, last), first, last, elements_are_sorted);
    }

    histogram_map() { }

    template<class FwdIt>
    histogram_map(size_type m_, FwdIt first, FwdIt last, elements_are_unsorted_t) {
        assign(m_, first, last, elements_are_unsorted);
    }

    template<class FwdIt>
    histogram_map(size_type m_, FwdIt first, FwdIt last, elements_are_sorted_t) {
        assign(m_, first, last, elements_are_sorted);
    }

    histogram_map(const this_type& x) {
        copy_from(x);
    }

    histogram_map(this_type&& x) noexcept {
        move_from(std::move(x));
    }

    this_type& operator=(const this_type& x) {
        clean();
        copy_from(x);
    }

    this_type& operator=(this_type&& x) {
        clean();
        move_from(std::move(x));
    }

    slice operator[](primary_key_type k) const {
        return slice(this, k);
    }

    ~histogram_map() {
        clean();
    }

};


template<template<class> class P1, class T1, template<class> class P2, class T2, class ...Ts, class SizeType, class DiffType>
class histogram_map<P1<T1>, P2<T2>, tagged_tuple<P2<T2>, Ts...>, SizeType, DiffType>
    : public histogram_map_base<histogram_map_traits<P1<T1>, P2<T2>, tagged_tuple<P2<T2>,Ts...>, SizeType, DiffType>> {
    typedef histogram_map<P1<T1>, P2<T2>, tagged_tuple<P2<T2>, Ts...>, SizeType, DiffType> this_type;
    typedef histogram_map_traits<P1<T1>, P2<T2>, tagged_tuple<P2<T2>, Ts...>, SizeType, DiffType> traits;
    typedef histogram_map_base<traits> base_type;

    using base_type::m;
    using base_type::values;
    using base_type::offsets;
    using base_type::value_alloc;
    using base_type::offset_alloc;

    using base_type::clean;
    using base_type::copy_from;
    using base_type::move_from;
    
public:
    typedef typename traits::partial_value_type partial_value_type;
    typedef typename traits::value_type value_type;
    typedef typename traits::size_type size_type;
    typedef typename traits::difference_type difference_type;
    typedef typename traits::reference reference;
    typedef typename traits::const_reference const_reference;
    typedef typename traits::pointer pointer;
    typedef typename traits::const_pointer const_pointer;

    typedef typename traits::primary_key_type primary_key_type;
    typedef typename traits::primary_key_tag primary_key_tag;
    typedef typename traits::secondary_key_type secondary_key_type;
    typedef typename traits::secondary_key_tag secondary_key_tag;

    typedef histogram_map_iterator<traits> iterator;
    typedef histogram_map_const_iterator<traits> const_iterator;
    typedef histogram_map_slice<traits, P2<T2>> slice;

    typedef std::allocator<size_type> offset_alloc_type;
    typedef std::allocator<partial_value_type> value_alloc_type;

    using base_type::size;
    using base_type::empty;
    using base_type::begin;
    using base_type::end;

    template<class RndIt>
    void assign(size_type m_, RndIt first, RndIt last, elements_are_unsorted_t) {
        clean();
        m = m_;
        if (m == 0) return ;
        offsets = offset_alloc.allocate(m + 1);
        std::fill_n(offsets, m + 1, 0);
        for (RndIt it = first; it != last; ++it)
            offsets[it->get(primary_key_tag())]++;
        for (size_type i = 0; i < m; ++i)
            offsets[i + 1] += offsets[i];
        
        histogram_sort(first, last, property::reversed<secondary_key_tag>());
        
        size_type n = offsets[m];
        values = value_alloc.allocate(n);
        for (RndIt it = first; it != last; ++it) {
            size_type pos = --offsets[it->get(primary_key_tag())];
            value_alloc.construct(values + pos, *it);
        }
    }

    template<class FwdIt>
    void assign(size_type m_, size_type n_, FwdIt first, FwdIt last, elements_are_sorted_t)  {
        clean();
        m = m_;
        if (m == 0) return;
        offsets = offset_alloc.allocate(m);
        values = value_alloc.allocate(n_);
        primary_key_type k_prev;
        FwdIt it = first;
        for (size_type i = 0; i < n_; ++i, ++it) {
            primary_key_type k_cur = it->get(primary_key_tag());
            if (i == 0 || k_prev != k_cur) {
                for (primary_key_type j = i ? k_prev + 1 : 0; ; ++j) {
                    offset_alloc.construct(offsets + j, i);
                    if (j == k_cur) break;
                }
                k_prev = k_cur;
            }
            value_alloc.construct(values + i, *it);
        }
        for (primary_key_type j = n_ ? k_prev + 1 : 0; ; ++j) {
            offset_alloc.construct(offsets + j, n_);
            if (j == m) break;
        }
    }

    template<class RndIt>
    void assign(size_type m_, RndIt first, RndIt last, elements_are_sorted_t)  {
        assign(m_, std::distance(first, last), first, last, elements_are_sorted);
    }



    histogram_map() { }

    template<class RndIt>
    histogram_map(size_type m_, RndIt first, RndIt last, elements_are_unsorted_t) {
        assign(m_, first, last, elements_are_unsorted);
    }

    template<class FwdIt>
    histogram_map(size_type m_, FwdIt first, FwdIt last, elements_are_sorted_t) {
        assign(m_, first, last, elements_are_sorted);
    }

    histogram_map(const this_type& x) {
        copy_from(x);
    }

    histogram_map(this_type&& x) noexcept {
        move_from(std::move(x));
    }

    this_type& operator=(const this_type& x) {
        clean();
        copy_from(x);
        return *this;
    }

    this_type& operator=(this_type&& x) {
        clean();
        move_from(std::move(x));
        return *this;
    }

    slice operator[](primary_key_type k) const {
        return slice(this, k);
    }

    ~histogram_map() {
        clean();
    }

};

}


#endif