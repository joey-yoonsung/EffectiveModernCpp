공부하면서 Agent 코드에 적용해야할 / 할만한 것들

## 1 Deducing Type
### Item 1: Understand template type deduction
### Item 2: Understand auto type deduction
### Item 3: Understand decltype
### Item 4:
 * Boost Type으로 상속관계 판단 할까?
### Item 5 : Prefer auto to explicit type declaration
### Item 6 : Use the explicitly typed initializer idiom when auto deduces undesired types.
### Item 7 : Distinguish () and {} when creating objects.
 * initialize : atomic int값 초기화 할 수 있다 (), {} 이걸로
### Item 9: Prefer alias declarations to typedefs.
 * 복잡한 선언들 std::list<shared_ptr<GuidBaseTraceWrapper>> 이런거 alias로 간단하게 만들자.
### Item 10 : Prefer scoped enums to unscoped enums.
 * native message 의 type flag 를 scoped type:underlying_type 써서 하자. 객체도 scoped enum 타입으로 갖게 하자.
