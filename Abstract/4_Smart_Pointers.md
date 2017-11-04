# Chapter 4. Smart Pointers

## raw pointer 를 쓰기 어려운 이유
 1. Its declaration doesn't indicate whether it points to a single object or to an array.
 2. Its declaration reveals nothing about whether you should destroy what it points to when you're done using it, i.e., if the pointer owns the thing it points to.
 3. If you determine that you should destroy what the pointer points to, there's no way to tell how. Should you use delete, or is there a different destruction mechanism (e.g., a dedicated destruction function the pointer should be passed to)?
 4. If you manage to find out that delete is the way to go, Reason 1 means it may not be possible to know whether to use the single-object form ("delete") or the array form ("delete []"). If you use the wrong form, results are undefined.
 5. Assuming you ascertain that the pointer owns what it points to and you discover how to destroy it, it's difficult to ensure that you perform the destruction exactly once along every path in your code (including those due to exceptions). missing a path leads to resource leaks, and doing the destruction more than once leads to undefined behavior.
 6. There's typically no way to tell if the pointer dangles, I.e. points to memory that no longer holds the object the pointer is supposed to point to. Dangling pointers arise when objects are destroyed while pointers still point to them.

이런 단점 극복하기 위해 Smart Pointers (raw pointer의 wrapper)
 * std::auto_ptr
 * std::unique_ptr
 * std::shared_ptr
 * std::weak_ptr

auto_ptr
 * C++98 부터 있으나, deprecated 됨
     * move operation 이 제대로 안됨
     * 컨테이너에 못 넣음
     * copy 하면 null 로 세팅됨.
     * 98로 컴파일 할때만 쓰셈
     * C++11에서는 unique_ptr 을 쓰셈

3개가 모두 constructor 만 같고 다 다른기 때문에 잘 알고 써야함

## Item 18 : Use std::unique_ptr for exclusive-ownership resource management.
raw pointer 와 사이즈 같고 가장 빠름.

exclusive ownership semantic.

move를 하면 source pointer가 null이 됨

copy가 안됨.

move-only type

destruction 에서 리소스가 지워짐(한번에 하나만 가지니까), 이 때 raw pointer도 delete됨.

use case
 * factory function
    * unique_ptr이 destory 될 때, raw pointer 를 자동 delete 보장. (자원 해제를 신경 안써도 됨)
```cpp
class Investment{...};
class Stock : public Investment {...};
class Bond : public Investment {...};
class RealEstate : public Investment {...};
```
 * ownership migration scenario
    * flow에 따라서 ownership이 전파됨. 마지막 사용한놈이 move 안하면 destory 됨.

construction 할 때, custom deleter 를 function object로 넣을 수 있어. destroy에서 이 함수를 불러줌.

```cpp
auto delInvmt = [](Investment* pInvestment){
    makeLogEntry(pInvestment);
    delete pInvestment;
}

template<typename... Ts>
std::unique_ptr<Investment, decltype(delInvmt)> //return type
makeInvestment(Ts&&... params){
    std::unique_ptr<Investment, decltype(delInvmt)> pInv(nullptr, delInvmt);
    if(/*Stock*/){ pInv.reset(new Stock(std::forward<Ts>(parmas)...) ); }
    else if(/*Bond*/){ pInv.reset(new Bond(std::forward<Ts>(parmas)...) ); }
    else if(/*RealEstate*/){ pInv.reset(new RealEstate(std::forward<Ts>(parmas)...) ); }
    return pInv;
}
```
설명
 * raw pointer를 assign 하면 implicit conversion 일어남
    * 그런데 smart pointer류는 implicit conversion 을 방지함 그래서 new대신에 reset해야함.
 * new 를 할때 parameter는 perfect-forwarding으로 함
 * custom deleter 는 raw pointer 타입을 parameter로 받아야 함.
 * 단, Investment 는 virtual Destructor 를 가져야 함.
    * 그래야 상속받은 concrete type 의 destructor가 불림.
 * C++14는 return type 을 auto로 해서 더 깔끔하게 만들 수 있어.
 * Custom deleter 를 넣으면 unique_ptr의 size는 raw pointer보다 커짐. ( 1 word -> 2word)
     * delete function 이 어떤 state를 가지냐에 따라서도 더 커질 수 잇음
        * no capture lamda function 인 경우는 1. (stateless)
        * capture less lamda function 으로 할 것을 추천!
     * 이 예제에서 capture less lamda function 으로 하면 리턴 타입의 사이즈는 Investment*, 일반 function pointer 로 하면 Investment* size + function pointer size


위의 factory예제는 Pimpl Idiom 으로 알려져 있음. 근데 어떤 케이스에서는 덜 직관적, 이거 [Item 22]에서 다룰거야.

unique_ptr<T> 와 unique_ptr<T[]> 를 다 지원, 하지만 [] operator 는 지원 안하기 때문에 array의 dereferencing 하기 어려울 수 있어.
 * 그래서 unique_tpr<T[]> 를 쓸 경우는 기존의 C-like API에서 heap array에 대한 raw pointer를 리턴한거 받아 쓰는 경우에만 쓰기를 권장

unique_ptr을 받아서 shared_ptr로 만들 수 있어. 그러니까 우선 factory에서는 unique_ptr을 던지고 받아 쓰는 쪽에서 결정하도록 해. [Item 19]

### 요약
 1. std::unique_ptr is a small, fast, move-only smart pointer for managing resources with exclusive-ownership semantics.
 2. By default, resource destruction takes place via delete, but custom deleters can be specified. Stateful deleters and function pointers as deleters increase the size of std::unique_ptr objects.
 3. Converting a std::unique_ptr to a std::shared_ptr is easy.