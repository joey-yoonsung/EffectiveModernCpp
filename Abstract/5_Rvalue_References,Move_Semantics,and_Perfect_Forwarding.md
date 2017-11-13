# 5. Rvalue References, Move Semantics, and Prefect Forwarding

#### Move Semantics
expensive copying operation 을 move operation으로 대체할 수 있다.

move constructor, move assignment operator 를 정의해서 사용할 수 있다.

move only-type 을 만들 수 있다. (eg. std::unique_ptr, std::future, std::thread)

#### Perfect Forwarding
template으로 arbitrary arg를 받아서 그대로 다른 function 으로 pass 할 수 있다.(받은 쪽에서는 origin function이 받은 것과 똑같이 받을 수 있음)

이번 장의 목표 : move semantic 과 perfect forwarding 에 깔린 mechanism 을 이해해보자.

오해하지 말하야 할 것
 * move semantic 은 아무것도 move하지 않을 수 있음.
 * move 가 copy보다 항상 싸지 않아.
 * perfect forwarding is imperfect.
 * type&& 는 항상 rvalue reference 는 아니야.

## Item 23 : Understand std::move and std::forward
이걸 이해하는 데 좋은 접근은 이놈들이 어떤 동작은 하지 않는지를 알아보는 것.

std::move, std::forward 는 실제로는 casting을 수행하는 function template일 뿐.
 * std::move : unconditionally casts its arg to rvalue.
 * std::forward : 조건하에서 cast 함.

move 간략 구현
```cpp
tempalte<typename T>
typename remove_reference<T>::type&&
move(T&& param)
{
    using ReturnType = typename remove_reference<T>::type&&;
    return static_cast<ReturnType>(param);
}
```
**move는 항상 rvalue로 캐스팅 수행. rvalues are only usually candidates for moving.

### move 예제
무조건 cast
```cpp
class string{
public:
    string(const string& rhs);  //copy ctor
    string(string&& rhs);       //move ctor
}
```

이 예제에서 주는 교훈
 * move 할 object 는 const 로 선언하지 마셈.
    * const 를 move에 넣으면 implicit copy 됨.
 * move 는 실제로 항상 move 는 아님. + move 의 대상이 move에 적합한 놈인지를 개런티해주지 않음.

### forward 예제
조건적 cast
```cpp
void process(const Widget& lvalArg);
void process(Widget&& rvalArg);

template<typename T>
void logAndProcess(T&& param)
{
    auto now = std::chrono::system_clock::now();
    process(std::forward<T>(param)); //이 지점에서 T가 rvalue initialize 가 안되어있음.
}
```
이거를 이렇게 콜해보면
```cpp
Widget w;

logAndProcess(w);               // call with lvalue
logAndProcess(std::move(w));    // call with rvalue
```
그런데 이렇게 하면 둘다 forward에 전달된 param은 lvalue. 이걸 막으려면 param 이 forward에 전달 될 때 initialize 되야함.
 * template parameter입장에서는 T의 encoded information 으로 판단함. forward에 T를 넣을 때 encoded information 을 불러옴.
    * **그래서 위 코드가 rvalue로 돈다는거야 안돈다는거야?**
        * 돈다.

결국 forward 로 모든 operation 을 할 수 있어. 그런데 사용편의성 관점에서는 move가 더 좋지. forward를 쓰면 encode할 타입을 맞춰야 하니까.

같은 내용으로 짜보면
```cpp
class Widget{   //move version
private:
    static std::size_t moveCtorCalls;
    std::string s;
public:
    Widget(Widget&& rhs)
    : s(std::move(rhs.s))
    { ++moveCtorCalls; }
}

class Widget{   //forward version
private: ...
public:
    Widget(Widget&& rhs)
    : s(std::forward<std::string>(rhs.s))   //std::string 명시한 부분이 encoding 되는 지점.
    { ++moveCtorCalls; }
}
```

### 요약
 1. std::move performs an unconditional cast to an rvalue. In and of itself, it doesn't move anything.
 2. std::forward casts its argument to an rvalue only if that argument is bound to an rvalue.
 3. Neither std::move nor std::forward do anything at runtime.

## Item 24 : Distinguish universal references from rvalue references.

rvalue reference type (T&&) 으로 진짜 rvalue 인지 알아보기 쉽지 않을껄?
```cpp
void f(Widget&& param);     // rvalue reference : no type deduction
Widget&& var1 = Widget();   // rvalue reference : no type deduction
auto&& var2     = var1;     // not rvalue reference

template<typenmae T>
void f(std::vector<T>&& param); //rvalue reference : T는 deduction 있지만, param의 타입은 rvalue reference 인걸로 정해짐.

template<typename T>
void f(T& param);               //not rvalue reference
```
T&& 는 const, volatile 도 받을 수 있음

universal reference 일 때, rvalue 의 판단은 initializer 가 rvalue 로 initialize 하면 rvalue 이다.
 * 두가지 케이스
    * template parameter T&&
    * auto&&
 * type deduction 이 필요하면 rvalue 인지 판단할 수 없다.
    * 이때는 encoded information 을 보고 rvalue 인지 안다.
        * encoded information 은 initializer 가 넣는다.

rvalue reference 인 것을 정해지는 경우.
 * Type deduction 이 필요 없는 경우.
 * const 붙이면.
 * class 선언에서 결정된 typename 으로 member function 에서 쓰는 경우.
    * 이미 클래스 선언할때 deduction 끝. 함수 호출시엔 이미 deduction 된 타입의 rvalue reference로 쓰인다.
        * 그래서 stl container 에서 emplace_back 함수를 지원

emplace_back 예제
```cpp
template<class T, class Allocator = allocator<T>>
class vector {
public:
    template <class... Args>    // 여기에 T를 또 쓰면 어떻게 되지?
    void emplace_back(Args&&... args);
}
```

**기억할것 : the foundation of universal reference is a lier, an "abstraction"**
 * reference collapsing 을 알아야함.[Item 28] 에서 다룸

### 요약
 1. It a function template parameter has type T&& for a deduced type T, or if an object is declared using auto&&, the parameter or object is a universal reference.
 2. If the form of the type declaration isn't precisely type&&, or if type deduction does not occur, type&& denotes an rvalue reference.
 3. Universal references correspond to rvalue references if they're initialized with rvalues. They correspond to lvalue references if they're initialized with lvalues.


## Item 25 : Use std::move on rvalue references, std::forward on universal references.

rvalue reference 는 rvalue로 조건없이 cast되야 하니까. move 로 넘겨.
 * move랑 universal reference랑 같이 쓰는건 lvalue 를 예상치않게 변경할 수 있음.
universal reference 는 rvalue 인지 조건 따져야 하니까 forward로 넘겨.
 * forward랑 rvalue reference랑 같이 쓰는 건 error-prone.

적절한 예제
```cpp
class Widget{
public:
    Widget(Widget&& rhs) : name(std::move(rhs.name)), p(std::move(rhs.p)){...}
    template<typename T>
    void setName(T&& newName)
    { name = std::forward<T>(newName); }
}
```

move 에 lvalue 가 들어가서 lvalue 값이 이상해지는 문제
```cpp
std::string getWidgetName();
Widget w;
auto n = getWidgetname();
w.setName(n);
// n's value now unknown
```

setName 함수를 이렇게 고쳐봐
```cpp
class Widget{
public:
    void setName(const std::string& newName){ name = newName; } //set from const lvalue
    void setName(std::string&& newName){ name = std::move(newName); } //set from rvalue
}
```
이렇게 두벌 만드는 것의 단점
 * 코드 유지보수 힘듬
 * overload 로 인한 runtime overhead
 * w.setName("new Name"); 이런식으로 쓸 경우 cost가 커짐.
    * string constructor -> move -> string destructor 타게됨. 걍 const char* 로 string 한번만 만들면 될 것을
        * **이건 좀 억지스러운데**
 * 가장 중요한 결점은 poor scalability of the design.
    * parameter 가 여러개라고 생각하면 n^2 으로 overload 해야하는 함수의 경우의 수가 많아짐.
    * make_shared, make_unique 를 참고해봐.

```cpp
template<class T, class.. Args>
shared_ptr<T> make_shared(Args&&... args);  //C++11

template<class T, class.. Args>
shared_ptr<T> make_unique(Args&&... args);  //C++14
```

std::move 랑 std::forward 를 universal reference 의 final use 로 보장하고 싶다.



