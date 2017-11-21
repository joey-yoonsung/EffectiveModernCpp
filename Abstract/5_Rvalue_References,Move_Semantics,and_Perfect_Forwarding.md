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

### move 에 lvalue 가 들어가서 lvalue 값이 이상해지는 문제
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
이렇게 하고 이 안에서 universal reference 를 forward로 받아서 써.


### std::move 랑 std::forward 를 universal reference 의 final use 로 사용한다.
#### std::forward 의 final use 예제
```cpp
template<typename T>
void setSignText(T&& text){
    sign.setText(text);
    auto now = std::chrono::system_clock::now();
    signHistory.add(now, std::forward<T>(text));
}
```

#### std::move 의 final use 예제
std::move_if_noexcept 를 어떨때 쓸지는 [Item14] 에서 다룸

universal reference 를 return by value 로 넘길 때
```cpp
Matrix
operator+(Matrix&& lhs, const Matrix& rhs){ // Q : 근데 이거 Matrix 타입이 명시되어있으니까 rvalue reference아닌가?
    lhs += rhs;
    return std::move(lhs);
}
```
여기서 lhs가 lvalue 일 때, std::move 를 안쓰면 컴파일러가 copy 로 넘김.
 * Matrix 가 moving을 지원 안해도 캐스팅할 때 copy 뜸.

std::forward로 return 할 때.
```cpp
template<typename T>
Fraction reduceAndCopy(T&& frac){
    frac.reduce();
    return std::forward<T>(frac); //rvalue 이면 move, lvalue는 copy
}
```
`주의! local variable 로  move, forward 때리면 안돼!`
 * C++ compiler 는 RVO를 해줘.
    * 불필요한 copy가 일어나지 않도록. (copy elision)
    * local object(variable) 은 return 할 때 copy가 일어나도록 함.
    * 그런데 std::move(local var) 의 결과는 local var 의 reference 이므로 local variable 이 아님. 따라서 RVO가 안일어나고 local var 는 free 됨.
        * return 받은 쪽에서는 free 된 놈의 reference 를 참조하니까 이상한 주소를 참조하게 됨.

RVO 예
```cpp
Widget makeWidget(Widget w){
    return sw;
}
//이 함수는 사실상 이렇게 동작함
Widget makeWidget(Widget w){
    return std::move(w);
}
```

### 요약
 1. Apply std::move to rvalue reference and std::forward to universal references the last time each is used.
 2. Do the same thing for rvalue references and universal references being returned from functions that return by value.
 3. Never apply std::move or std::forward to local objects if they would otherwise be eligible for the return value optimization.

## Item 26 : Avoid overloading on universal references.

### universal reference parameter 로 불필요한 copy 를 줄이자.
예제
```cpp
std::multiset<std::string> names;

void logAndAdd(const std::string& name){
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(name);
}

std::string petName("Darla");
logAndAdd(petName);                     //(1) pass lvalue
logAndAdd(std::string("Persephone");    //(2) pass rvalue
logAndAdd("Patty Dog");                 //(3) pass literal
```
 * 비효율적
    * (1) names.emplace  에서 copy가 됨.
    * (2) 이것도 names.emplace 에서 copy 가 된다. 단, rvalue 를 lvalue 인 name 에 bind 하는 과정에서 move.
        * **Q : move가 암시적으로 불린다는 건가?**
    * (3) literal 로 넘어가고 literal 이 emplace 부분에서 copy 되고 string 으로 생성됨. 2와의 차이점은 move operation 비용 없이 copy 한 번만 있다는 것.

universal reference 로 개선
```cpp
template<typename T>
void logAndAdd(T&& name){
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(std::forward<T>(name));
}
```
이렇게 하면 위에 2, 3에 있던 copy 과정이 없이 들어간다.

### universal reference 가 쓰인 overload 케이스.
#### 일반 함수
```cpp
void logAndAdd(int idx){
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(nameFromIdx(idx));
}
short nameIdx;
logAndAdd(nameIdx); //error!
```
여기서 short 로 넣으면 parameter가 int인게 불리는게 아니라, universal reference 인 메소드가 불림.
 * std::string 이 short 를 받는 constructor 가 없어서 에러.

#### perfect forwarding constructor 의 copy, move constructor
```cpp
class Person{
public:
    tmeplate<typename T>
    explicit Person(T&& n) : name(std::forward<T>(n)) {} // perfect forwarding ctor;

    explicit Person(int idx) : name(nameFromIdx(idx)){}

    Person(const Person& rhs);  // compiler-generated copy ctor
    Person(Person&& rhs);       // compiler-generated move ctor
private:
    std::string name;
}
Person p("Nancy");
auto cloneOfP(p);   // this won't compile!
```
copy constructor 가 불리는 게 아니라 perfect forwarding constructor 가 불림.
 * p는 non-const니까 templatized parameter 로 넘어가는게 적합.
 * 그런데 perfect forwarding constructor 내부에서 string 을 초기화할 때 string 이 Person을  constructor arg로 받지 못해서 에러.

#### perfect forwarding constructor 가 쓰인 클래스를 상속받는 클래스

```cpp
class SepcialPerson : public Person{
public:
    SpecialPerson(const SpecialPerson& rhs) : Person(rhs){...} //copy ctor; calls base class forwarding ctor.
    SpecialPerson(SpecialPerson&& rhs) : Person(std::move(rhs)){...} //move ctor; calls base class forwarding ctor.
}
```
여기서 Person 의 perfect forwarding constructor 로 SpecialPerson 을 넘김. 내부에서 SepcialPerson 을 받는 string constructor 가 없어서 에러.

universal reference 를 overloading 하는 이런 여러가지 경우를 어떻게 해결하는지는 [Item 27]에서 다룬다.

### 요약
 1. Overloading on universal reference almost always leads to the universal reference overload being called more frequently than expected.
 2. Perfect-forwarding constructors are especially problematic, because they're typically better matches than copy constructors for non-const lvalues, and they can hijack derived class calls to base class copy and move constructors.

## Item 27 : familiarize yourself with alternatives to overloading on universal references.
### [Item 26] 의 첫번째 문제 -> Use Tag dispatch
Item 26의 logAndAdd를 바꿔보기
```cpp
template<typename T>
void logAndAdd(T&& name){
    logAndAddImpl(std::forward<T>(name), std::is_integral<typename std::remove_reference<T>::type>());
    // remove_reference 를 안하면 [Item 28] 에서 배운 것 처럼 int 가 lvalue로 들어와서 int& 로 옴.
    // * 따라서 std::is_integral< T >() 는 항상 false.
}
```
이렇게하면 std::is_integral 의 결과가 런타임에 결정되는거에 따라 아래의 두개의 Impl 로 overload 할 수 있어.
```cpp
template<typename T>
void logAndAddImpl(T&& name, std::false_type){
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(std::foward<T>(name));
}

std::string nameFromIdx(int idx); //as Item 26
template<typename T>
void logAndAddImpl(int idx, std::true_type){
    logAndAdd(nameFromIdx(idx));
}
```
여기서 std::true_type, std::false_type 쓴거는 "tags" : only purpose is to force overload resolution
 * _tag dispatch_ : tag object 생성을 통해서 적절한 overload 가 불리도록 하는 방법.
    * 표준 라이브러리에서 많이 발견할 수 있음

### Constraining template that take universal reference.
tag dispatch 의 핵심은 client API 하나로 한다는 것.

#### [Item 26] 의 두번째 문제. -> enable_if
 * 진짜 문제는 compiler-generated function 이 때때로 tag dispatch design 을 지나친다는게 아님.
    * 항상 pass 하지는 않는다는게 문제!
 * 여기서 써야하는건 tag dispatch가 아니라 enable_if

enable_if
 * force compilers to behave as if a particular template didn't exist.

```cpp
class Person{
public:
    template<typename T, typename = typename std::enable_if<condition>::type>
    explicit Person(T&& n);

}
```
`enable_if 가 어떻게 동작하는지 알고 싶으면 "SFINAE" technology 를 찾아봐`

우리가 원하는 조건은
```cpp
!std::is_same<Person, T>::value
```
근데 이렇게 하면 안되지롱! [Item 28]
 * lvalue 로 initialize 된 universal reference 는 항상 lvalue reference 로 type deduction 된다.
```cpp
Person p("GawangKim");
auto cloneOfP(p); //initialize from lvalue.
// std::is_same<Person, Person&>::value == false
```
의도는 같은 타입이어서 copy constructor 타게 하고 싶은건데 이렇게하면 perfect forwarding parameter 를 탄다.
 * reference 무시하고(벗기고) 타입 확인
 * const 와 volatile 무시하고(벗기고) 타입 확인
 * std::decay< T >::type 을 쓰면 됨.
    * reference and cv-qualifier 를 제외한 타입을 던짐.

적용
```cpp
!std::is_same<Person, typename std::decay<T>::type>::value

class Person{
public:
    template<
        typename T,
        typename = typename std::enable_if<
                    !std::is_same<Person,
                                    typename std::decay<T>::type
                                    >::value
                    >::type
    >
    explicit Person(T&& n);
}
```
근데 이거 알아보기 힘듬.

#### [Item 26] 의 세번째 문제 -> enable_if + is_base_of
```cpp
std::is_base_of<T1, T2>::value
```
T2가 T1을 상속 받았는지를 확인할 수 있음.
 * is_same 대신 is_base_of를 쓰면 댐

```cpp
class Person{
public:
    template<
        typename T,
        typename = typename std::enable_if<
                    !std::is_base_of<Person,
                                    typename std::decay<T>::type
                                    >::value
                    >::type
    >
    explicit Person(T&& n);
}
```
C++14 예제는 깔끔하게 생략한다.

integral type 도 들어오면 안댐
 1. integral args 를 받는 constructor 를 overload 한다.
 2. perfect forwarding constructor 에 integral type 을 제외하는 로직을 추가한다.

```cpp
class Person{
public:
    template<
        typename T,
        typename = typename std::enable_if<
                    !std::is_base_of<Person, typename std::decay<T>::type>::value
                    &&
                    !std::is_integral<std::remove_reference_t<T>>::value
                    >::type
    >
    explicit Person(T&& n) : name(std::forward<T>(n)){...};
    explicit Person(int idx) : name(nameFromIdx(idx)){...};
private:
    std::string name;
}
```

#### `주의! universal reference 와의 overloading 을 피할 수 없을 때 쓰란거! 먼저 피하는게 Better!`

비교!
 1. abandon overloading, passing by const T&, passing by value
 2. perfect forwarding.
    * 장점 : more efficient.
        * temp object 생성이 줄어듬
    * 단점 : usability
        * some args can't be perfect-forwarded
        * [Item 30] 에서 이 perfect forwarding 이 실패하는 케이스르 다룸.
        * invalid arg 를 던졌을 때, client 측에서 받는 error message 가 복잡해짐.

### perfect fowarding 에 invalid arg 를 던졌을 때, client 측에서 받는 error message 가 복잡해짐 -> static_assert 쓰셈.
예제
```cpp
Person p(u"Kawang Kim."); // 16-bit character instead of char
```
const char16_t[12] 를 int 나 std::string 으로 conversion 할 수 없다고 떨구겠지?
 * 아니야! 우선 forward 되고 string constructor 에서 받아서 160라인이 넘는 에러메세지를 받을 것.
 * forwarding 된 이후에 복잡한 시스템일 수록 더 복잡한 에러메세지를 뱉을 수 있어.
    * perfect forwarding 을 많이 사용할수록...
    * 성능 이슈도 있음.
 * static_assert 에 is_constructable 로 컴파일 타임에 검증할 수는 있음

static_assert를 이용한 검증
```cpp
class Person{
public:
    template<
        typename T,
        typename = typename std::enable_if<
                    !std::is_base_of<Person, typename std::decay<T>::type>::value
                    &&
                    !std::is_integral<std::remove_reference_t<T>>::value
                    >::type
    >
    explicit Person(T&& n) : name(std::forward<T>(n)){
        static_assert(
            std::is_constructable<std::string, T>::value,
            "Parameter n can't be used to construct a std::string"
        );
    };
}

```

### 요약
 1. Alternatives to the combination of universal references and overloading include the use of distinct function names, passing parameters by lvalue-reference-to-const, passing parameters by vlaue, and using tag dispatch.
 2. Constraining templates via std::enable_if permits the use of universal references and overloading together, but it controls the conditions under which compiler may use the universal reference overloads.
 3. Universal reference parameters often have efficiency advantages, but they typically have usability disadvantages.


## Item 28 : Understand reference collapsing.

universal reference parameter 가 type deduction 하는 rule
 * lvalue : lvalue reference
 * rvalue : non-reference

C++ 에서 reference 의 reference 는 에러!

그런데 lvalue 를 넣는거는 사실상 보면 reference to reference 인거야.
```cpp
void func(Widget& && param);
```

그런데 어떻게 여기서 Widget& param이 될까?
 * 여기서 사용하는게 reference collapsing

reference to reference 가 발생할 때(eg. during template instantiation) reference collapsing 이 발동되서 single reference 로 만듬.
```
If either reference is an lvalue reference, the result is an lvalue reference. Otherwise (t.e., if both are rvalue reference) the result is an rvalue reference.
```

#### reference collapsing 이 발생하는 4가지 경우
  1. template instantiation (most common case)
  2. type generation for auto variable
    * type deduction 원리와 똑같으므로 1.의 케이스 처럼 동작함
  3. generation and use of typedefs and alias declarations.
  4. decltype. decltype 에서 타입 추론하는 과정에서 reference to reference 문제를 해결함.


### reference collapsing on template instantiation
std::forward 구현 간략
```cpp
template<typename T>
T&& forward(typename remove_reference<T>::type& param){
    return static_cast<T&&>(param);
}
```

#### lvalue 의 std::forward 과정
여기에 lvalue 가 넘어갔다고 하면 이렇게 된다고 볼 수 있음
```cpp
Widget& && forward(typename remove_reference<Widget&>::type& param){
    return static_cast<Widget& &&>(param);
}
```

remove_reference 가 적용되면 그냥 Widget 이 떨궈짐
```cpp
Widget& && forward(Widget& param){
    return static_cast<Widget& &&>(param);
}
```

return type 에 대한 reference collapsing 결과
```cpp
Widget& forward(Widget& param){
    return static_cast<Widget&>(param);
}
```

#### rvalue 의 std::forward 과정
```cpp
Widget&& forward(typename remove_reference<Widget>::type& param){
    return static_cast<Widget&&>(param);
}
```

remove_reference 가 적용되면 그냥 Widget 이 떨궈짐
```cpp
Widget&& forward(Widget& param){
    return static_cast<Widget&&>(param);
}
```
이 경우에는 reference collapsing(reference to reference) 가 없음.

### reference collapsing on type generation for auto variable

#### lvalue
```cpp
Widget w;

auto && w1 = w;
```
이 과정은

```cpp
Widget& && w1 = w;
```
```cpp
Widget& w1 = w;
```

#### rvalue
```cpp
Widget widgetFactory();

auto && w2 = widgetFactory();
```
no reference collapsing

#### universal reference 가 rvalue reference 가 되는 2가지 조건 (&&)
  1. Type deduction distinguishes lvalues from rvalues
    * lvalue -> T&
    * rvalue -> T
  2. Reference collapsing occurs
  * universal reference concept 이 좋은 이유
    * reference collapsing context 로 부터 자유롭다.


### reference collapsing on generation and use of typedefs and alias declarations.
```cpp
template<typename T>
class Widget{
public:
    typedef T&& RvalueRefToT;
}
```
여기서 Widget의 T를 lvalue reference type 으로 만들면
```cpp
Widget<int&> w;
```
```cpp
typedef int& && RvalueRefToT;
```
Reference collapsing 이 발생한다. 따라서 결과는
```cpp
typedef int& RvalueRefToT;
```

### 요약
  1. Reference collapsing occurs in four contexts:
    1) template instantiation
    2) auto type generation
    3) creation and use of typedefs and alias declarations
    4) decltype
  2. When compilers generate a reference to a reference in a reference collapsing context, the result becomes a single reference. If either of the original references is an lvalue reference, the result is an lvalue reference. Otherwise it's an rvalue reference.
  3. Universal references are rvalue reference in contexts where type deduction distinguishes lvalues from rvalues and where reference collapsing occurs.


## Item 29 : Assume that move operations are not present, not cheap, and not used.

C++11 컴파일러는 move operation 을 auto-generate 해줌 (||)
 * no copy operations
 * no move operations
 * no destructors 

[Item 11] 에 나온 것 처럼 member 를 moving 못하게 해서 move operation auto generation 을 막을 수 있음

std container 는 모두 move operation 을 지원 하지만, move가 항상 빠르다고 생각하면 안됨.

### move가 copy 보다 빠르지 않은 경우 (copy 가 move보다 느리지 않음)
 * std::array
    * heap 에 있는 data 자체를 move 함 : linear time
 * std::string based on SSO (small string optimization)
    * 대략 15글자 이하의 글자는 string 이 기본 buffer로 가질 수 있어 (dynamic alloc 안일어남.)
        * 따라서 move 나 copy 나 차이없음.

C++98 의 copy를 C++11의 move로 replace 보장 해주는 경우는 해당 move 가 throw를 던지지 않을 때.(exception safety guarantee)-noexcept

#### Scenarios of C++11's move semantics not good
 1. No move operations
    * move operation 제공에 실패하면 copy 때려버림
 2. Move not faster
    * **[my opinion] 괜히 dangling 될 수 있음**
 3. Move not usable
    * context 상 exception 을 던지지 않지만, noexcept 선언이 안된 경우.
 4. Source object is lvalue
    * [Item 25] 에 있는 몇 예외만 빼면, rvalue 만 쓰셈.

### 요약
  1. Assume that move operations are not present, not cheap, and not used.
  2. In code with known types or support for move semantics, there is no need for assumptions.

## Item 30 : Familiarize yoursef with perfect forwarding failure cases.

perfect forwarding
  * 목표는 second function 이 first function 과 같은 객체를 사용하는 것!!
  * reference 인 애들만 perfect forwarding 의 대상으로 삼는다.
  * 단순히 객체의 forwarding 뿐만 아니라 특성까지 (l/r value, cv-qualifier)
  * universal reference 인 것을 암시 (l/r value 다 받으려면), (reference parameter 에 대해서만 다루니까)

형태
```cpp
template<typename T>
void fwd(T&& param){
    f(std::forward<T>(param));
}
template<typename... Ts>
void fwd(Ts&&... params){
    f(std::forward<Ts>(params));
}
```
이제 위의 예에서 f(expression) 과 fwd(expression) 가 실패하는 케이스를 다룰거야

### Braced Initializer
f의 형태가
```cpp
void f(const std::vector<int>& v);
```
이고 f를 아래와 같이 콜하면
```cpp
f({1,2,3}); //implicitly converted to std::vector<int>
```
하지만 fwd는 컴파일이 안됨
```cpp
fwd({1,2,3}); //error
```
braced initializer는 perfect forwarding failure case 로 정해져있음
 * std::initializer_list 는 non-deduced context 이다
    * 이 타입은 deduction 을 수행하지 않겠다!  parameter가 std::initializer_list로 선언되어있지 않으니까 컴파일러가 deduction을 막는다.
        * [Item 2]에서 다룬 auto 는 initializer_list object라고 컴파일러가 판단하니까 되는거임.
            * auto로 local var선언하고 이 var를 넘기셈

```cpp
auto il = {1,2,3};
fwd(il); //fine
```
#### perfect forwarding 이 실패하는 두 가지 경우
 1. Compilers are unable to deduce a type
 2. Compilers deduce the "wrong" type
    * wrong의 의미는 fwd의 초기화가 deduced type으로 컴파일 될 수 없을 때
    * 여기서는 fwd 로 deduced 되서 들어간게 f 로 들어갔을 때 deduced 된거랑 다를 수 있음.

### 0 or NULL as null pointers
[Item 8]에서 다룬것 처럼 0과 null 은 type deduction 에서 integral type 으로 deduction 됨. 따라서 perfect forwarding 에서도 nullptr 로 forward 되지 못함.

### Declaration-only integral static const data members
일반적인 룰에 따르면 integral static const 멤버를 클래스에 선언할 필요가 없어 애초에
 * 그냥 혼자 선언하는 것으로 충분
 * compiler가 const propagation을 한다. 그래서 그 멤버에대한 메모리 요구를 지움
    * (예제)컴파일러가 MinVals 사용된 곳을 다찾아가서 28을 넣어줌.

```cpp
class Widget{
public:
    static const std::size_t MinVals = 28;
};
std::vector<int>widgetData;
widgetData.reserve(Widget::MinVals);

f(Widget::MinVals); //fine. f(28);
fwd(Widget::MinVals); //error! shoudn't link
```
MinVal은 모두 28 값으로 다 컴파일러가 채워넣었는데 fwd는 parameter 로 universal reference 를 받으니까 주소를 찾는데, 찾을 수 없어서 link error.
 * pointer를 넘기는거랑 같음. (dereferenced pointer가 넘어간 꼴)
 * **Q : 근데 cpp 파일에 선언하면 할 수 있다??**

### Overloaded function names and template names
#### Overloaded function name 이 실패하는 케이스
```cpp
void f(int (*pf)(int));
void f(int pf(int));
int processVal(int value);
int processVal(int value, int priority);
f(processVal);
```
이 경우에 compiler 가 f 에는 int parameter 하나짜리 processVal 함수가 필요한 줄 알고 선택해줌

```cpp
fwd(processVal);
```
하지만 이렇게 하면 processVal 은 함수 이름이고, type을 가지지 않음 타입이 없으니까 type deduction 을 하지 못함. type deduction 을 못하면 perfect forwarding 도 실패.

#### Overloaded template name 이 실패하는 케이스
```cpp
template<typename T>
T workOnVal(T param){ ... } // template for processing values

fwd(workOnVal); //error which workOnVal instantiation?
```
function template은 여러개의 function 을 지칭함. 하나로 deduction 할 수가 없음.

해결하려면 overload 된 function name 이나 template name 중에 perfect forwarding 을 원하는 함수만 수동으로 type을 명확하게 해줘.
```cpp
using ProcessFuncType = int (*)(int);
ProcessFuncType processValPt = processVal;
fwd(processValPtr); //fine
fwd(static_cast<ProcessFuncType>(workOnVal)); //fine
```

`이런거 있으면 Document 화 하셈!!!`

### Bitfields
IPv4 header 예제
```cpp
struct IPv4Header{
    std::uint32_t version:4,
                    IHS:4,
                    DSCP:6,
                    ECN:2,
                    totalLength:16;
};

void f(std::size_t sz);
IPv4Header h;
f(h.totalLength); //fine

fwd(h.totalLength); //error
```
C++ Standard condemn : "A non-const reference shall not be bound to a bit-field"
 * bitfield 는 machine word의 arbitrary part에 존재해서 direct address 할 방법이 없음
    * C++ 에서는 pointer할 수 있는 최소 단위가 char임. bit단위는 pointing 할 수 없음.
        * bitfield 받아 쓰려면 copy 받아서 쓰셈
        * reference-to-const 로 쓰는거는 사실상. const 로 reference 할때 Integral type으로 bitfield를 copy 후 binding 되서 그 주소를 참조하는 거임.

정 쓰고 싶으면 auto 로 받아서 쓰셈
```cpp
auto length = static_cast<std::uint16_t>(h.totalLength);
fwd(legnth); // forward the copy
```

### Upshot
Perfect forwarding is imperfect. Important is knowing how to work around them.

### 요약
 1. Perfect forwarding fails when template type deduction fails or when it deduces the wrong type.
 2. The kinds of arguments that lead to perfect forwarding failure
    1) braced initializer
    2) null pointers expressed as 0 or NULL
    3) declaration-only integral const static data members
    4) template and overloaded function names
    5) bitfield.