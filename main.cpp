#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>

#include <vtkVersion.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

#include <vtkAxis.h>
#include <vtkTextProperty.h>
#include <vtkChartLegend.h>
#include <vtkChartXY.h>

#include <vtkTable.h>
#include <vtkPlot.h>
#include <vtkFloatArray.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
//#include <vtkPen.h>

#include "MathFunctions.hpp"

#include <boost/any.hpp>
#include <boost/variant.hpp>

#include "utility.hpp"
#include "color.hpp"
//#include "extra_printers.hpp"

namespace mvis{

    //subclass of chartXY so I can do more stuff
    class ChartXY : public vtkChartXY{
    public:
        vtkTypeMacro(ChartXY, vtkChartXY);

        static ChartXY *New(){
            return new ChartXY();
        }

        vtkChartLegend* GetLegend(){
            return Legend.Get();
        }

        void SetAxisTitle(int vtk_axis, const vtkStdString &title){
            if(GetAxis(vtk_axis)){
                GetAxis(vtk_axis)->SetTitle(title);
            }
        }

        void SetAxisColor(int vtk_axis, double r, double g, double b){
            if(GetAxis(vtk_axis)) {
                GetAxis(vtk_axis)->GetLabelProperties()->SetColor(r, g, b);//the axis numbers
                GetAxis(vtk_axis)->GetTitleProperties()->SetColor(r, g, b);//the axis title
            }
        }

        void SetTextColor(double r, double g, double b){
            GetLegend()->GetLabelProperties()->SetColor(r, g, b);//the legend with te function names
            if(GetAxis(vtkAxis::LEFT)) {
                GetAxis(vtkAxis::LEFT)->GetLabelProperties()->SetColor(r, g, b);//the axis numbers
                GetAxis(vtkAxis::LEFT)->GetTitleProperties()->SetColor(r, g, b);//the axis title
            }
            if(GetAxis(vtkAxis::BOTTOM)) {
                GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetColor(r, g, b);//the axis numbers
                GetAxis(vtkAxis::BOTTOM)->GetTitleProperties()->SetColor(r, g, b);//the axis title
            }
            if(GetAxis(vtkAxis::RIGHT)) {
                GetAxis(vtkAxis::RIGHT)->GetLabelProperties()->SetColor(r, g, b);//the axis numbers
                GetAxis(vtkAxis::RIGHT)->GetTitleProperties()->SetColor(r, g, b);//the axis title
            }
            if(GetAxis(vtkAxis::TOP)) {
                GetAxis(vtkAxis::TOP)->GetLabelProperties()->SetColor(r, g, b);//the axis numbers
                GetAxis(vtkAxis::TOP)->GetTitleProperties()->SetColor(r, g, b);//the axis title
            }
            if(GetAxis(vtkAxis::PARALLEL)) {
                GetAxis(vtkAxis::PARALLEL)->GetLabelProperties()->SetColor(r, g, b);//the axis numbers
                GetAxis(vtkAxis::PARALLEL)->GetTitleProperties()->SetColor(r, g, b);//the axis title
            }

        }
    };

    //todo: make the function plotters a subclass of this class and make this class only be a parent. Also share chart.
    template <class Number>
    class Graph{

        typedef std::function< Number(Number& x)>function_type;

        typedef std::pair<
                vtkSmartPointer<vtkFloatArray>,
                function_type
        > function_pair;

        typedef std::vector<function_pair> function_vector;

        typedef std::pair<function_vector, double > function_group;

        typedef std::vector<function_group> function_groups;

    public:
        Graph():
                table(vtkSmartPointer<vtkTable>::New()),
                x_axis(vtkSmartPointer<vtkFloatArray>::New()),
                view(vtkSmartPointer<vtkContextView>::New()),
                chart(vtkSmartPointer<mvis::ChartXY>::New())
        {
            x_axis->SetName("x axis");//vtk segfaults if there's not a name
            table->AddColumn(x_axis);

            setBGColor(17,17,17);
        }

        void setBottomAxisTitle(const char* title){
            chart->SetAxisTitle(vtkAxis::BOTTOM, title);
        }

        void setLeftAxisTitle(const char* title){
            chart->SetAxisTitle(vtkAxis::LEFT, title);
        }

        void setBGColor(const double& r, const double& g, const double& b){
            bg_red=r;
            bg_green=g;
            bg_blue=b;

            double hue = mvis::color::get_hue_from_rgb(r,g,b);
            double lum = mvis::color::get_luma_from_rgb(r,g,b);

            //todo: set up a scroll bar that allows for changing color and find the best values there

            if(lum>=170){
                chart->SetTextColor(0.0, 0.0, 0.0);
            }
            else if(lum<170 && lum>85){
                std::array<double, 3> text_rgb = mvis::color::get_rgb_from_hsv(hue,(double)100.0,(double)50.0);
                chart->SetTextColor(text_rgb[0], text_rgb[1], text_rgb[2]);
            }else{//lum<=85
                chart->SetTextColor(1.0, 1.0, 1.0);
            }
        }

        double _next_hue(const size_t& i){
            return mvis::util::ith_middle(i, 0.0, 360.0);
        }

        void addFunction(const char* function_name,
                         function_type function, const size_t& group = (size_t)-1){
            vtkSmartPointer<vtkFloatArray> column = vtkSmartPointer<vtkFloatArray>::New();
            column->SetName(function_name);
            table->AddColumn(column);

            if(function_group_list.size()>0){
                if(function_group_list.size()>group && group!=(size_t)-1){
                    function_group_list[group].first.push_back(function_pair(column, function));
                }else{
                    //todo: send out of bounds error unless -1 was sent
                    function_group_list.back().first.push_back(function_pair(column, function));
                }
            }else{
                double hue = _next_hue(0);
                function_vector fvec;
                function_group_list.push_back(function_group(fvec, hue));
                function_group_list.back().first.push_back(function_pair(column, function));
            }
        }

        void addFunctionGroup(const double& hue = -1){
            function_vector fvec;
            if(hue<-0.1){
                function_group_list.push_back(function_group(fvec, _next_hue(function_group_list.size())));
            }else{
                function_group_list.push_back(function_group(fvec, hue));
            }
        }

        void calcRegion(const size_t& x_min, const size_t& x_max, const Number& x_increment){
            if(x_min>x_max) return;

            size_t num_points = (Number)(x_max - x_min) / x_increment;
            table->SetNumberOfRows(num_points);
            for(int i=0; i<num_points;++i){
                Number x_value = (Number)i*x_increment + x_min;
                table->SetValue(i, 0, x_value); // set x axis
                int l=0;
                for(int j=0; j< function_group_list.size(); ++j){
                    for(int k=0; k<function_group_list[j].first.size(); ++k, ++l){
                        Number f_value = function_group_list[j].first[k].second(x_value);
                        table->SetValue(i, l+1, f_value);
                    }
                }
            }
        }

        void setupView(){
            view->GetRenderer()->SetBackground(bg_red/255.0,bg_green/255.0,bg_blue/255.0);
            view->GetScene()->AddItem(chart);

            //chart->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetColor(1.0,1.0,1.0);

            int k=0;
            for(int i=0; i<function_group_list.size();++i) {
                int c=0;
                for(int j=0; j<function_group_list[i].first.size();++j, ++k){
                    vtkPlot *line = chart->AddPlot(vtkChart::LINE);

#if VTK_MAJOR_VERSION <= 5
                    line->SetInput(table, 0, k+1);
#else
                    line->SetInputData(table, 0, k+1);
#endif

                    //todo: make this its own function
                    double hue, sat, val;
                    do {
                        double mid = mvis::util::ith_middle(j + c, 0.0, 160.0);
                        hue = function_group_list[i].second;
                        val = std::max(std::min(mid, 100.0), 20.0);
                        sat = 100.0 - std::max(mid - 80.0, 0.0);
                        std::cout<<"val:"<<val<<"\n";
                        std::cout<<"sat:"<<sat<<"\n";
                        std::array<double, 3> rgb = mvis::color::get_rgb_from_hsv(hue, sat, val);
                        //std::cout<<"rgb:"<<rgb<<"\n";
                        double lum_fg = mvis::color::get_luminance_from_rgb(rgb[0]/255.0,rgb[1]/255.0,rgb[2]/255.0);
                        double lum_bg = mvis::color::get_luminance_from_rgb(bg_red/255.0, bg_green/255.0, bg_blue/255.0);

                        std::cout<<"lum_fg:"<<lum_fg<<"\n";
                        std::cout<<"lum_bg:"<<lum_bg<<"\n";
                        double ratio=0;
                        if(lum_fg >= lum_bg){
                            ratio = (lum_fg+.05) / (lum_bg + .05);
                        } else{
                            ratio = (lum_bg + .05) / (lum_fg + .05);
                        }
                        std::cout<<"ratio: "<<ratio<<"\n";

                        if(ratio<3.0){
                            ++c;
                        }else{
                            break;
                        }

                    }while(true);

                    std::array<double, 3> rgb = mvis::color::get_rgb_from_hsv(hue, sat, val);

                    line->SetColor((unsigned char)mvis::color::_255_limit(rgb[0]),
                                   (unsigned char)mvis::color::_255_limit(rgb[1]),
                                   (unsigned char)mvis::color::_255_limit(rgb[2]), 255);
                    line->SetWidth(1.0);
                    line = chart->AddPlot(vtkChart::LINE);
                }
            }
        }

        void showView(){
            view->GetInteractor()->Initialize();
            view->GetInteractor()->Start();
        }

    private:
        vtkSmartPointer<vtkTable> table;
        vtkSmartPointer<vtkFloatArray> x_axis;

        function_groups function_group_list;

        vtkSmartPointer<vtkContextView> view;

        double bg_red;
        double bg_green;
        double bg_blue;

        vtkSmartPointer<mvis::ChartXY> chart;
    };
}

int main() {

    mvis::Graph<float> graph;

    graph.addFunctionGroup(0.0);
    graph.addFunction("fib", mvis::fib<float>);
    graph.addFunction("x", [](const float& x){return x;});
    graph.addFunction("x^2", [](const float& x){return x*x;});
    graph.addFunctionGroup(60.0);
    graph.addFunction("2x", [](const float& x){return x*2;});
    graph.addFunction("2x^2", [](const float& x){return x*x*2;});

    graph.calcRegion(0, 10, 0.1);

    graph.setupView();
    graph.showView();

    return 0;
}