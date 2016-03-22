//
// Created by joshm on 3/19/2016.
//

#ifndef MVIS_UTILITY_HPP
#define MVIS_UTILITY_HPP

#include <type_traits>
#include <cmath>

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
            if(min>max) return n;
            if(n>max)
                return (Number)max;
            if(n<min)
                return (Number)min;
            return n;
        }

        /*
         * take a stick, break it in half, then break those sticks in half, and so on, until you do
         * it i times.
         * If these sticks were instead rulers, this function would return where the next break
         * would be in the ruler's range, if you were neatly breaking the ruler from left to right.
         *
         * credit for making this non-recursive goes to: http://math.stackexchange.com/a/1706893/324663
         */
        template<class Number>
        Number ith_middle(const size_t& i, const Number& min, const Number& max){
            size_t depth = std::floor(std::log2(i+1));
            size_t node = i - (1<<depth) +1; //1<<u = 2^u

            Number section_size = (max - min) / (1<<depth);

            Number middle = min + node*section_size + section_size/2;

            return middle;
        };
    }
}

#endif //MVIS_UTILITY_HPP
