# Intorduction
p1~

## Terminology and Convention
### lvalue && rvalue
 * move semantic
    * move는 ravlue 에 대해서만 적합.

 * lvalue, rvalue 판단기준
    * lvalue : 주소가 필요해?
    * rvalue reference type (&& var)의 파라메터가 있을 때 이놈은 원래 lvalue 인 놈 가지고 ravalue 받고 싶어서 쓰는거
    ```cpp
    class Widget{
    public:
        Widget(Widget&& rhs); //넘기는 rhs 자체는 lvalue, 하지만 rvalue reference type 으로 해당 메소드 안에서는 rvalue 로 온다.
    }
    ```

 * initializing
    * 같은 타입의 다른 객체와 함께 초기화 할 때는 copy로 가져와야 함
        * rvalue는 move constructed
        * lvalue는 copy constructed

 * function call
    * function을 콜 할때 입장에서는 argument.
    * function definition 쪽에서는 parameter.
    ```cpp
    void seomFunc(widget w);
    Widget wid;
    someFunc(wid);
    someFunc(std::move(wid));
    ```
        * 여기서 w는 parameter.
            * parameter는 lvalue
        * wid, move(wid) 는 argument.
            * argument 는 넣을 때 rvalue 또는 lvalue 로 초기화 됨.
            perfect-forwarding이라고 한다. [item30]에서 다룸

    * well-designed function은 exception safe하다.
        * 최소한의 basic exception safety guarantee(basic guarantee)가 있다.
        * strong guarantee : 메소드 수행중 익셉션이 생기더라도 caller에게 프로그램에 해당 function 을 수행 전상태로 남겨주는 것

 * function object
    * object의 정의 : operator() 를 멤버 함수로 지원하는 모든 타입의 객체
    * 다른말로 하면 모든 객체는 function 처럼 쓸 수 있다.
    * callable object : C-like function pointer.를 멤버로 가진 객체
    * closures 로 알려진 lamda expression 으로 생성될 수 있다
        * closure와 lamda expression 의 구분. template and template function. Ditto, class templates and template classes.

