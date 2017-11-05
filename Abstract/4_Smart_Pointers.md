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


## Item 19 : Use std::shared_ptr for shared-ownership resource management.

포인터의 life time management를 해준다!

shared_ptr 객체에 대한 사용이 끝나면, destruction 과정에서 원본 객체도 detroy 해준다.

대신 object 의 destruction timing은 deterministic

constructor 가 불릴 때 count 증가한다. destructor 가 불릴 때 count 가 감소한다.
 * 감소할 때, use count 가 0이면 원본에 대한 destroy 과정을 진행한다.

= operator 는 left value 에 대해서 count 감소, right value 에 대해서 count 증가 한다.

성능문제
 * twice size of a raw pointer : raw pointer + reference count
 * Memory for the reference count must be dynamically allocated.
    * make_shared를 쓰지 않았을 때.
 * Increments and decrements of the reference count must be atomic.
 * Move construction & Move copying is faster than ordinal construction & copying.
    * move 하면 source object 는 nullptr이 됨.
    * reference count 에 대한 조작이 필요 없기 때문에.

이거도 custom deleter를 지원함.
 * unique_ptr과의 차이점은, unique_ptr은 deleter의 타입이 smart pointer의 타입의 한 부분인데, shared_ptr 은 그게 아님.
    * type선언에 안들어가고 걍 parameter로만 들어간다는 말.

shared_ptr custom deleter 예제
```cpp
auto loggingDel = [](Widget* pw){
                        makeLogEntry(pw);
                        delete pw;
                        };

std::unique_ptr<Widget, decltype(loggingDel)> upw(new Widget, loggingDel);
std::shared_ptr<Widget> spw(new Widget, loggingDel);

```

### unique ptr과의 차이점 1
shared_ptr은 deleter가 type 에 포함되어 있지 않기 때문에 좀 더 flexible 하다.
 * deleter가 다르더라도 container에 같이 넣을 수 있어.
    * **Q : 그러면 unique_ptr은 왜 type에 들어가도록 만들었지?**

```cpp
auto customDeleter1 = [](Widget *pw){...};
auto customDeleter2 = [](Widget *pw){...};

shared_ptr<Widget> pw1(new Widget, customDeleter1);
shared_ptr<Widget> pw2(new Widget, customDeleter2);

std::vector<std::shared_ptr<Widget>> vpw{ pw1, wp2};

pw1 = pw2;
```

### unique ptr과의 차이점 2
custom deleter의 사이즈가 object size에 영향을 안미침. 무조건 2 pointer size
 * function object가 heap에 잡히고, shared_ptr 은 그걸 pointing 만 한다. (unique_ptr은 자기가 가짐)

shared_ptr의 메모리구성
 * ptr to T
 * ptr to control block
    * reference count
    * weak reference count
    * other data
        * custom deleter
        * allocator

shared_ptr의 control block과 관련된 rule
 * std::make_shared ([Item 21]) always creates a control block.
 * A control block is created when a std::shared_ptr is constructed from a unique-ownership pointer (i.e., a std::unique_ptr or std::auto_ptr).
    * unique_ptr 로 부터 shared_ptr 받을 때, unique_ptr의 ownership 은 nullptr 이 됨.
 * When a std::shared_ptr constructor is called with a raw pointer, it creates a control block.
    * 이미 control block을 가진 pointer (shared_ptr, weak_ptr) 을 parameter로 받아서 shared_ptr 객체가 create 되는 경우는 이미 가진 control block을 사용함.

잘못된 예제 1 : 하나의 raw pointer로 부터 2개의 control block 이 사용됨 
```cpp
auto pw = new Widget; //raw pointer  -그런데 shared_ptr쓸 거면서 이렇게 먼저 raw pointer를 만들어 놓는 것도 문제.
std::shared_ptr<Widget> spw1(pw, loggingDel);
std::shared_ptr<Widget> spw2(pw, loggingDel);
```

문제점 :  위의 룰에 따라 하나의 raw pointer로 부터 별도의 control block 을 가지는 두개 이상의 객체를 생성하게 되면, 하나의 raw pointer에 대해서 destroy가 두 번 이상 일어나게 된다. (SEGEV fault 남)
 * std::make_shared 를 쓰셈
    * 그런데 make_shared를 쓰면 custom deleter를 못 씀.
    * 그러면 make_shared 블럭 안에서 new 를 때리셈
     
수정 : custom deleter를 쓰고 같은 control block 을 쓰도록 만드는 예제 
```cpp
std::shared_ptr<Widget> spw1(new Widget, loggingDel);
std::shared_ptr<Widget> spw2(spw1);
```

잘못된 예제 2 : shared_ptr을 담을 곳에 this를 넘김
```cpp
std::vector<std::shared_ptr<Widget>> processedWidgets;
class Widget{
public:
    void process(){
        processedWidgets.emplace_back(this);
        //emplace_back()에 대해서는 [Item 42]에서 배운다.
    }
};
```
shared pointer로 담을 컨테이너에 this를 넘기면 안돼!
 * this를 받아서 shared_ptr의 constructor 가 불리기는 하지만.
 * process 함수가 불릴 때마다 같은 pointer의 다른 shared_ptr (다른 control block을 가짐)이 생김.
    * 원하는 동작이 아님. 잘 못된 예제1과 같은 상황이 발생함.
  
수정 : enable_shared_from_this로 해결한다.
```cpp
class Widget : public std::enable_shared_from_this<Widget>{
public : 
    void process(){
        processedWidgets.emplace_back(shared_from_this());
    }
};
```
이렇게 하면 또 다른 shared_ptr 객체가 들어가더라도 같은 control_block을 쓰는 객체가 들어간다.
 * shared_from_this는 현재 객체의 control_block을 look up 해와서 해당 control_block을 refer하는 객체를 new한다.
    * 단, 해당 객체를 가진 shared_ptr이 없는 상태에서 하면 exception. 이거 보장 해줘야함.
    
shared_from_this()가 원본 shared_ptr 객체가 있는 상태에서 불리는 것을 보장해주는 예제
```cpp
class Widget : public std::enble_shared_from_this<Widget>{
public:
    template<typename... Ts>
    static std::shared_ptr<Widget> create(Ts&&... params);
private:
    Widget(); //ctor를 private 으로
};
```

shared_ptr 쓸 때 메모리 생각
 * control block이 잡아먹는다.
    * allocator, deleter
        * 따로 구현 안하면 작긴 하겠지만, 예측불가
    * 그 외에도 virtual function 들도 상속됨.
    * atomic reference count
    