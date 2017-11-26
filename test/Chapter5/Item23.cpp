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

template <typename S>
std::string getReturn(S&& param){
    std::string local("b");
    cout << (int*)&local << endl;
    return std::forward<std::string>(local);
}

template <typename S>
std::string getMoveReturn(S&& param){
//    std::string&& local //= std::forward<std::string>(param);
    std::string local("B"); //= std::forward<std::string>(param);
    cout << (int*)&local << endl;
    return std::move(local);
}

TEST_CASE("Item5", "[effective][move][forward][deduction]") {

    GIVEN("where encoding ?") {
        Widget w;

        CHECK(logAndProcess(w) == "lvalue");
        CHECK(logAndProcess(std::move(w)) == "rvalue");
    }

    GIVEN(" forward return"){
        std::string input("a");
        cout << (int*) &(input) << endl;
        auto val = getReturn(input);
        cout << (int*) &val<< endl;

        WHEN("move return"){
            auto mval = getMoveReturn(input);
            CHECK(mval == "B");
            cout << mval << endl;
            cout << input << endl;
            cout << (int*)&mval << endl;

        }

    }
    GIVEN("perfect forwarding failure")
}


