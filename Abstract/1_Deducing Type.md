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


