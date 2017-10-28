# 3 Moving to Modern C++

 * Modern C++ 에서 중요한 것
    * auto
    * smart pointers
    * move semantics
    * lamdas
    * concurrency


## Item7 : Distinguish () and {} when creating objects.
creating object
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

어디서든 single initialization syntax 로 표현할 수 있음
```cpp
    std::vector<int> v{1, 3, 5};
```

non-static member data에도 쓸 수 잇음 ( parentheses 는 안됨)
```cpp
    class Widget{
    private:
        int x{0};
        int y = 0;
        int z(0); //error
    }
```
uncopyable object 는 braces, parentheses 로만 가능 ( '='는 안됨)(eg.std::atomic)
```cpp
    std::atomic<int> ai1{0};
    std::atomic<int> ai2(0);
    std::atomic<int> ai3 = 0; //error
```
 * 즉, braces 만 모든 초기화에 사용할 수 있다.
    * built-in type 에 대해 암시적 narrowing conversions을 막는 기능이 추가됨
        * parentheses 랑 '=' 는 narrowing conversion 을 체크하지 않음.
            * 기존 레거시 코드에 영향을 미치므로.
    ```cpp
        double x,y,z;
        int sum1{x+y+z}; //error sum of doubles may not be expressible as int
        int sum2( x+ y+ z);     //okay
        int sum3 = x + y + z;   //ditto
    ```
    * immunity to C++'s most vexing parse
        * C++의 룰에 따른 side effect
            * 모든 것은 declaration 된것로 parsing 될 수 있다

vexing parse 예제
```cpp
        Widget w1(10);  //call Widget ctor with arg 10

        Widget w2(); //most vexing parse! declares a function named w2 that returns a Wdget
        // Widget을 리턴하는 함수인지 헷갈린다는 말씀!

        Widget w3{}; // 이렇게 하면 헷갈릴 일이 없지!
```

 * 그래도 braced initialize의 단점이 있어
    * std::initializer_list
        * [Item2] 에서 말한 것 처럼, auto를 좋아할 수록 braced initializer랑 멀어질거야.
    * constructor overload resolution
        * constructor 가 parameter로 initializer_list 를 받을 때, 컴파일러는 initializer_list를 선호한다. Strongly!

braced constructor 잘 되는 케이스
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
```

braced constructor 가 initializer_list를 선호하는 케이스
```cpp

    class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d); // 더블디와 함께한 밤 에블바리 모두 출석 쳌
        Widget(std::initializer_list<long double> il);// 추가된 선언
    };

    Widget w1(10, true);    // first ctor
    Widget w2{10, true};    // call std::initializer_list ctor : 10, true를 long double로 캐스팅
    Widget w3(10, 5.0);     // second ctor
    Widget w4{10, 0};       // call std::initializer_list ctor : 10, 5.0를 long double로 캐스팅
```

copy & move construction can be hijacked by std::initializer_list
```cpp
    class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d);
        Widget(std::initializer_list<long double> il);//added
        operator float() const;
    };
    Widget w1(10, true);
    Widget w2{10, true};
    Widget w3(10, 5.0);
    Widget w4{10, 0};
    Widget w5(w4);
    Widget w6{w4};              // calls std::initializer_list cotr, w4->float->double implicitly converted
    Widget w7(std::move(w4));
    Widget w8{std::move(w4)};
```

Specifing other type in std::intializer_list is also fault.
```cpp
    class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d);
        Widget(std::initializer_list<bool> il);  //not double or int,
        operator float() const;
    };
    Widget w{10, 5.0}; //error! requires narrowing conversions
```

 * 하지만 여전히 에러
    * int 랑 double 은 bool 로 conversion 될 수 있음. 그러면 compiler 는 initializer_list를 우선적으로 요청, 암시적 conversion이 일어남.
    * conversion안되는 타입으로 선언하는 수 밖에 없음
    ```cpp
    Widget(std::initializer_list<std::string> il);
    Widget w1(10, true);    //first ctor
    Widget w2{10, true};    //first ctor
    Widget w3(10, 5.0);     //second ctor
    Widget w4{10, 0};       //second ctor
    ```

끝난줄 알았지? ㅋ 아직 하나의 edge case가 남았어
```cpp
class Widget{
public:
    Widget();
    Widget(std::initializer_list<int> il);
};
    Widget w1;      //default ctor
    Widget w2{};    //default ctor
    Widget w3();    // ? vexing parse
```
 * 너는 빈 list 를 넣고 싶었어도, default ctor를 부른다

이런 경우는 braces 를 parentheses나 braces 안에 넣어야함.
```cpp
    Widget w4({});
    Widget w5{{}};
```

이게 얼마나 코딩에 영향 미치겠어? 간과하지마
```cpp
    std::vector<int> v1(10, 20); // 20을 가진 element 10개
    std::vector<int> v2{10, 20); // element 10, 20 두개 뿐
```
 * [author] constructor overloading 결론
    * constructor overloading 을 할 때, braced, parentheses initializer가 다른 기능을 하게 하지마.
        * std::vector 처럼 하지마! std 구려!
    * std::initializer_list 쓰지 않는경우도 만들어 놓긴 해야해
        * 사용자가 braced intialize방법을 쓰면 resolve 시켜줘야 하니까
    * std::initializer_list constructor overload는 제대로 생각하고 great deliberation 하도록 만드셈

 * [client] choose carefully between parentheses and braces when creating objects.
    * braces : 범용적이야
    * parentheses : braces가 std::initializer_list로 인해 갖는 3가지 문제를 피할 수 있어. 기존 스타일(C++98)과도 맞아.
    * 선택하셈

Template author라면 arbitrary num of args 로부터 arbitrary object를 생성하는 상황을 보면 애매해
```cpp
    template<typename T, typename... Ts>
    void doSomework(Ts&&... params0) { //create local T object from params...}
```
localObject는
```cpp
    T localObject(std::ofrward<Ts>(params)...);
    std::vector<int> v;
    doSomeWork<std::vector<int>>(10,20);
```