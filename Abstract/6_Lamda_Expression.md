# 6. Lamda Expression

람다는 function object 를 만드는 거야!

용어 정의
 * **lamda expression** :  그냥 하나의 expression 이야. \[](){} 로 되어있는.
 * **closure**: lamda를 통해 만들어진 runtime object를 말해.
    * capture mode 에 따라 closure는 capture 된 데이터의 copied object 나 reference 를 들고 있어.
 * **closure class** : 이걸로부터 하나의 closure object 가 만들어지는거.
    * Each lamda causes compilers to generate a unique closure class. 
    * The statements inside a lamda become executable instructions in the member functions of its closure class.
    
    
하나의 lamda 식으로 여러개의 closure object를 얻는 방법
```cpp
int x;
auto c1 = [x](int y){ return x*y > 55; }; 
// c1 is copy of the closure produced by the lamda
auto c2 = c1;       // c2 is copy of c1
auto c3 = c2;       // c3 is copy of c3
```

컴파일 타임 : lamdas and closure classes

런타임 : closures

## Item 31 : Avoid default capture mode.

Default capture mode in C++11.
 * by-reference
    * dangling pointer 만들 수 있어. 주의.
 * by-value
    * 위 문제로 부터 자유로운듯 하지만 아니야!
    * 내가 만든 closure 가 self-contained 인듯 하지만 아니야! 
    * (그래서 어쩌라고!)
    
### The Danger of default by-reference capture

closure 가 local var 의 reference 나 lamda 가 정의된 scope 에서 가능한 parameter 의 reference 를 받았을 때, 그런데 이 closure 가 해당 scope 을 벗어나서 쓰일 때, closure 안에 있는 reference 가 dangle 됨

예제
```cpp
using FilterContainer = std::vector<std::function<bool(int)>>; //bool(int)가 머징
FilterContinaer filters;
filters.emplace_back( [](int value){ return value % 5 == 0; } ) ; 
//* Item 42 : emplace_back 은 type deduction 이 여기서 일어나.
```

그런데 divisor 가 runtime 에 필요하다면 코드를 이렇게 짤 수 있어
```cpp
void addDivisorFilter(){
    auto calc1 = computeSomeValue1();
    auto calc2 = computeSomeVlaue2();
    auto divisor = computeDivisor(calc1, calc2);
    
    filters.emplace_back( [&](int value){ return value % divisor == 0;} );
    // * &divisor  를 넣어도 마찬가지
}
```
그런데 이러면 이 블럭을 넘어서 filter 에서 lamda 로 넣은 function 을 부르면 원하던 divisor 를 찾을 수 없음.
 * &divisor 를 넣어도 마찬가지
    * 대신  &divisor 를 쓰면 '아, divisor 의 lifetime 을 신경써야겠구나' 생각하게 만들 수는 있음
    
즉시 사용되고 마는 경우 (stl algorithm 함수들은 이걸 보장함) 는 lifetime don't care.
 * default by reference capture mode 도 전혀 오버헤드가 없어.
 
#### 해결방법 1 : default by value capture
```cpp
filters.emplace_back(
    [=](int value){ return value % divisor == 0; }
);
```
그런데 이렇게 하면, pointer 에 대해서 by value capture 를 하면 해당 원본 pointer 가 delete 되었을 때 closure 가 가진 pointer 가 dangle 되는건 여전히 막을 수 없어.

예제
```cpp
class Widget{
public:
    void addFilter() const{
        filters.emplace_back(
            [=](int value){ return value % divisor == 0; }
        );
    }
    
private:
    int divisor;
}
```
