//
// Created by joshm on 3/19/2016.
//

#ifndef MVIS_UTILITY_HPP
#define MVIS_UTILITY_HPP

#include <type_traits>

//todo: use boost mpl instead of learning all of template metaprogramming

//todo: test whatever witchcraft this is
template<class T>
struct get_precision : std::integral_constant<
        int,
        std::conditional<
        std::is_integral<T>,
        1,
                       std::conditional<
                               std::is_same<T, float>,
        32,
                       std::conditional<
                               std::is_same<T, double>,
        64,
                       std::conditional<
                               std::is_same<T, long double>,
        128,
        0
        >
        >
        >
        >
        >{};

template <class... Tail> struct get_highest_precision;

template <class Head, class... Tail>
struct get_highest_precision<Head, Tail...>{

    typedef std::conditional<
            (get_precision<Head>::value > get_precision<get_highest_precision<Tail...>::type >::value),
            Head,
            get_highest_precision<Tail...> >::type type;
};

struct get_highest_precision<class Head>{
    typedef Head type;
};

#endif //MVIS_UTILITY_HPP
