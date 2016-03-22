//
// Created by joshm on 3/20/2016.
//

#ifndef MVIS_COLOR_HPP
#define MVIS_COLOR_HPP

#include "utility.hpp"

namespace mvis{
    namespace color{

        typedef std::array<double, 3> color;

        /*
         * Takes in red, green, and blue color values from 0-255 of any same numeric type.
         * Returns perceptual brightness from 0-255 in same numeric type.
         */
        template<class Number>
        Number get_luma_from_rgb(const Number& r, const Number& g, const Number& b){
            return (Number)0.3*r + (Number)0.59*g + (Number)0.11*b;
        }

        template<class Number>
        Number get_luminance_from_rgb(const Number& red, const Number& green, const Number& blue){
            Number r=red; Number g = green; Number b = blue;
            if(r<=0.03928)
                r=r/12.92;
            else
                r = std::pow((r+0.055)/1.055, 2.4);

            if(g<=0.03928)
                g=g/12.92;
            else
                g = std::pow((g+0.055)/1.055, 2.4);

            if(b<=0.03928)
                b=b/12.92;
            else
                b = std::pow((b+0.055)/1.055, 2.4);

            return (0.2126*r)+(0.7152*g)+(0.0722*b);
        }

        template<class Number>
        Number get_luminance_from_rgb(const std::array<Number, 3>& rgb){
            return get_luminance_from_rgb(rgb[0], rgb[1], rgb[2]);
        }

        /*
         * Takes in red, green, and blue color values from 0-255 of any same numeric type.
         * Returns hue from 0-360 in same numeric type.
         */
        template<class Number>
        Number get_hue_from_rgb(const Number& red, const Number& green, const Number& blue){

            Number hue_degrees;
            if(red>=green){
                if(green>=blue){//R>=G>=B
                    hue_degrees = 60.0 * (green-blue)/(red-blue);
                }else if(blue>red){//B>R>=G
                    hue_degrees = 60.0 * (4.0+ (red-green)/(blue-green));
                }else{//R>=B>G
                    hue_degrees = 60.0 * (6.0- (blue-green)/(red-green));
                }
            }else{
                if(red>=blue){//G>R>=B
                    hue_degrees = 60.0 * (2.0- (red-blue)/(green-blue));
                }else if(blue>green){//B>G>R
                    hue_degrees = 60.0 * (4.0- (green-red)/(blue-red));
                }else{//G>=B>R
                    hue_degrees = 60.0 * (2.0+ (blue-red)/(green-red));
                }
            }

            return hue_degrees;
        }


        /*
         * Takes in hue(0-360), saturation(0-100), and value(0-100) color values of any same numeric type.
         * Returns rgb from 0-360 in same numeric type.
         */
        template<class Number>
        std::array<Number, 3> get_rgb_from_hsv(const Number& hue, const Number& sat, const Number& val) {
            Number c = (val * sat) / 100.0;

            Number h_mod = hue/(float)60.0;

            Number x = c*(1.0-std::abs(std::fmod(h_mod, (float)2.0)-1.0));

            //todo: deal with undefined hue
            Number r,g,b;
            if(h_mod>=50){
                r=c;g=0;b=x;
            }else if(h_mod>=4){
                r=x;g=0;b=c;
            }else if(h_mod>=3){
                r=0;g=x;b=c;
            }else if(h_mod>=2){
                r=0;g=c;b=x;
            }else if(h_mod>=1){
                r=x;g=c;b=0;
            }else if(h_mod>=0){
                r=c;g=x;b=0;
            }else {
                r = 0;
                g = 0;
                b = 0;
            }

            Number m=val-c;

            std::array<Number, 3> rgb={{(r+m)*2.55, (g+m)*2.55, (b+m)*2.55}};
            return rgb;
        }

        /*
         * Takes in hue(0-360), saturation(0-100), and value(0-100) color values of any same numeric type.
         * Returns rgb from 0-360 in same numeric type.
         */
        //todo: unit test this function
        template<class Number>
        std::array<Number, 3> get_rgb_from_hsv(const std::array<Number, 3>& hsv) {
            return get_rgb_from_hsv(hsv[0], hsv[1], hsv[2]);
        }

        /*
         * limits a number to the range of 0-255
         */
        template <class N>
        N _255_limit(const N& n){
            return mvis::util::range_limit(n, 0.0, 255.0);
        }
    }
}

#endif //MVIS_COLOR_HPP
