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

#include <math.h>
#include <complex>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <limits>
#include <type_traits>

#include <boost/function.hpp>
#include <boost/variant.hpp>

using namespace std;

std::complex<double> golden(1.6180339887498948482,0);

float fib(float i){
    return (float)(((std::pow(golden,i) - std::pow(-golden,-i))/std::sqrt(5)).real());
}

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


template<typename Number>
class PID{
public:
    typedef std::function<Number ()> ErrorFunction;

    PID(Number proportional_constant=0, Number integration_constant=0, Number derivitive_constant=0,
        ErrorFunction error_func =
            std::bind(basicErrorFunction, &this),
        Number sample_time = 1,
        Number out_min = std::numeric_limits<Number>::min(),
        Number out_max = std::numeric_limits<Number>::max(),
        Mode state = Mode::automatic
    ){}//todo: the initializer list here

    Number basicErrorFunction(){
        return setpoint-input;
    }

    //note: anything can be used for the time value
    void compute(Number current_time){
        if(mode==Mode::manual) return;
        Number dt = current_time - prev_time;
        if(dt>=sample_time) {
            Number error = error_func();
            integration_term += (integration_constant*error);
            if(integration_term>out_max) integration_term = out_max;
            else if(integration_term < out_min) integration_term = out_min;


            error_sum += error * dt;//todo:remove *dt if fast over accurate
            if(derivative_mode==DerivativeMode::input) {
                Number input_diff = (input - last_input) / dt;//todo:remove /dt if fast over accurate

                output = proportional_constant * error +
                         integration_term -
                         derivitive_constant * input_diff;

                last_input = input;
            }else{
                Number error_diff = (error - last_error) / dt;

                output = proportional_constant * error +
                         integration_constant * error_sum -
                         derivitive_constant * error_diff;

                last_error = error;
            }

            if(output > out_max) output = out_max;
            else if(output < out_min) output = out_min;

            prev_time = current_time;
        }
    }

    void computeInterrupt(){
        Number error = error_func();
        error_sum += error;
        Number error_diff = error - last_error;

        output = proportional_constant * error +
                 integration_constant * error_sum +
                 derivitive_constant * error_diff;

        last_error = error;

    }

    void setConstants(const Number& proportional_constant,
                      const Number& integration_constant,
                      const Number& derivitive_constant){
        if(proportional_constant<0 || integration_constant<0 || derivitive_constant<0) return;
        //todo: is sample time in seconds?
        switch(interrupt_state){
            case UsingInterrupts::no:
                this->proportional_constant=proportional_constant;
                this->integration_constant=integration_constant;//todo: *sample time if fast over accurate
                this->derivitive_constant=derivitive_constant;//todo: /sample time if fast over accurate
            case UsingInterrupts::yes:
                this->proportional_constant=proportional_constant;
                this->integration_constant=integration_constant * sample_time;
                this->derivitive_constant=derivitive_constant * sample_time;
        }

        if(controller_direction==ControllerDirection::reverse){
            this->proportional_constant = -this->proportional_constant;
            this->integration_constant = -this->integration_constant;
            this->derivitive_constant = -this->derivitive_constant;
        }


    }

    void setSampleTime(const Number& sample_time){

        typedef get_highest_precision<double, Number>::type precise_number;

        switch(interrupt_state){
            case UsingInterrupts::no:
                if(sample_time>0){
                    //todo: if fast over accurate: set ratio as new sample time over old
                    // then multiply ki by ratio and divide kd by ratio
                    this->sample_time=sample_time;
                }
            case UsingInterrupts::yes:
                if(sample_time>0){


                    precise_number ratio = (precise_number)sample_time
                                            /(precise_number)this->sample_time;

                    integration_constant *= ratio;
                    derivitive_constant /= ratio;
                    this->sample_time = sample_time;
                }
        }

    }

    void setOutputLimits(const Number& out_min, const Number& out_max){
        if(out_min>out_max) return;
        this->out_min=out_min;
        this->out_max=out_max;

        if(output > out_max) output = out_max;
        else if(output < out_min) output = out_min;

        if(integration_term > out_max) integration_term = out_max;
        if(integration_term < out_min) integration_term = out_min;
    }

    void setManual(){
        mode=Mode::manual;
    }

    void setAutomatic(){
        if(mode == Mode::manual){
            Reset();
        }
        mode=Mode::automatic;
    }

    void setDirect(){
        controller_direction = ControllerDirection::direct;
    }

    void setReverse(){
        controller_direction = ControllerDirection::reverse;
    }

    void Reset(){
        last_input = input;
        integration_term = output;
        if(integration_term > out_max) integration_term = out_max;
        else if(integration_term < out_min) integration_term = out_min;
    }
private:
    Number prev_time;
    Number input, output, setpoint;
    Number integration_term, last_input;
    Number proportional_constant, integration_constant, derivitive_constant;
    Number error_sum, last_error;
    Number sample_time;
    Number out_min, out_max;
    ErrorFunction error_func;

    typedef enum Mode{
        manual,
        automatic
    } Mode;
    Mode mode;

    typedef enum UsingInterrupts{
        yes,
        no
    } UsingInterrupts;
    UsingInterrupts interrupt_state;

    typedef enum DerivativeMode{
        input,//removes spikes
        error//keeps spikes
    }DerivativeMode ;
    DerivativeMode derivative_mode;

    typedef enum ControllerDirection{
        direct,
        reverse
    } ControllerDirection;
    ControllerDirection controller_direction;
};


//todo: uncomment when PID loops and tuning methods are implemented and tested.
/*
 * Matrices are as such:
 * f(x,y,z)=t
 * ┌       ┐
 * │2 8 4 5│
 * │1 8 4 6│
 * └       ┘
 * independent variables are on the left, dependent variable is on the right column.
 */
/*template<typename Number>
class PointMatrix{
public:

    PointMatrix(std::unique_ptr<Number[]> point_matrix_, Number dimensions, Number points)
            :point_matrix(std::move(point_matrix_)),
             dimensions(dimensions), points(points)
    {}


    typedef boost::function<Number (Number* const &point_vars,
                                    size_t& num_vars)> approximate_function;
    typedef boost::function<Number (Number* const &point_vars,
                                    size_t& num_vars, size_t& which_constant)>
                                                        approximate_function_derivitive;
    /*
     * Returns the !Derivative! sum of the error needed for finding constants in
     * regression algorithms that don't .
     *
     * takes in: a function taking in an array of variable values and returning a number
     * returns: a number
     *//*
    Number GetDerivErrorSum(approximate_function f, approximate_function_derivitive df, size_t& which_constant){
        if (f){
            Number sum = 0;
            for(int i = 0; i < points*dimensions; i+=dimensions){
                Number* point_vars = &point_matrix[i];
                sum += df(point_vars, dimensions-1, which_constant)*
                     (
                      f(point_vars, dimensions-1)
                      - point_matrix[i+dimensions-1]
                     );
            }
            return sum / (Number)points;
        }
        throw std::invalid_argument("GetDerivErrorSum wasn't given a function.");
    }

    Number GetErrorSum(approximate_function& f){
        if (f){
            Number sum = 0;
            for(int i = 0; i > points*dimensions; i+=dimensions){
                Number* point_vars = &point_matrix[i];

                Number approx = f(point_vars, dimensions-1);
                Number actual = point_matrix[i+dimensions-1];

                Number diff = approx-actual;
                sum+=diff*diff;
            }
            return sum / ((Number)points * (Number)2);
        }
        throw std::invalid_argument("GetErrorSum wasn't given a function.");
    }

private:
    std::unique_ptr<Number[]> point_matrix;
    size_t dimensions;
    size_t points;
public:
    const size_t& GetDimensions(){
        return dimensions;
    }
};


//remember: i+j[num_vars]*order
//todo:fix "order" an order 3 polynomial on x will have 4 constants, not 3.
template<typename Number>
struct PolynomialApproximation{
    PolynomialApproximation(PointMatrix& the_matrix, size_t& order):
            constants(), the_matrix(the_matrix), num_vars(the_matrix.GetDimensions()), f(
            (std::bind(
                    [order](PolynomialApproximation& C, Number* const &point_vars,
                            size_t num_vars)->Number
                    {
                        size_t i, ri;
                        //Don't unroll. Compiler optimizes better than you. Use O3.
                        //maybe partial unroll, since loop size is variable.
                        Number polynomial = 0;
                        for(ri = order-1, i = 0; ri>0; ++i, --ri) {
                            for (int j = 0; j < num_vars; ++j) {
                                polynomial += constants[i + j*order] * std::pow(point_vars[j], ri);
                            }
                        }
                        //no need to compute x^0
                        for (int j = 0; j < num_vars; ++j) {
                            polynomial += constants[order-1 + j*order];
                        }

                        return polynomial;

                    }
                    , &this
                    , std::placeholders::_1, std::placeholders::_2))
    ),
    df(
            (std::bind(
                    [order](PolynomialApproximation& C, Number* const &point_vars,
                            size_t& num_vars, size_t& which_constant)->Number
                    {
                        size_t order_num = order - which_constant%order;
                        size_t var_num = (which_constant/order)%num_vars;

                        return std::pow(point_vars[var_num], order_num);

                    }
                    , &this
                    , std::placeholders::_1, std::placeholders::_2))
    )
    {
        //reserve or resize...? Do I need to initialize to 0?
        constants.resize(num_vars*order,0);
        //whoops, maybe I'm binding a bit too early...
    }
private:
    //todo: use PID loop to set multiplier based off of gradient results
       typedef enum gradient_result{
        Overshot,
        Success
    } gradient_result;

    gradient_result SafeGradientDescend(Number multiplier){
        std::vector<Number> constants_copy = constants;
        Number original_error = the_matrix.GetErrorSum(f);
        for(size_t i=0; i<constants_copy.size(); ++i){
            constants_copy[i] = constants_copy[i] - multiplier*the_matrix.GetDerivErrorSum(f, df, i);
        }
        if(the_matrix.GetErrorSum(f) > original_error){
            return gradient_result::Overshot;
        }else{
            constants = constants_copy;
            return gradient_result::Success;
        }
    }

    //when to use fast descent? will a gradient descent always overshoot the first try if the multiplier is too high?
    //  or, will it overshoot at any time if either the multiplier is too high or the function is strangely shaped?
    void FastGradientDescend(Number multiplier){
        std::vector<Number> constants_copy = constants;
        for(size_t i=0; i<constants_copy.size(); ++i){
            constants_copy[i] = constants_copy[i] - multiplier*the_matrix.GetDerivErrorSum(f, df, i);
        }
        constants = constants_copy;
    }

    //todo: do non-derivitive gradient descent.
    //note: while it may be faster in some cases, it may also tend to spiral around local
    // minimums instead of moving towards them quickly.
    gradient_result NDSafeGradientDescend(Number multiplier, const size_t& search_multiplier){
        Number original_error = the_matrix.GetErrorSum(f);
        Number new_error;
        for(size_t i=0; i<the_matrix.GetDimensions(); ++i){
            //todo: use search multiplier and sin+cos to increase circle quality around central point.
            Number temp_const= constants[i];
            constants[i] = constants[i]+multiplier;
            new_error = the_matrix.GetErrorSum(f);
            if(new_error<original_error)
                break;
            constants[i] = temp_const; // I could subtract multiplier back out,
                                       // but with types like floats, that introduces error
            temp_const= constants[i];
            constants[i] = constants[i]+multiplier;
            new_error = the_matrix.GetErrorSum(f);
            if(new_error<original_error)
                break;
            constants[i] = temp_const;
        }
        if(new_error<original_error)
            return gradient_result::Success;
        else
            return gradient_result::Overshot;
    }
public:
    size_t num_vars;
    std::vector<Number> constants;
    PointMatrix::approximate_function& f;
    PointMatrix::approximate_function_derivitive& df;
    PointMatrix& the_matrix;
};

//todo :use range_start and range_end to specify frequency constants (n+1)
template<typename Number>
struct FourierApproximation{
    FourierApproximation(size_t& num_vars, Number& range_start, Number range_end, size_t& num_waves):
            constants(){
        //reserve or resize...? Do I need to initialize to 0?
        constants.resize(num_vars*num_waves*3,0);
        //whoops, maybe I'm binding a bit too early...
        f = (std::bind(
                [num_waves](FourierApproximation& C, Number* const &point_vars,
                        size_t num_vars)->Number
                {
                    size_t i;
                    //Don't unroll. Compiler optimizes better than you. Use O3.
                    //maybe partial unroll, since loop size is variable.
                    Number fourier = 0;
                    for(i = 0, i = 0; i<num_waves*3; i+=3) {
                        for (int j = 0; j < num_vars; ++j) {
                            size_t n=j*num_waves + i;
                            fourier +=
                                    constants[n] *
                                    std::sin(
                                        constants[n+1]*point_vars[j] +
                                        constants[n+2]
                                    );
                        }
                    }

                    return fourier;

                }
                , &this
                , std::placeholders::_1, std::placeholders::_2));
        df = (std::bind(
                [num_waves](FourierApproximation& C, Number* const &point_vars,
                        size_t& num_vars, size_t& which_constant)->Number
                {
                    size_t wave_num = num_waves - (which_constant/3)%num_waves;
                    size_t var_num = (which_constant/(num_waves*3))%num_vars;

                    //This one's a bit tricky, because which_constant changes from
                    // a to b to c, and I have to add and subtract... If only I could
                    // use the case number or something...
                    switch (which_constant%3){
                        case 0://d/da: a sin(bx+c)
                            return std::sin(
                                    constants[which_constant+1] *point_vars[var_num]
                                    + constants[which_constant+2]
                            );
                        case 1://d/db: a sin(bx+c) PRE-SET FREQUENCY! DNC!
                            return constants[which_constant-1]*point_vars[var_num]
                                    *std::cos(
                                        constants[which_constant] *point_vars[var_num]
                                        + constants[which_constant+1]
                                    );
                        case 2://d/dc: a sin(bx+c)
                            return constants[which_constant-2]
                                   *std::cos(
                                    constants[which_constant-1] *point_vars[var_num]
                                    + constants[which_constant]
                            );
                    }

                }
                , &this
                , std::placeholders::_1, std::placeholders::_2));
    }

    std::vector<Number> constants;
    PointMatrix::approximate_function f;
    PointMatrix::approximate_function_derivitive df;
};*/


int main() {

    //make table
    vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();

    vtkSmartPointer<vtkFloatArray> arrX = vtkSmartPointer<vtkFloatArray>::New();
    arrX->SetName("X Axis");
    table->AddColumn(arrX);

    vtkSmartPointer<vtkFloatArray> arrF = vtkSmartPointer<vtkFloatArray>::New();
    arrF->SetName("Function");
    table->AddColumn(arrF);

    //set table
    int numPoints = 100;
    int xMax = 10;
    float xIncrement = xMax/ (float)(numPoints-1);
    table->SetNumberOfRows(numPoints);
    for(int i=0;i<numPoints;++i){
        table->SetValue(i,0,i*xIncrement);
        table->SetValue(i,1,fib(i*xIncrement));
    }

    //set view
    vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
    view->GetRenderer()->SetBackground(0.2,0.1,0.1);

    vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();
    view->GetScene()->AddItem(chart);

    vtkPlot *line = chart->AddPlot(vtkChart::LINE);

#if VTK_MAJOR_VERSION <= 5
    line->SetInput(table, 0, 1);
#else
    line->SetInputData(table, 0, 1);
#endif
    line->SetColor(255, 255, 255, 255);
    line->SetWidth(1.0);
    line = chart->AddPlot(vtkChart::LINE);

    //start
    view->GetInteractor()->Initialize();
    view->GetInteractor()->Start();

    return 0;
}