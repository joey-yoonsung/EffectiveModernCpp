# 1. Deducing Type

 * c++11에서 추가된 것
    * auto
    * decltype

 * type deduction 의 장점
    * 코딩 생산성 증가
    * more adaptable
        * 최초 type 선언 부에서만 명시적이면 나머지는 auto 로 propagate 되니까

 * type deduction 의 단점
    * 덜 직관적 : 사람은 추론된 code 를 이해하기 힘듬 (auto로 선언 된 부분만 보고)

 * type deduction 의 operation을 정확히 이해하고 써야 함
 * type deduction 이 쓰이는 context
    * in calls to function templates (auto)
    * in decltype expression
    * (C++14) decltype(auto) construct



## Item 1: Understand template type deduction
  * auto : template에 대한 type deduction

예제
```cpp
  template<typename T>
  void f(ParamType param);

  f(expr);
```
이때 T에 대한 type deduction 은 expr뿐만 아니라 ParamType에 대해서도 dependent 하다

 * 3가지 케이스로 나눌 수 있음
    * ParamType is pointer or reference type (lvalue), not universal reference.
    * ParamType is a universal reference.
    * ParamType is neither a pointer nor a reference.(rvalue)


### Case 1: ParamType is pointer or reference type (lvalue), not universal reference.

 * works like this
    * If expr's type is a reference, ignore the reference part.
    * Then pattern-match expr's type against ParamType to determine T.

#### Case1 : 예제1
```cpp
  template<typename T>
  void f(T& param);

  int x = 27;
  const int cx = x;
  const int& rx = x;

  f(x);         //(1)
  f(cx);        //(2)
  f(rx);        //(3)
```

 1. T = int, param = int&
 2. T = int, param = const int&
    * 여기서 이미 f 안에 param 을 수정하는 로직이 들어가 있다면? 런타임 에러?

 3. T = int, param = const int&
    * T는 non-reference 로 deduce됨.

#### Case1 : 예제2
```cpp
  template<typename T>
  void f(const T& param);

  int x = 27;
  const int cx = x;
  const int& rx = x;

  f(x);         //(1)
  f(cx);        //(2)
  f(rx);        //(3)
```

 1. T = int, param = const int&
 2. T = int, param = const int&
 3. T = int, param = const int&


#### Case1 : 예제3
```cpp
  template<typename T>
  void f(T* param);

  int x = 27;
  const int *cx = &x;

  f(&x);         //(1)
  f(cx);        //(2)
```

 1. T = int, param = int *
 2. T = int, param = const int*


### Case 2: ParamType is a universal reference.

 * works like this
    * If expr is an lvalue, both T and PramType are deduced to be lvalue references.
    * If expr is an rvalue, the normal(Case1) rules apply.


#### Case2 : 예제1
```cpp
  template<typename T>
  void f(T&& param);

  int x = 27;
  const int cx = x;
  const int& rx = x;

  f(x);         //(1)
  f(cx);        //(2)
  f(rx);        //(3)
  f(27);        //(4)
```

 1. (because x is lvalue) T = int&, param = int&
 2. (because cx is lvalue)T = const int&, param = const int&
 3. (because rx is lvalue)T = const int&, param = const int&
 4. (because 27 is rvalue)T = int, param = int&&
 * [Item24] 에서 상세히 설명


### Case 3: ParamType is Neither a Pointer nor a Reference
즉, pass by value

 * works like this
    * If expr's type is a reference, ignore the reference part.
    * If, after ignoring expr's reference-ness, expr is const, ignore that, too. if it's volatile, also ignore that. (detail [Item40])

#### Case3 : 예제1
```cpp
  template<typename T>
  void f(T param);

  int x = 27;
  const int cx = x;
  const int& rx = x;

  f(x);         //(1)
  f(cx);        //(2)
  f(rx);        //(3)
```

 1. T = int, param = int
 2. T = int, param = int
 3. T = int, param = int

#### Case3 : 예제2
```cpp
  template<typename T>
  void f(T param);

  const char* const ptr = "Fun"; //const object 의 pointer
  f(ptr);                        //pass arg of type const char* const
```

 1. 오른쪽 const 는 ptr 이 const - 포인터 위치를 못바꿈
 2. 왼쪽 const 는 char* 이 const - string을 못바꿈
 3. 위의 룰에 따라서 ptr의 value인 const char* 가 넘어간다. 넘어간 포인터는 modifiable.


## Array Arguments

결론은 parameter로 truly array 를 못하고 & 로 받는다. 그래서 ParamyType은 T&.

그리고 그 array 사이즈는 compile time에 계산된다.

 * 그래서 머 어쩌라고?ㅋ
 * 그러면 rumtime 에 사이즈 결정되는 array 를 템플릿에 넘기면 어떻게하지?

## Function Arguments
function name 을 Param으로 넘기면 어떻게 되나? function pointer로 deduction 된다.

결론은 array가 pointer 로 deduction 되는 방식과 같다.

 * 그래서 어쩌라고?ㅋ

여기까지 auto 를 이해하기 위한 기초작업은 완료 되었음.


## Item 2: Understand auto type deduction
template type deduction 은 template, function, parameter에 관해서

auto는 위 3가지에 쓰이는 것은 아니지만, 원리는 template type deduction 이랑 똑같음 (예외 1가지 있음)

 * auto의 역할
    * template 에서 T 역할.
    * type specifier 는 ParamType 처럼 동작.

 * Case
    * Case 1: The type specifier is a pointer or reference, but not a universal reference.
    * Case 2: The type specifier is an universal reference.
    * Case 3: The type sepcifier is neither a pointer nor a reference.

예제1
```cpp
    auto x = 27;            // case 3 : x is value
    const auto cx = x;      // case 3 : x is value
    const auto& rx = x;     // case 1 : rx is a non-universal reference

    auto&& uref1 = x;       // x is int and lvalue, uref1 is int&
    auto&& uref2 = cx;      // cx is const int and lvalue, uref2 is const int&
    auto&& uref3 = 27;      // 27 is int and rvalue, uref3 is int&&
```

예제2 : array and function names
```cpp
    const char name[]="R. N. Briggs";

    auto arr1 = name;               // const char*
    auto& arr2 = name;              // const char (&)[13]

    void someFunc(int, double);

    auto func1 = someFunc;          // void(*)(int, double)
    auto& func2 = someFunc;         // void(&)(int, double)
```

예외 : braced initializer
```cpp
    int x1 = 27;    //C++98
    int x2(27);     //C++98
    int x3 = {27};  //C++11
    int x4{27};     //C++11
    /* all value is int 27 */
```
```cpp
    auto x1 = 27;
    auto x2(27);
    // x1, x2 work same above

    auto x3 = {27};
    auto x4{27};
    // x3, x4 are type std::initializer_list<int>, value is {27}

    auto x5 = {1,2, 3.0} // error,
    //단, std::initializer_list로는 deduction 이 성공.
    //T를 deduction 하는 과정에서 실패한 거. template type deduction 에서 T 는 한가지 타입이어야 하므로
```
하지만 template type deduction 에서는 이 initializer_list가 적용되지 않는다.
```cpp
    template<typename T>
    void f(T param);
    f({1, 2, 3}); // error, {} 가 T로 initalize될 수 없음

    template<typename T>
    void f(std::initializer_list<T> initList); // success, T를 int로 deduction
```

 * 바보같이 std::initializer_list 로 타입 선언 하지말자. auto를 쓰고 꼭 해야할 때만 하자.
    * 꼭 해야할 때를 알고시펑? -> [Item7]로 와

### 정리
 * auto type deduction 의 동작은 template type deduction 이랑 같음.
    * 예외 1: braced initializer일 때는 std::initializer_list<T> 로 함.
 * function return type과 lamda parameter에서 auto 는 template type deduction 을 따른다.


## Item 3: Understand decltype
<<<<<<< HEAD
쉽게 이해하면 auto는 선언된 형식으로부터 추론, decltype은 값으로 부터 타입을 추론한다고 생각하면 된다.
 * auto : template에 대한 type deduction
 
 * 주된 사용 : ParamType 에 따른 return type의 추론. 
 

## Item 4: Know how to view deduced types.

### IDE Editor 쓰셈
### Compiler diagnostics
컴파일 옵션으로 no-warning 같은거 때리지 말고 평소에 잘 보셈
### Runtime Output
 * printf 쓰지마
 * typeid(var).name()
    * 대신 컴파일러마다 다름
    * Clang, GNU
        * 약자로 씀
        * int -> i
        * pointer const -> PK
    * Microsoft
        * human readable 하게 찍어줌
    * template function 안에서
    ```cpp
        template<typename T>
        void f(const T& param){
            using std::cout;
            cout << "T = " << typeid(T).name() << endl;
            cout << "param = " << typeid(param).name() << endl;
        }
        //result 둘다 : PK6Widget (const Widget *)
    ```
        * 그런데 여기서 param은 const Widget* const &인데, 잘못 나오네?
            * template-function에서 typeid().name()은 무조건 by-value 로만 인식함. 
                * reference-ness 무시, volatile, const 무시.
                
            * 근데 IDE도 믿을만한게 못됨. ㅋㅋ
        * typeid, IDE 모두 못 믿을 땐, Boost.TypeIndex
    ```cpp
        #include<boost/type_index.hpp>
        
        template<typename T>
        void f(const T& param){
            using std:cout;
            using boost::typeindex::type_id_with_cvr;
            
            cout << "T = " << type_id_with_cvr<T>().pretty_name() << endl;
            cout << "param = " << type_id_with_cvr<decltype(param)>().pretty_name() << endl;
    ```
        * 이렇게 하면 위의 예제의 param 이 제대로 Widget const* const& 가 나옴.
        
### 요약
 * Deduced type 확인하는 방법 : IDE, compiler error/warning message, Boost.TypeIndex lib.
 * 원리를 이해하고 있으셈
=======
 * auto : template에 대한 type deduction
 * decltype : 값으로 type deduction

 * UseCase
    * 주로 ParamType에 대해서 return type 이 결정될 때 많이 쓴다.

예제
```cpp
    template<typename Container, typename Index>
    auto authAndAccess(Container& c, Index i) -> decltype(c[i]){
        authenticateUser();
        return c[i];
    }
```
 * 예제 설명
    * return type 이 auto인 것은 type deduction 이랑 관련 없다.
    * trailing return type 문법 : return type 은 parameter 리스트와 그에 따른 -> 뒤의 값을 따른다.

 * return type & lamda
    * C++11 : single statement lamdas to be deduced.
    * C++14 : all lamdas and all functions

예제2 : C++14
```cpp
    template<typename Container, typename Index>
    auto authAndAccess(Container& c, Index i){ //C++14 don't need '->'
        authenticateUser();
        return c[i];
    }

    std::deque<int> d;
    authAndAccess(d,5) = 10; //error
```
 * 문제점 auto는 referenceness를 제거 하기 때문에 c[i]는 값으로만 사용할 수 밖에 없다.
    * d[5]는 int& 를 반환하지만 auto 때문에 int를 return 함.
    * **Q : 그럼 예제1 에서는 int&가 리턴되는거야?**
        * C++11에서는 decltyp(expression)이 expression 을 가감하지 않고 그대로 추론해준다.
        * 위 문제는 C++14에서 ->decltype이 없는 경우에 해당한다.

예제3 : 해결책
```cpp
    template<typename Container, typename Index>
    decltype(auto) authAndAccess(Container& c, Index i){ //C++14 don't need '->'
        authenticateUser();
        return c[i];
    }
```
 * 해결 : decltype(auto)
    * 이렇게 하면 auto가 무엇이든간에 decltype으로 추론함. -> T& 그대로 리턴함.
    * auto쓰는 모든 곳에 적용 가능

예제4 : decltype(auto) type variable definition
```cpp
    Widget w;
    const Widget& cw = w;
    auto myWidget1 = cw;            //type is Widget
    decltype(auto) myWidget2 = cw; // type is const Widget&
```

예제 5 : lvalue, rvalue judgement by template
```cpp
    template<typename Container, typename Idex>
    decltype(auto) authAndAccess(Container& c, Index i);
```
 * 선언만 보고 return 이 어떤 타입이 올지 추론
    * lvalue-reference to non-const 를 받았으므로 수정가능한 element를 던질 것이다.
    * 단, rvalue를 pass하는 건 불가능하다.
        * rvalue 는 lvalue reference랑 바인드 할 수 없으므로
        * rvalue 쓰는게 말이 안되는게, rvalue로 들어오면 임시객체이므로 해당 statement 가 끝나면 dangled pointer 가 될 거다.

예제6 : want to get rvalue
```cpp
    std::deque<std::string> makeStringDeque(); //factory function
    //copy of 5th element of deque
    auto s = authAndAccess(makeStringDeque(), 5);
```
 * 이렇게 쓰고 싶으면 rvalue, lvalue reference 케이스 모두 Overload 하셈.
 * 근데 유지보수하기 힘드니까 universal reference 쓰셈 (자세한건 [Item24])
    * 근데 이렇게 하려면 리턴을 forward해야함 (자세한건 [Item25])
```cpp
    template<typename Container, typename Idex>
    decltype(auto) authAndAccess(Container&& c, Index i){
        authenticateUser();
        return std::forward<Container>(c)[i];
    }
```

 * decltype이 예상한데로 동작 안하는 경우도 드물게 있어!
    * Names are lvalue expression : works fine.
    * lvalue expressions : 이건 decltype 이 항상 lvalue reference로 인식해버려

예제 : lvalue expression to decltype
```cpp
    decltype(auto) f1(){ int x = 0; return x;}     //return int
    decltype(auto) f2(){ int x = 0; return (x);}   //return int&
```
 * 여기서 f2의 리턴은 local 변수의 reference. 따라서 코드블럭이 끝나기 때문에 리턴된 이후에는 이상한 주소 참조.
 * 그래서 [Item4] 에서 말하는 거처럼 decltype(auto)쓸 때는 주의깊게 확인해.
 * 근데 normal한 경우에는 대부분 예상한대로 동작하고 사용성/성능에 좋아.

### 요약
 * decltype almost always yields the type of a variable or expression without any modification.
 * For lvalue expressions of type T other than names, decltype always reports a type of T&.
 * C++14 supports decltype(auto), which, like auto, deduces a type from its initializer, but it performs the type deduction using the decltype rules.

>>>>>>> 6654f95980c6c78d2a947c5a8f682710fb3b19d5
