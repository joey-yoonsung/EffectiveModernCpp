# 3 Moving to Modern C++

 * Modern C++ 에서 중요한 것
    * auto
    * smart pointers
    * move semantics
    * lamdas
    * concurrency


## Item7 : Distinguish () and {} when creating objects.
 * creating object
```cpp
    int x(0);
    int y = 0;
    int z{0};
    int z = {0};
```
```cpp
    Widget w1;      // call default constructor
    Widget w2 = w1; // not an assignment; call copy constructor
    w1 = w2;        // an assignment; call copy operator=
```
 * C++98 에는 intialize 하기힘든 케이스가 몇개 있어 (stl container가 초기값으로 몇개 가지고 initialize되는 케이스 등)
    * C++11 uniform initialization (= braced initialization)
        * 어디서든 single initialization syntax 로 표현할 수 있음
    ```cpp
    std::vector<int> v{1, 3, 5};
    ```
 * non-static member data에도 쓸 수 잇음 ( parentheses 는 안됨)
    ```cpp
    class Widget{
    private:
        int x{0};
        int y = 0;
        int z(0); //error
    }
    ```
 * uncopyable object 는 braces, parentheses 로만 가능 ( '='는 안됨)(eg.std::atomic)
    ```cpp
    std::atomic<int> ai1{0};
    std::atomic<int> ai2(0);
    std::atomic<int> ai3 = 0; //error
    ```
 * 즉, braces 만 모든 초기화에 사용할 수 있다.
    * built-in type 에 대해 암시적 narrowing conversions을 막는 기능이 추가됨
    ```cpp
        double x,y,z;
        int sum1{x+y+z}; //error sum of doubles may not be expressible as int
        int sum2( x+ y+ z);     //okay
        int sum3 = x + y + z;   //ditto
    ```
    * immunity to C++'s most vexing parse
        * C++의 룰에 따른 side effect
            * 모든 것은 declaration 된것로 parsing 될 수 있다
    ```cpp
    class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d); // 더블디와 함께한 밤 에블바리 모두 출석 쳌
    };
    Widget w1(10, true);
    Widget w2{10, true};
    Widget w3(10, 5.0);
    Widget w4{10, 0};

    Widget(std::initializer_list<long double> il);//added
    ```
    ```cpp
    class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d); // 더블디와 함께한 밤 에블바리 모두 출석 쳌
        Widget(std::initializer_list<long double> il);//added
    };
    Widget w1(10, true);
    Widget w2{10, true};
    Widget w3(10, 5.0);
    Widget w4{10, 0};       // uses braces, but now calls std::initializer_list constructor
    ```
    ```cpp
    class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d); // 더블디와 함께한 밤 에블바리 모두 출석 쳌
        Widget(std::initializer_list<long double> il);//added
        operator float() const;
    };
    Widget w1(10, true);
    Widget w2{10, true};
    Widget w3(10, 5.0);
    Widget w4{10, 0};       // uses braces, but now calls std::initializer_list constructor
    Widget w5(w4);
    Widget w6{w4};
    Widget w7(std::move(w4));
    Widget w8{std::move(w4)};
    ```