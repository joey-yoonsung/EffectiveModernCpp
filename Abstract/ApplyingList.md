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
### Item 19 : Use std::shared_ptr for shared-ownership resource management.
 * boost::asio::service 로 trace 객체로 넘길 때, move로 넘기면 더 빠르지 않을까?
 * shared_ptr로 만들 모든 객체 create 패턴으로 만들도록 하자.
## Item 20 : Use std::weak_ptr for std::shared_ptr-like pointers that can dangle.
 * Tree  shared_ptr - weak_ptr 을 unique_ptr - raw ptr로 바꿀것
 * 아니야 그러면 소유권이 이동되어서 원래 tree에 있는 객체가 null이 됨.
 * 나는 destroy 시점이 명확하고, 안쓰이는게 보장되니까 그냥 raw-pointer 쓰면 대.
## Item 21 : Prefer std::make_unique and std::make_shared to direct use of new.
 * 근데 나는 멍청하게 make_shared 쓰면서 클래스 이름 두번 썼네?
