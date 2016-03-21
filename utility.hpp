//
// Created by joshm on 3/19/2016.
//

#ifndef MVIS_UTILITY_HPP
#define MVIS_UTILITY_HPP

#include <type_traits>


namespace mvis {
    namespace util {
        /*namespace mpl {
            //todo: doesn't work. replace with boost mpl
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
            > {
            };

            template<class... Tail>
            struct get_highest_precision;

            template<class Head, class... Tail>
            struct get_highest_precision<Head, Tail...> {

                typedef std::conditional<
                        (get_precision<Head>::value > get_precision<get_highest_precision<Tail...>::type>::value),
                        Head,
                        get_highest_precision<Tail...> >::type type;
            };

            struct get_highest_precision<class Head> {
                typedef Head type;
            };
        }*/

        template<class Number, class Number2>//Using two templates in case people type in 2.0 instead of a var name.
        Number range_limit(const Number& n, const Number2& min, const Number2& max){
            if(n>max)
                return (Number)max;
            if(n<min)
                return (Number)min;
            return n;
        }


    }
}

#endif //MVIS_UTILITY_HPP
