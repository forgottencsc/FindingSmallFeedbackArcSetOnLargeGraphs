#ifndef UTIL_ITERATOR_RANGE_HPP
#define UTIL_ITERATOR_RANGE_HPP

namespace util {

template<class Iter>
struct iterator_range {
    Iter first, last;

    iterator_range(Iter first_, Iter last_) : first(first_), last(last_) {}

    Iter begin() const { return first; }

    Iter end() const { return last; }
};

}

#endif