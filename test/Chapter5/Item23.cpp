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

class Widget{

};
const string process(const Widget& lvalArg){
    cout <<"process lvalue" << endl;
    return "lvalue";
}
const string process(Widget&& rvalArg){
    cout <<"process rvalue" << endl;
    return "rvalue";
}

template<typename T>
const string logAndProcess(T&& param){
    return process(forward<T>(param));
}

TEST_CASE("Item5", "[effective][move][forward][deduction]") {

    GIVEN("where encoding ?") {
        Widget w;

        CHECK(logAndProcess(w) == "lvalue");
        CHECK(logAndProcess(std::move(w)) == "rvalue");
    }
}
