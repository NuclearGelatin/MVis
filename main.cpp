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

template<class Number, class Number2>//compilers often change numbers to
Number range_limit(const Number& n, const Number2& min, const Number2& max){
    if(n>max)
        return max;
    if(n<min)
        return min;
    return n;
}


//todo: make the function plotter a subclass of this class and make this class only be a parent.
namespace mvis{
    template <class Number>
    class Graph{
    public:
        Graph(const char* x_axis_label = "X Axis"):
                table(vtkSmartPointer<vtkTable>::New()),
                x_axis(vtkSmartPointer<vtkFloatArray>::New()),
                view(vtkSmartPointer<vtkContextView>::New()),
                chart(vtkSmartPointer<vtkChartXY>::New())
        {
            x_axis->SetName(x_axis_label);
            table->AddColumn(x_axis);

            bg_color[0] = 1.0;
            bg_color[1] = 0.95;
            bg_color[2] = 0.9;

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
            view->GetRenderer()->SetBackground(bg_color[0],bg_color[1],bg_color[2]);
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

                float brightness = (float)(i*255)/function_list.size();

                float material_red = 255;
                float material_green = 127;
                float material_blue = 12;

                double red = brightness /
                             (0.30 + 0.59*(material_green/material_red) + 0.11*(material_blue/material_red));
                double green = brightness /
                               (0.30*(material_red/material_green) + 0.59 + 0.11*(material_blue/material_green));
                double blue = brightness /
                              (0.30*(material_red/material_blue) + 0.59*(material_green/material_blue) + 0.11);

                line->SetColor(color_limit(red), color_limit(green), color_limit(blue), 255);
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
    private:

        function_vector function_list;

        vtkSmartPointer<vtkContextView> view;
        float bg_color[3];



        vtkSmartPointer<vtkChartXY> chart;
    };
}

int main() {

    mvis::Graph<float> graph;

    graph.addFunction("fib", mvis::fib<float>);
    graph.addFunction("x", [](const float& x){return x;});
    graph.addFunction("x^2", [](const float& x){return x*x;});
    graph.addFunction("15log2x", [](const float& x){return 15*std::log2(x);});
    graph.addFunction("50sin x", [](const float& x){return 50*std::sin(x);});
    graph.addFunction("5/x", [](const float& x){return 5/x;});
    graph.addFunction("50cos x", [](const float& x){return 50*std::cos(x);});

    graph.calcRegion(0, 10, 0.1);

    graph.setupView();
    graph.showView();

    return 0;
}