#error "Incomplete! Don't use!"
#ifndef GRAPH_DYNAMIC_HPP
#define GRAPH_DYNAMIC_HPP
#include <tagged_tuple.hpp>
#include <util/pointer_proxy.hpp>

#if __GNUC__ > 3 || \
    (__GNUC__ == 3 && (__GNUC_MINOR__ > 2 || \
                       (__GNUC_MINOR__ == 2 && \
                        __GNUC_PATCHLEVEL__ > 0)))
#define GRAPH_HAVE_PBDS 1
#include <ext/pb_ds/assoc_container.hpp>
#else 
#define GRAPH_HAVE_PBDS 0
#include <unordered_map>
#endif

namespace graph {

using namespace util;

using property::src;
using property::dst;
using property::weight;

template<class K1, class K2, class V>
class dynamic_edge_container;

template<class K1, class K2, class V>
struct dynamic_edge_container_traits;

template<class Traits>
class dynamic_edge_container_iterator;

template<class Traits>
class dynamic_edge_container_const_iterator;

template<class Traits>
class dynamic_edge_container_iterator {
    typedef dynamic_edge_container_iterator<Traits> this_type;
public:
    typedef typename Traits::value_type value_type;
    typedef typename Traits::difference_type difference_type;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_reference const_reference;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::const_pointer const_pointer;

    typedef typename Traits::edge_container edge_container;

    typedef typename Traits::primary_key_type primary_key_type;
    typedef typename Traits::primary_key_tag primary_key_tag;
    typedef typename Traits::primary_key primary_key;
    typedef typename Traits::secondary_key_type secondary_key_type;
    typedef typename Traits::secondary_key_tag secondary_key_tag;
    typedef typename Traits::secondary_key secondary_key;
    
    typedef typename Traits::adjacency_container::iterator iterator;

private:
    edge_container* c;
    primary_key_type k;
    iterator p;

    dynamic_edge_container_iterator(edge_container* c_, primary_key_type k_, iterator p_) : c(c_), k(k_), p(p_) {}

public:

    template<class K1, class K2, class V>
    friend class dynamic_edge_container;

    dynamic_edge_container_iterator() : c(nullptr) {}
    dynamic_edge_container_iterator(const this_type&) = default;
    dynamic_edge_container_iterator(this_type&&) = default;

    reference operator*() const {
        return tagged_tuple_cat(make_tagged_tuple<primary_key, secondary_key>(k, p->first), p->second);
    }

    pointer operator->() const {
        return pointer(**this);
    }

    this_type& operator++() {
        p++;
        while (p == c[k].end()) {
            k++;
            p = c[k].begin();
        }
        return *this;
    }

    this_type operator++(int) {
        this_type tmp = *this;
        ++*this;
        return tmp;
    }

    this_type& operator--() {
        while (p == c[k].begin()) {
            k--;
            p = c[k].end();
        }
        p--;
        return *this;
    }

    this_type operator--(int) {
        this_type tmp = *this;
        --*this;
        return tmp;
    }

};

template<class Traits>
class dynamic_edge_container_slice;

template<template<class> class P1, class T1, template<class> class P2, class T2, class ...Ts>
struct dynamic_edge_container_traits<P1<T1>, P2<T2>, tagged_tuple<Ts...>> {
public:
    typedef dynamic_edge_container<P1<T1>, P2<T2>, tagged_tuple<Ts...>> container_type;

    typedef tagged_tuple<P1<T1>, P2<T2>, Ts...> value_type;
    typedef tagged_tuple<Ts...> partial_value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef add_qualifier_t<partial_value_type, std::add_lvalue_reference> partial_reference;
    typedef add_qualifier_t<partial_value_type, std::add_const, std::add_lvalue_reference> partial_const_reference;

    typedef tagged_tuple_cat_result_t<P1<const T1>, P2<const T2>, partial_reference> reference;
    typedef tagged_tuple_cat_result_t<P1<const T1>, P2<const T2>, partial_const_reference> const_reference;

    typedef pointer_proxy<reference> pointer;
    typedef pointer_proxy<const_reference> const_pointer;

    typedef P1<T1> primary_key;
    typedef T1 primary_key_type;
    typedef P1<void> primary_key_tag;
    typedef P2<T2> secondary_key;
    typedef T2 secondary_key_type;
    typedef P2<void> secondary_key_tag;

#if GRAPH_HAVE_PBDS
    typedef __gnu_pbds::gp_hash_table<T2, partial_value_type> adjacency_container;
#else
    typedef std::unordered_map<T2, partial_value_type> adjacency_container;
#endif

    typedef std::vector<adjacency_container> edge_container;

};


template<template<class>class P1, class T1, template<class>class P2, class T2, class ...Ts>
class dynamic_edge_container<P1<T1>, P2<T2>, tagged_tuple<Ts...>> {
public:
    
    typedef dynamic_edge_container_traits<P1<T1>, P2<T2>, tagged_tuple<Ts...>> traits;
    
    typedef typename traits::value_type value_type;
    typedef typename traits::size_type size_type;
    typedef typename traits::difference_type difference_type;
    typedef typename traits::reference reference;
    typedef typename traits::const_reference const_reference;
    typedef typename traits::pointer pointer;
    typedef typename traits::const_pointer const_pointer;

    typedef typename traits::adjacency_container adjacency_container;
    typedef typename traits::edge_container edge_container;

    typedef typename traits::primary_key primary_key;
    typedef typename traits::primary_key_type primary_key_type;
    typedef typename traits::primary_key_tag primary_key_tag;
    typedef typename traits::secondary_key_type secondary_key_type;
    typedef typename traits::secondary_key_tag secondary_key_tag;

    typedef typename adjacency_container::iterator adjacency_iterator;
    typedef typename adjacency_container::const_iterator adjacency_const_iterator;
    typedef dynamic_edge_container_iterator<traits> iterator;
    typedef dynamic_edge_container_const_iterator<traits> const_iterator;

    edge_container c;

    template<class FwdIt>
    dynamic_edge_container(size_type n, FwdIt first, FwdIt last) : c(n) {
        for (FwdIt it = first; it != last; ++it)
            add_edge(*it);
    }

    template<class Tpl>
    pair<iterator, bool> add_edge(Tpl&& tpl) {
        auto res = c[tpl.get(primary_key_tag())].insert({ tpl.get(secondary_key_tag()), std::forward<Tpl>(tpl) });
        return pair<iterator, bool>(iterator(&c, tpl.get(primary_key_tag()), res.first), res.second);
    }

    void del_edge(T1 u, T2 v) {
        c[u].erase(v);
    }

    iterator find_edge(T1 u, T2 v) {
        adjacency_iterator it = c[u].find(v);
        if (it == c[u].end())
            return end();
        else 
            return iterator(&c, u, it);
    }

    iterator find_edge(T1 u, T2 v) {
        adjacency_iterator it = c[u].find(v);
        if (it == c[u].end())
            return end();
        else 
            return iterator(&c, u, it);
    }

    const_iterator find_edge(T1 u, T2 v) const {
        adjacency_const_iterator it = c[u].find(v);
        if (it == c[u].end())
            return end();
        else 
            return const_iterator(&c, u, it);
    }

};

template<class VertexSizeType, class EdgeSizeType, class EdgeWeightType>
struct dynamic_graph_traits {
    
    typedef VertexSizeType vertex_id_type;

    typedef EdgeSizeType edge_id_type;

    typedef EdgeWeightType edge_weight_type;

    typedef tagged_tuple<src<vertex_id_type>, dst<vertex_id_type>, weight<edge_weight_type>> edge_type;
    typedef tagged_tuple<src<vertex_id_type>, weight<edge_weight_type>> in_edge_type;
    typedef tagged_tuple<dst<vertex_id_type>, weight<edge_weight_type>> out_edge_type;

#if GRAPH_HAVE_PBDS
    typedef __gnu_pbds::gp_hash_table<vertex_id_type, edge_weight_type> adjacency_container;
#else 
    typedef std::unordered_map<vertex_id_type, out_edge_type> out_adjacency_container;
    typedef std::unordered_map<vertex_id_type, in_edge_type> in_adjacency_container;
#endif
    typedef std::vector<adjacency_container> edge_container;
};

template<class Traits>
class dynamic_graph_adjacency_iterator : Traits::edge_container::iterator {
public:
    
};

template<class VertexSizeType, class EdgeSizeType, class EdgeWeightType>
class dynamic_graph {
public:
    typedef dynamic_graph<VertexSizeType, EdgeSizeType, EdgeWeightType> this_type;

    typedef dynamic_graph_traits<VertexSizeType, EdgeSizeType, EdgeWeightType> traits;

    typedef typename traits::vertex_id_type vertex_id_type;
    typedef typename traits::edge_id_type edge_id_type;

    typedef typename traits::edge_weight_type edge_weight_type;

    typedef typename traits::edge_type edge_type;

    typedef typename traits::out_edge_container out_edge_container;
    typedef typename traits::in_edge_container in_edge_container;

    typedef typename out_edge_container::value_type out_edge_type;
    typedef typename in_edge_container::value_type in_edge_type;

private:
    vertex_id_type m_n;
    in_edge_container m_in_edges;
    out_edge_container m_out_edges;

public:
    dynamic_graph() = default;
    dynamic_graph(const this_type&) = default;
    dynamic_graph(this_type&&) = default;

    this_type& operator=(const this_type&) = default;
    this_type& operator=(this_type&&) = default;

    ~dynamic_graph() = default;

    template<class FwdIt>
    dynamic_graph(vertex_id_type n, FwdIt begin_, FwdIt end_) : m_n(n), m_in_edges(n), m_out_edges(n) {
        for (auto it = begin_; it != end_; ++it) {
            m_in_edges[it->dst][it->src] += it->weight;
            m_out_edges[it->src][it->dst] += it->weight;
        }
    }

    vertex_id_type num_vertices() const { return m_n; }

    edge_id_type num_edges() const { return m_out_edges.size(); }


    in_adjacency_container in_edges(vertex_id_type v) const {
        return in_adjacency_container(m_in_edges.begin(v), m_in_edges.end(v));
    }

    out_adjacency_container out_edges(vertex_id_type v) const {
        return out_adjacency_container(m_out_edges.begin(v), m_out_edges.end(v));
    }

};


}


#endif