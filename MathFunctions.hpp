//
// Created by joshm on 3/19/2016.
//

#ifndef MVIS_MATHFUNCTIONS_HPP
#define MVIS_MATHFUNCTIONS_HPP

#include <complex>
#include <cmath>

namespace mvis {
    //the golden ratio has to be a complex to be useful for power functions
    //todo: if number is std::complex, don't make a complex of a complex
    //todo: do something to make this at least c++11 and not c++14
    template<class Number>
    std::complex<Number> golden = std::complex<Number>(1.6180339887498948482,0);

    template<class Number>
    Number fib(const Number& x) {
        return (Number)
                (((std::pow(golden<Number>, x)
                   - std::pow(-golden<Number>, -x))
                  / (float)std::sqrt(5)).real());
    }
}

#endif //MVIS_MATHFUNCTIONS_HPP
