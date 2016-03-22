//
// Created by joshm on 3/19/2016.
//

#ifndef MVIS_MATHFUNCTIONS_HPP
#define MVIS_MATHFUNCTIONS_HPP

#include <complex>
#include <cmath>

namespace mvis {

    template<class Number>
    Number fib(const Number& x) {
        //the golden ratio has to be a complex to be useable for power functions
        //todo: if number is std::complex, don't make a complex of a complex
        std::complex<Number> golden = std::complex<Number>(1.6180339887498948482,0);

        return (Number)
                (((std::pow(golden, x)
                   - std::pow(-golden, -x))
                  / (float)std::sqrt(5)).real());
    }
}

#endif //MVIS_MATHFUNCTIONS_HPP
