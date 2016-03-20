#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>

#include <vtkVersion.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkChartXY.h>
#include <vtkTable.h>
#include <vtkPlot.h>
#include <vtkFloatArray.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkPen.h>

#include "MathFunctions.hpp"

#include <boost/any.hpp>
#include <boost/variant.hpp>

#include <type_traits>

//todo:put util stuff in utility.hpp
template<class Number, class Number2>//compilers often change numbers to
Number range_limit(const Number& n, const Number2& min, const Number2& max){
    if(n>max)
        return max;
    if(n<min)
        return min;
    return n;
}
//todo:put color stuff in its own file
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

template<class Number>
std::array<Number, 3> get_rgb_from_hsv(const Number& hue, const Number& sat, const Number& val) {
    Number c = (val * sat) / 100.0;
    std::cout << "c: " << c << "\n";

    Number h_mod = hue/(float)60.0;
    std::cout << "h_mod: " << h_mod << "\n";

    Number x = c*(1.0-std::abs(std::fmod(h_mod, (float)2.0)-1.0));
    std::cout << "x: " << x << "\n";

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

template <class T, std::size_t N>
ostream& operator<<(ostream& o, const std::array<T, N>& arr)
{
    copy(arr.cbegin(), arr.cend(), std::ostream_iterator<T>(o, " "));
    return o;
}

//todo: make the function plotter a subclass of this class and make this class only be a parent.
namespace mvis{
    template <class Number>
    class Graph{
    public:
        Graph(float r=255, float g=187, float b=28, const char* x_axis_label = "X Axis"):
                table(vtkSmartPointer<vtkTable>::New()),
                x_axis(vtkSmartPointer<vtkFloatArray>::New()),
                view(vtkSmartPointer<vtkContextView>::New()),
                chart(vtkSmartPointer<vtkChartXY>::New()),
                material_red(r), material_green(g), material_blue(b)
        {
            x_axis->SetName(x_axis_label);
            table->AddColumn(x_axis);

            float hue =get_hue_from_rgb(r,g,b);
            std::cout<<hue<<"\n";
            std::array<float, 3> bg_rgb = get_rgb_from_hsv(hue,(float)20,(float)95);
            bg_red = bg_rgb[0];
            bg_green = bg_rgb[1];
            bg_blue = bg_rgb[2];

            std::cout<<"bgrgb"<<bg_rgb<<"\n";
        }

        typedef std::function< Number(Number& x)>function_type;

        typedef std::pair<
                vtkSmartPointer<vtkFloatArray>,
                function_type
        > function_pair;

        typedef std::vector<function_pair> function_vector;

        template <class N>
        N color_limit(const N& n){
            return range_limit(n, 0.0, 255.0);
        }

        void addFunction(const char* function_name,
                         function_type function){
            vtkSmartPointer<vtkFloatArray> column = vtkSmartPointer<vtkFloatArray>::New();
            column->SetName(function_name);
            table->AddColumn(column);
            function_list.push_back(
                    function_pair(column, function)//pass in function by value for speed
            );
        }

        //todo: go back to single function method and have 2 different vectors(for single and multi) later
        //todo: instead of 0-10, it's going from 0-52
        void calcRegion(const size_t& x_min, const size_t& x_max, const Number& x_increment){
            if(x_min>x_max) return;

            size_t num_points = (Number)(x_max - x_min) / x_increment;
            table->SetNumberOfRows(num_points);
            for(int i=0; i<num_points;++i){
                Number x_value = (Number)i*x_increment + x_min;
                table->SetValue(i, 0, x_value); // set x axis
                for(int j = 0; j < function_list.size(); ++j){
                    Number f_value = function_list[j].second(x_value);
                    table->SetValue(i, j+1, f_value);
                }
            }
        }

        void setupView(){
            view->GetRenderer()->SetBackground(bg_red/255.0,bg_green/255.0,bg_blue/255.0);
            view->GetScene()->AddItem(chart);

            //chart->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetColor(1.0,1.0,1.0);

            for(int i=0; i<function_list.size();++i) {
                vtkPlot *line = chart->AddPlot(vtkChart::LINE);


                //todo: for loop all the functions, not just first one
#if VTK_MAJOR_VERSION <= 5
                line->SetInput(table, 0, i+1);
#else
                line->SetInputData(table, 0, i+1);
#endif

                float val = std::min(((float)(2*i)/(function_list.size())), (float)1.0);
                std::cout<<"val"<<val<<"\n";
                float sat = std::min(1-(((float)(2*i)/(function_list.size()))-1), (float)1.0);
                std::cout<<"sat"<<sat<<"\n";

                float hue =get_hue_from_rgb(material_red,material_green,material_blue);
                std::array<float, 3> rgb = get_rgb_from_hsv(hue, sat*60+40, val*80+20);
                line->SetColor(color_limit(rgb[0]), color_limit(rgb[1]), color_limit(rgb[2]), 255);
                line->SetWidth(1.0);
                line = chart->AddPlot(vtkChart::LINE);
            }

        }

        void showView(){
            view->GetInteractor()->Initialize();
            view->GetInteractor()->Start();
        }

    private:
        vtkSmartPointer<vtkTable> table;
        vtkSmartPointer<vtkFloatArray> x_axis;

        function_vector function_list;

        vtkSmartPointer<vtkContextView> view;

        float material_red;
        float material_green;
        float material_blue;

        double bg_red;
        double bg_green;
        double bg_blue;

        vtkSmartPointer<vtkChartXY> chart;
    };
}

int main() {

    mvis::Graph<float> graph;

    graph.addFunction("fib", mvis::fib<float>);
    graph.addFunction("x", [](const float& x){return x;});
    graph.addFunction("x^2", [](const float& x){return x*x;});
    graph.addFunction("2x", [](const float& x){return x*2;});
    graph.addFunction("2x^2", [](const float& x){return x*x*2;});

    graph.calcRegion(0, 10, 0.1);

    graph.setupView();
    graph.showView();

    return 0;
}