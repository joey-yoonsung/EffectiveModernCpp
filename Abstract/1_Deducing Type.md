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
  * auto : template에 대한 type deduction
