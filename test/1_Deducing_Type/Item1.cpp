//
// Created by yoonsung-mac on 21/10/2017.
//
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <vector>
#include <climits>
#include <deque>
using namespace std;

class Temp{
public:
    Temp(const string &value) : value(value) {}

private:
    string value;
public:
    string getValue() const {
        return value;
    }

    void setValue(const string value) {
        Temp::value = value;
    }

};

static const string ORIGIN("ORIGIN");
static const string CHANGE("CHANGE");

template<typename T >
void f(T& param){
//    param = (Temp) param;
//    param.setValue(CHANGE);
    Temp a = param;
    a.setValue(CHANGE);
//    cout <<typeid(param).name() << endl;
//    if(typeid(param) == typeid(int)){
//        int a = static_cast<int>(param);
//    }
}
template<typename Container, typename Index>
auto authAndAccess(Container& c, Index i)-> decltype(c[i]){ //C++14 don't need '->'
    return c[i];
}

class Widget{
public:
    Widget(int a, double d){
      cout << "int, double"<< endl;
    }
    Widget(std::initializer_list<double> li){
        cout << "list"<< endl;
    }
};

TEST_CASE("Item1", "[effective][template][deduction]") {

    GIVEN("Case1"){
        string x(ORIGIN);
//        WHEN("param is int "){
//            string param = x;
//            THEN("param is 1"){
//                CHECK(param == ORIGIN);
//                f(param);
//                CHECK(param == CHANGE);
//            }
//        }
//        WHEN("param is const int "){
//            const string param = x;
//            THEN("runtime error"){
//                CHECK(param == ORIGIN);
//                f(param);
//                CHECK(param == CHANGE);
//            }
//        }
//        WHEN("param is const int& "){
//            const string& param = x;
//            THEN("runtime error"){
//                CHECK(param == ORIGIN);
//                f(param);
//                CHECK(param == CHANGE);
//            }
//        }
    }
    GIVEN("auto"){
        std::deque<int> d;
        d.push_back(1);
        d.push_back(2);
        authAndAccess(d, 1) = 10;
        CHECK(d[1]==10);
    }

    GIVEN("brace"){
        WHEN("normal new"){
            Widget a{1, 2};
            Widget b(1, 2);

        }
        WHEN("smart ptr"){

            shared_ptr<Widget> ptr = make_shared<Widget>(1,2);
            shared_ptr<Widget> ptr2 = make_shared<Widget>(std::initializer_list<double>{1.0, 2.0});

            double d = NULL;
        }
    }
}
