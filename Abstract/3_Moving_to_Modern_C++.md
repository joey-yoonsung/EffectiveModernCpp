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
이렇게 쓴다고 하면.
```cpp
    T localObject(std::forward<Ts>(params)...);
    std::vector<int> v;
    doSomeWork<std::vector<int>>(10,20);
```
 * 문제
    * parentheses 를 쓰면 10인 element 20개가 되고
    * braces 를 쓰면 element 가 2개가 됨.
    * 근데 이게 진짜 문제가 되는 거는 smart pointer(std::make_unique, std::make_shared) 쓸 때

### 요약
 1. Braced initialization hte most widely usable initialization syntax, it prevents narrowing conversions, and it's immune to C++'s most vexing parse.
 2. During constructor overload resolution, braced initializers are matched to std::initializer_list parameters if at all possible, even if other constructors offer seemingly better matches.
 3. An example of where the choice between parentheses and braces can make a significant difference is creating a std::vector<numeric type> with two arguments.
 4. Choosing between parentheses and braces for object creation inside templates can be challenging.


## Item8: Prefer nullptr to 0 and NULL.

 * 주의 : 0은 원래 int 이지만, pointer에 대고 하면 nullptr을 의미한다.
 * 주의 : NULL이 주는 불확실함이 있어.
    * int 가 아닌 integral type은 NULL할당이 가능

C++98에서 pointer 와 integral type의 overloading 할때, pointer overload가 절대 불리지 않아.
```cpp
    void f(int);
    void f(bool);
    void f(void*);
    f(0);
    f(NULL); // f(int)를 부름.
```
이런 문제에도 계속 NULL과 0을 쓴다...

nullptr의 원리
 * 모든 타입의 포인터로 본다. (integer로 안봄)
    * std::nullptr_t
        * implicit conversion to all raw pointer types.
```cpp
    f(nullptr); //call void f(void*)
```
overload 문제 해결 + code clarity 향상
```cpp
    auto result = findRecord(/*args*/);
    if(result == 0 ){...}       // (1)
    if(result == nullptr ){...} // (2)
```

nullptr을 써야하는 이유 when use template
```cpp
    int f1(std::shared_ptr<Widget> spw);
    double f2(std::unique_ptr<Widget> upw);
    bool f3(Widget* pw);
    template<typename FuncType, typename MuxType, typename PtrType>
    auto lockAndCall(FuncType func, MuxType& mutex, PtrType ptr) -> decltype(func(ptr))
    {
        MuxGuard g(mutex);
        return func(tpr);
    }

    auto result1 = lockAndCall(f1, f1m, 0);         //error
    auto result2 = lockAndCall(f2, f2m, NULL);      //error
    auto result3 = lockAndCall(f3, f3m, nullptr); //fine
```
 * template, auto type deducing 에서 shared_ptr, unique_ptr로 추론했는데, int가 입력되어 있어서 컴파일 에러.

### 요약
 1. Prefer nullptr to 0 and NULL.
 2. Avoid overloading on integral and pointer types.


## Item 9: Prefer alias declarations to typedefs.
typedefs are soooo C++98!!!

typedef는 터널증후근 위험을 만들어!! ㅋㅋㅋㅋㅋ
```cpp
    typedef std::unique_ptr<std::unordered_map<std::string, std::string>> UPtrMapSS;
    using  UPtrMapSS = std::unique_ptr<std::unordered_map<std::string, std::string>>;
```

Function Pointer선언을 보면 alias가 더 편하고 직관적인 것을 알 수 있어.
```cpp
    typedef void (*FP)(int, const std:string&);
    using FP= void (*)(int, const std::string&);
```

template 의 경우는 typedef 를 선호할수도 있다?
 * alias 는 templatized 가능 (alias templates), typedef는 불가능.
    * struct 안에 nested 된 typedef 로 C++98에서는 함.

alias templates
```cpp
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;
```

C++98's struct nested typedefs
```cpp
template<typename T>
struct MyAllocList{
    typedef std::list<T, MyAlloc<T>> type;
};
```

typedef inside template
 * MyAllocList<T> 는 dependent type이다.
 * dependent type 은 앞에 typename 이 앞에 와야한다
 * ::type 으로 실제 타입을 받을 수 있다.
```cpp
template<typename T>
class Widget{
private:
    typename MyAllocList<T>::type list;
};
```

alias inside template
 * non-dependent type이다.
 * 일반 alias 선언을 해놓았다면 그냥 쓸 수 있다.
```cpp
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;

template<typename T>
class Widget{
private:
    MyAllocList<T> list;
};
```

type-traits ::type
 * const, & 벗기고 T만 받고 싶을 때
 * &를 붙이고 싶을 때
 * 전부 제공 하는건 아니지만, predictable interface 임
```cpp
    std::remove_const<T>::type      //T from const T
    std::remove_reference<T>::type  // T from T7 and T&&
    std::add_lvalue_referenece<T>::type // T& from T
```

C++14에서 std:;transformation_t 를 추가함. 근데 C++11의 std::transformation::type 이랑 사실상 같아서 의미없음
```cpp
    std::remove_const_t<T>            // T from const T
    std::remove_reference_t<T>        // T from T7 and T&&
    std::add_lvalue_referenece_t<T>   // T& from T
```

정리
 1. typedefs don't support templatization, but alias declarations do.
 2. Alias templates avoid the "::type" suffix and, in templates, the "typename" prefix often required to refer to typedefs.
 3. C++14 offers alias templates for all the C++11 type traits transformations.

## Item 10 : Prefer scoped enums to unscoped enums.

### 장점 1
C++98 stype, unscoped enums
```cpp
enum Color { black, white, red }; //same scope as Color
auto white = false; //error! white already declared in this scope
```

C++11 scoped enums
```cpp
enum class Color { black, white, red };
auto white = false;     // fine
Color c = white;        // error - no enumerator named white in this scope
Color c = Color::white; // fine
auto c = Color::white;  // fine
```

### 장점 2

unscoped enums can implicitly convert to integral types
```cpp
enum Color { black, white, red};

std:;vector<std::size_t> primeFactors(std::size_t x);

Color c = red;
if(c<14.5){
    auto factor = primeFactors(c);
}
```

scoped enum 는 strong type!
```cpp
enum class Color { black, white, red};
Color c = Color::red;

if(c < 14.5){ //error
    auto factors = primeFactors(c); //error! can't pass Color to function expecting std::size_t
}
```

그래도 다른 타입과 호환하고 싶으면 static_cast를 쓰셈
```cpp
if(static_cast<double>(c) < 14.5){ // valid
    auto factors = primeFactors(static_cast<std::size_t>(c)); // can compile
}
```

### 장점 3
forward declared 가능
```cpp
enum Color; //error
enum class Color; // fine
```

### 잘못된 점 ???
enum의 값은 compiler가 integral type 중에 임의로 넣음.

때론 range가 너무 커질 수 있음
```cpp
enum Status {
    good = 0,
    incomplete = 100,
    corrupt = 200,
    indeterminate = 0xFFFFFFFF
};
```

### 장점 4
forward declare가 불가능한 enum은 값을 추가하면 Status 를 쓰는 코드 전부 재컴파일 해야함
```cpp
enum Status {
    good = 0,
    incomplete = 100,
    corrupt = 200,
    audited = 500,
    indeterminate = 0xFFFFFFFF
};
```

### 장점 5
underlying type 을 선언할 수 있음 (default 는 int)
```cpp
enum class Status; //int
enum class Status: std::uint32_t;
enum Color: std::unint8_t; //이렇게하면 forward declared enum type이 됨. unscoped 이더라도
enum class Status: std::uint32_t{ good =0 , failed =1 }; // definition 과 같이 쓸 수 있음
```

이렇게 scoped enum으로 namespace pollution, implicit type conversion 을 피할 수 있어

### indexing
field 이름으로 get 할 수 있어
```cpp
using UserInfo = std::tuple<std::string,    //name
                            std::string,    //email
                            std::size_t;    //reputation

UserInfo uInfo;
auto val = std::get<1>(uInfo); // do not convince 1'st field is name, and not capturable code

enum UserInfoFields{ uiName, uiEmail, uiReputation};
auto val = std::get<uiEmail>(uInfo);
```
```cpp
enum class UserInfoFields{ uiName, uiEmail, uiReputation};
auto val = std::get<static_cast<std::size_t>(UserInfoFields::uiEmail)>(uInfo);
```
그런데 이런식으로 쓰려면 constexpr ([Item 15]) function으로 써야지.

constexpr과 std::underlying_type<E>::type 으로 이렇게 만들수 있어
```cpp
template<typename E>
constexpr typename std::underlying_type<E>::type toUType(E enumerator) noexcept{
    return static_cast<typename std::underlying_type<E>::type> enumerator;
}
```
C++14에서는 std::underlying_type<E>::type 을 std::underlying_type_t로 할 수 있다.
```cpp
template<typename E>
constexpr typename std::underlying_type_t<E> toUType(E enumerator) noexcept{
    return static_cast<typename std::underlying_type_t<E>> enumerator;
}
```
그러면 이렇게 쓸 수 있어
```cpp
    auto val = std::get<toUType(UserInfoFields::uiEmail)>(uInfo);
```
**Q : 근데 이러면 앞에꺼보다 더 복잡한거 아니야?**

enum의 pitfall을 피하기 위해서는 조금 손 더 가는 것은 감수해야...

### 요약
 1. C++98-style enums are now known as unscoped enums.
 2. Enumerators of scoped enums are visible only within the enum. They convert to other types only with a cast.
 3. Both scoped and unscoped enums support specification of the underlying type. The default underlying type for scoped enums is int. Unscoped enums have no default underlying type.
 4. Scoped enums may always be forwarded-declared. Unscoped enums may be forward-declared only if their declaration specifies an underlying type.

## Item 11 : Prefer deleted functions to private undefined ones.
클라이언트한테 노출 시키고 싶지 않은 member function 이 있을때, public - =delete 로 선언하셈.

C++98에서는 private하는 방법 밖에 없는데, private 은 friend 를 쓰면 쓸 수 있어.

 * public 으로 해야하는 이유는?
    * private과 delete가 같이 있을 때, private으로 먼저 accessibility 를 결정하고 deleted가 무시됨.

int로의 implicit conversion (C로부터온 heritage중 가장 불편하게만드는 것) 을 막을 수 있음
```cpp
bool isLucky(int member);

if(isLucky('a')) ...
if(isLucky(true)) ...
if(isLucky(3.5)) ...
```
이런 경우 delete로 다른 타입을 만들면 다른 타입일 때 compile-error
```cpp
bool isLucky(int member);
bool isLucky(bool member) = delete;
bool isLucky(char member) = delete;
bool isLucky(double member) = delete;


if(isLucky('a'))    //error
if(isLucky(true))   //error
if(isLucky(3.5))    //error
```

template instantiation 을 막을 수 있다
```cpp
template<typename T>
void processPointer(T* ptr);

template<>
void processPointer<void>(void*) = delete;
template<>
void processPointer<char>(char*) = delete;
```
const version, volatile 버전도 만드셈

다른 포인터는 가능한데 void* 만 지우고 싶다면?
 * 클래스 안에서는 불가능
 * 클래스 밖에서 delete선언
```cpp
class Widget{
public:
    template<typename T>
    void processPointer(T* ptr){...}
};
template<>
void Widget::processPointer<void>(void*) = delete;
```

### 요약
 1. Prefer deleted functions to private undefined ones.
 2. Any function may be deleted, including non-member functions and template instantiations.