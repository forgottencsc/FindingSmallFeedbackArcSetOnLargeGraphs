#ifndef UTIL_TAGGED_TUPLE
#define UTIL_TAGGED_TUPLE
#include <type_traits>

/*  TODO:
*/

namespace util {

template<class TFrom, class TTo, class Other>
struct tag_rebinder;

template<class TFrom, class TTo, class Other = tag_rebinder<void, void, void>>
struct tag_rebinder;

template<class TFrom, class TTo, class Other>
struct tag_rebinder : Other {
    typedef tag_rebinder<TFrom, TTo, Other> this_type;
    typedef Other base_type;

    using Other::operator[];

    constexpr TTo operator[](TFrom) const { return TTo(); }
    
};

template<>
struct tag_rebinder<void, void, void> {
    template<class T>
    constexpr T operator[](T) const { return T(); }
};

template<class T>
struct is_tag_rebinder_impl : std::false_type {};

template<class TFrom, class TTo, class Other>
struct is_tag_rebinder_impl<tag_rebinder<TFrom, TTo, Other>> : std::true_type {};

template<class T>
struct is_tag_rebinder : is_tag_rebinder_impl<std::decay_t<T>> {};

template<class T>
bool is_tag_rebinder_v = is_tag_rebinder<T>::value;


template<class ...Ts>
class tagged_tuple;

template<class T>
struct is_tagged_tuple_impl : std::false_type {};

template<class ...Ts>
struct is_tagged_tuple_impl<tagged_tuple<Ts...>> : std::true_type {};

template<class T>
struct is_tagged_tuple : is_tagged_tuple_impl<std::decay_t<T>> {};

template<class T>
bool is_tagged_tuple_v = is_tagged_tuple<T>::value;


template<>
class tagged_tuple<> {
public:
    typedef tagged_tuple<> this_type;

    tagged_tuple() {}

    template<class ...U>
    tagged_tuple(U&& ...) {}

    template<class T>
    void operator[](T) const {}

    template<class T>
    void get(T) const {}

    bool operator<(const this_type&) const { return false; }
    bool operator>(const this_type&) const { return false; }
    bool operator<=(const this_type&) const { return true; }
    bool operator>=(const this_type&) const { return true; }
    bool operator==(const this_type&) const { return true; }
    bool operator!=(const this_type&) const { return false; }

    template<class T>
    this_type& operator=(T&&) {
        return *this;
    }

    template<class U, class R>
    this_type& assign(U&& u, R r) {
        return *this;
    }

};

#define UTIL_DEFINE_PROPERTY(name)                                          \
namespace util{                                                             \
                                                                            \
namespace property {                                                        \
                                                                            \
template<class T = void>                                                    \
struct name {                                                               \
    operator name<void>() { return name<void>(); }                          \
};                                                                          \
                                                                            \
}                                                                           \
                                                                            \
template<class T, class ...Ts>                                              \
class tagged_tuple<property::name<T>, Ts...>                                \
    : public tagged_tuple<Ts...> {                                          \
                                                                            \
    typedef tagged_tuple<property::name<T>, Ts...> this_type;               \
    typedef tagged_tuple<Ts...> base_type;                                  \
                                                                            \
public:                                                                     \
    typedef T name##_type;                                                  \
    T name;                                                                 \
                                                                            \
    tagged_tuple() {}                                                       \
                                                                            \
    template<class U, class ...Us,                                          \
        std::enable_if_t<!is_tagged_tuple<U>::value, int> = 0>              \
    tagged_tuple(U&& u, Us&& ...us) : base_type(std::forward<Us>(us)...),   \
        name(std::forward<U>(u)) {}                                         \
                                                                            \
    template<class U,                                                       \
        std::enable_if_t<is_tagged_tuple<U>::value, int> = 0>               \
    tagged_tuple(U&& u) : base_type(std::forward<U>(u)),                    \
        name(std::forward<U>(u)[property::name<>()]) {}                     \
                                                                            \
    template<class U, class R,                                              \
        std::enable_if_t<is_tag_rebinder<R>::value, int> = 0>               \
    tagged_tuple(U&& u, R r) : base_type(std::forward<U>(u), r),            \
        name(std::forward<U>(u)[r[property::name<>()]]) {}                  \
                                                                            \
    using base_type::operator[];                                            \
    using base_type::get;                                                   \
                                                                            \
    T& operator[](property::name<>) {                                       \
        return name;                                                        \
    }                                                                       \
                                                                            \
    const T& operator[](property::name<>) const {                           \
        return name;                                                        \
    }                                                                       \
                                                                            \
    T& get(property::name<>) {                                              \
        return name;                                                        \
    }                                                                       \
                                                                            \
    const T& get(property::name<>) const {                                  \
        return name;                                                        \
    }                                                                       \
                                                                            \
    template<class U>                                                       \
    this_type& operator=(U&& u) {                                           \
        name = std::forward<U>(u)[property::name<>()];                      \
        static_cast<base_type&>(*this) = u;                                 \
        return *this;                                                       \
    }                                                                       \
                                                                            \
    template<class U, class R>                                              \
    this_type& assign(U&& u, R r) {                                         \
        name = std::forward<U>(u)[r[property::name<>()]];                   \
        static_cast<base_type&>(*this).assign(u, r);                        \
        return *this;                                                       \
    }                                                                       \
                                                                            \
    bool operator==(const this_type& c) {                                   \
        return name == c.name &&                                            \
            static_cast<const base_type&>(*this) ==                         \
            static_cast<const base_type&>(c);                               \
    }                                                                       \
                                                                            \
    bool operator!=(const this_type& c) {                                   \
        return !(*this == c);                                               \
    }                                                                       \
                                                                            \
    bool operator<(const this_type& c) const {                              \
        if (name < c.name)                                                  \
            return true;                                                    \
        else if (c.name < name)                                             \
            return false;                                                   \
        else                                                                \
            return static_cast<const base_type&>(*this) <                   \
                   static_cast<const base_type&>(c);                        \
    }                                                                       \
                                                                            \
    bool operator>(const this_type& c) const {                              \
        return c < *this;                                                   \
    }                                                                       \
                                                                            \
    bool operator<=(const this_type& c) const {                             \
        return !(c > *this);                                                \
    }                                                                       \
                                                                            \
    bool operator>=(const this_type& c) const {                             \
        return !(c < *this);                                                \
    }                                                                       \
                                                                            \
};                                                                          \
                                                                            \
}


template<class T, size_t I>
struct tagged_tuple_element;

template<class T, class ...Ts, size_t I>
struct tagged_tuple_element<tagged_tuple<T, Ts...>, I> {
    typedef typename tagged_tuple_element<tagged_tuple<Ts...>, I - 1>::type type;
};

template<template<class>class P, class T, class ...Ts>
struct tagged_tuple_element<tagged_tuple<P<T>, Ts...>, 0> {
    typedef T type;
};

template<class T, size_t I>
using tagged_tuple_element_t = typename tagged_tuple_element<T, I>::type;


template<size_t I, template<class> class P, class T, class ...Ts>
const tagged_tuple_element_t<tagged_tuple<P<T>, Ts...>, I>&
get(const tagged_tuple<P<T>, Ts...>& t) {
    if constexpr(I == 0)
        return t[P<void>()];
    else 
        return get<I - 1>(static_cast<const tagged_tuple<Ts...>&>(t));
}

template<size_t I, template<class> class P, class T, class ...Ts>
tagged_tuple_element_t<tagged_tuple<P<T>, Ts...>, I>&
get(tagged_tuple<P<T>, Ts...>& t) {
    if constexpr(I == 0)
        return t[P<void>()];
    else 
        return get<I - 1>(static_cast<tagged_tuple<Ts...>&>(t));
}

template<class ...Tpls>
struct first_tpl;

template<>
struct first_tpl<> {
    typedef tagged_tuple<> type;
};

template<template<class>class ...Ps, class ...Ts, class ...T>
struct first_tpl<tagged_tuple<Ps<Ts>...>, T...> {
    typedef tagged_tuple<Ps<Ts>...> type;
};


template<class T1, class T2, class Tl>
struct tagged_tuple_cat_impl0;

template<class ...Ts2>
struct tagged_tuple_cat_impl0<tagged_tuple<>, tagged_tuple<Ts2...>, tagged_tuple<>> {
    typedef tagged_tuple<Ts2...> type;
    template<class ...Ts>
    type operator()(Ts&& ...args) {
        return type(std::forward<Ts>(args)...);
    }
};

template<class ...Ts1, class ...Ts2, class Tpl, class ...Tpls, template<class>class ...Ps>
struct tagged_tuple_cat_impl0<
    tagged_tuple<Tpl, Tpls...>,
    tagged_tuple<Ts2...>,
    tagged_tuple<Ps<Ts1>...>> {
    typedef tagged_tuple_cat_impl0<
        tagged_tuple<Tpls...>,
        tagged_tuple<Ts2..., Ps<Ts1>...>,
        typename first_tpl<std::decay_t<Tpls>...>::type
    > next;
    typedef typename next::type type;
    template<class ...Ts>
    type operator()(Tpl&& tpl, Tpls&& ...tpls, Ts&& ...ts) {
        return next()(std::forward<Tpls>(tpls)..., std::forward<Ts>(ts)..., std::forward<Tpl>(tpl).get(Ps<void>())...);
    }
};


template<class ...Tpls>
using tagged_tuple_cat_impl = tagged_tuple_cat_impl0<
    tagged_tuple<Tpls...>,
    tagged_tuple<>,
    typename first_tpl<std::decay_t<Tpls>...>::type
>;

template<class ...Tpls>
using tagged_tuple_cat_result_t = typename tagged_tuple_cat_impl<Tpls...>::type;

template<class ...Tpls>
tagged_tuple_cat_result_t<Tpls...> tagged_tuple_cat(Tpls&& ...tpls) {
    return tagged_tuple_cat_impl<Tpls...>()(std::forward<Tpls>(tpls)...);
}


template<class T, class U, template<class>class Q>
struct add_qualifier_impl;

template<class ...Us, template<class>class Q>
struct add_qualifier_impl<tagged_tuple<>, tagged_tuple<Us...>, Q> {
    typedef tagged_tuple<Us...> type;
};

template<template<class>class P, class T, class ...Ts, class ...Us, template<class>class Q>
struct add_qualifier_impl<tagged_tuple<P<T>, Ts...>, tagged_tuple<Us...>, Q> {
    typedef typename add_qualifier_impl<tagged_tuple<Ts...>, tagged_tuple<Us..., P<typename Q<T>::type>>, Q>::type type;
};

template<class T, template<class>class ...Q>
struct add_qualifier;

template<class T>
struct add_qualifier<T> { typedef T type; };

template<class T, template<class>class Q, template<class>class ...Qs>
struct add_qualifier<T, Q, Qs...> : add_qualifier_impl<typename add_qualifier<T, Qs...>::type, tagged_tuple<>, Q> {};

template<class T, template<class>class ...Qs>
using add_qualifier_t = typename add_qualifier<T, Qs...>::type;


template<class ...Ts, class ...Us>
tagged_tuple<Ts...> make_tagged_tuple(Us&& ...args) {
    return tagged_tuple<Ts...>(std::forward<Us>(args)...);
}

template<class ...Ts>
struct tagged_tuple_cmp;

template<>
struct tagged_tuple_cmp<> {
    template<class U>
    bool operator()(const U&, const U&) { return false; }
};

template<template<class> class C, template<class>class P, class T, class ...Ts>
struct tagged_tuple_cmp<C<P<T>>, Ts...> {
    template<class U>
    bool operator()(const U& a, const U& b) {
        return C<T>()(a.get(P<void>()), b.get(P<void>())) ? true :
               C<T>()(b.get(P<void>()), a.get(P<void>())) ? false :
               tagged_tuple_cmp<Ts...>()(a, b);
    }
};

template<class K, class T>
struct tagged_tuple_lower_bound_cmp;

template<template<class>class C, template<class>class P, class T, class U>
struct tagged_tuple_lower_bound_cmp<C<P<T>>, U> {
    bool operator()(const U& u, const T& k) {
        return C<T>()(u.get(P<void>()), k);
    }
};


template<class K, class T>
struct tagged_tuple_upper_bound_cmp;

template<template<class>class C, template<class>class P, class T, class U>
struct tagged_tuple_upper_bound_cmp<C<P<T>>, U> {
    bool operator()(const T& k, const U& u) {
        return C<T>()(k, u.get(P<void>()));
    }
};

}

UTIL_DEFINE_PROPERTY(src)
UTIL_DEFINE_PROPERTY(dst)
UTIL_DEFINE_PROPERTY(weight)

#endif