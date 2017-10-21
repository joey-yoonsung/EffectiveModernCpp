//
// Created by yoonsung-mac on 21/10/2017.
//
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <vector>
#include <climits>

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
}

TEST_CASE("Item1", "[effective][template][deduction]") {

    GIVEN("Case1"){
        string x(ORIGIN);
        WHEN("param is int "){
            string param = x;
            THEN("param is 1"){
                CHECK(param == ORIGIN);
                f(param);
                CHECK(param == CHANGE);
            }
        }
        WHEN("param is const int "){
            const string param = x;
            THEN("runtime error"){
                CHECK(param == ORIGIN);
                f(param);
                CHECK(param == CHANGE);
            }
        }
        WHEN("param is const int& "){
            const string& param = x;
            THEN("runtime error"){
                CHECK(param == ORIGIN);
                f(param);
                CHECK(param == CHANGE);
            }
        }
    }
}
