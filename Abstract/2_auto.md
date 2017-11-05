# 2. auto
 * 왜 쓰나?
    * save typing
    * prevent correctness and performance issues (manual type declaration 할 때 발생할 수 있는)


## Item 5 : Prefer auto to explicit type declarations

예제 : dereferencing iterator
```cpp
    template<typename It>
    void dwim(It b, It e){
        typename std::iterator_traits<It>::value_type
        currValue = *b;
    }
```
 * 이 타입이 맞나?
    * local variable 이 closure 라면? 컴파일러만 아는데?

 * auto를 써보자.
    * auto는 initializer로부터 타입추론을 함.
    * initializer가 없으면 못함.
    * innitialize를 강제함.
```cpp
    int x;          //potentially uninitialized
    auto x2;        //error
    auto x3 = 0;    // well-defined
```

위의 예제를 바꾸면
```cpp
    template<typename It>
    void dwim(It b, It e){
        auto currValue = *b;
    }
```

예제 : compare 함수 in lamda
```cpp
    auto derefUPLess =
    [](const std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2)
    {return *p1<*p2;};
```
C++14에서는 parameter도 auto 를 쓸수있다
```cpp
    auto derefUPLess =
    [](const auto& p1, const auto& p2)
    {return *p1<*p2;};
```

 * closure var의 선언은 굳이 auto 필요 없지 않나
    * std::function 을 생각해보자

 * std::function
    * function pointer를 generalize한 개념
    * function 처럼 invoke되는 모든걸 가리킬 수 있다.(function 뿐만 아니라, callable object를 가리킬 수 있다.)

예제 : std::function
```cpp
   bool(const std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2);

   //std::function
   std::function<bool(const std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2)> func;

   //lamda로 closure를 function object 에 담으면
   std::function<bool(const std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2)> derefUPLess =
   [](const std::unique_ptr<Widget>& p1, const std::unique_ptr<Widget>& p2)
       {return *p1<*p2;};
```

 * auto와 std::function 이 다른점
    * auto는 closure와 같은 타입을 holding 하는 변수를 선언
        * closure가 가지는 메모리 만큼만 사용
    * std::function 은 자신이 원래 가지는 구조를 인스턴스화.
        * closure가 요구 메모리가 function 객체구조에 맞지 않으면 heap메모리를 할당
            * 거의 std::function 이 메모리를 더 씀
    * std::function 으로 나온 closure를 invoke 하는게 더 느림 (auto로 한 것 보다)
    * 즉, function pointer를 동적으로 쓰는 거는 auto가 코드 생산성이나 성능면이나 더 좋다.
        * 더불어 std::bind 대신에 lamda + auto로 써라.([Item 34])

 * auto는 type shortcut 이상을 해준다.

예제 : size_type
```cpp
    std::vector<int> v;
    unsigned sz = v.size();
    auto size = v.size();
```
 * 문제
    * 32-bit 에서 vector::size() 는 32bit, 64bit에서는 64bit
        * unsigned int는 32bit.

예제 : unordered map, const
```cpp
    std::unordered_map<std::string, int> m;
    for(const std::pair<std::string, int>& p : m){
        // do something
    }
```
 * 문제
    * p는 const선언 되어있음. 사실상 p는 std::pair<const std::string, int>& 타입. 따라서 모든 key 값에 대해서 m을 copy한 임시객체가 만들어지고. 그것을 deferencing 하게 됨.
        * m에 대해서는 아무 동작이 일어나지 않음.(기대한 것과 다르게 됨)
        * 걍 auto 쓰셈 : const auto& p : m

 * 결론 : type을 명시적으로 쓰는게 오히려 암시적인 type conversion 이 일어나게 만듬. auto를 쓰면 이런 걱정 없음. refactoring 에도 좋음(initialize 부분이 바뀌어도 안 바뀜)

## Item 6 : Use the explicitly typed initializer idiom when auto deduces undesired types.

예제 : std::vector<bool>
```cpp
    Widget w;
    bool highPriority = features(w)[5];
    processWidget(w, highPriority);         //works expected
    auto highPriority2 = features(w)[5];
    processWidget(w, highPriority2);        //works unexpected
```
 * 원리
    * std::vector는 bool을 1bit 로 담게 해준다
    * 그런데 []가 동작하기 위해서는 주소를 참조해야하는데
    * bit는 reference 할 수 없다.
    * 그래서 그 reference역할을 std:;vector<bool>::reference object가 해준다.
 * 분석
    1. bool highPriority = features(w)[5];
        * feature는 vector<bool> 을 리턴한다.
        * operator[] 는 std::vector<bool>::reference를 리턴한다
        * reference가 가진 bit를 highPriority에 넣는다.
    2. auto highPriority2 = features(w)[5];
        * feature는 vector<bool> 을 리턴한다.
        * operator[] 는 std::vector<bool>::reference를 리턴한다
        * 이 reference 로 타입추론을 한다. auto는 레퍼런스가 = operator에 대해서 어떤 동작을 imple했는지는 관심없다.
        * 그런데 feature() 가 리턴한 std::vecor<bool> 객체는 임시객체다. 따라서 temp::reference도 임시 객체의 값이다.
        * 할당이 끝나면 임시객체는 사라진다
        * processWidget() 함수에 들어갈 때 highPriority2 는 dangling pointer 가 된다.
 * std::vector<bool>::reference 와 같은 동작은 proxy class 의 예이다.
    * proxy class : 어떤 타입의 동작을 emulate하기 위한 클래스
    * smart pointer 류도 프록시 개념
    * Matrix로 + 연산하면, 그 Matrix자체가 떨궈지지 않고 Proxy객체가 결과로 만들어진다. (Sum<Matrix, Matrix>)
    * Proxy객체가 = 연산을 정의해서 원본으로 implicit conversion이 일어나는 것.
    * 그래서 이런식으로 implicit conversion 이 일어날 invisible proxy class류에는 auto 쓰지마셈.
        * 그래서 IDE에서 shared_ptr류를 리턴하면 타입추론이 제대로 안되는 건가?
    * auto는 proxy가 가진 타입은 모르고, proxy type 자체를 가져올거야

 * 그럼 이런 proxy class 인지 어떻게 알어?
    * document 를 보셈. 이건 특이한 동작이니까 명시해 놨을 것.
        * 너가 만든거 중에 이런 거 있으면 너도 명시해놓으셈
    * 헤더를 보셈
    ```cpp
    namespace std{
        template<class Allocator>
        class vector<bool, Allocator>{
        public:
            ...
            class reference { ... };
            reference operator[](size_type n);
        }
    }
    ```
 * 그래도 auto를 쓰고싶어?
    * static_cast를 쓰셈 (explicitly typed initializer idiom)
        * 변수 만드는거 강조하고 싶을 때도 쓰면 좋음 (implicit convert되는데 쓰이는 비용이 주는 성능이점이 있는건 아님)
        ```cpp
        double calcEpsilon();
        float ep = calcEpsilon();  //implicit covert
        auto ep = static_cast<float>(calcEpsilon());
        ```
        * int로 index 쓸때도 이렇게 쓰셈
        ```cpp
        auto index = static_cast<int>(d*c.size());
        ```

### 결론
 * "Invisible" proxy types can cause auto to deduce the "wrong" type for an initializing expression.
 * The explicitly typed initializer idiom forces auto to deduce the type you want it to have.