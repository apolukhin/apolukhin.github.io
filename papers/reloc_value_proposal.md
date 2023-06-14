# Minimal Language Relocation

## TL;DR:

The paper proposes a **minimal** set of changes to allow C++ to have:
* Better safety by allowing non-nullable types with ownership transfer
* Better performance due to a way to fully transfer ownership and control of the destructor invocation
* Avoids relocation pitfalls (like double destruction or partial destruction)

Technical features:
* Overload resolution not changed and no new reference types added
* Truly zero-overhead relocation due to control over the destructor, while not introducing "partial" destructors
* Works only on function local variables of automatic duration
* Allows customization of relocation while it is still noexcept
* Mandatory (not an optimization hint for the compiler)

This paper is a mix of ideas from papers of Alisdair Meredith, Arthur O’Dwyer,
Barry Revzin, Brian Bi, Denis Bider, Ed Catmur, Joshua Berne, Mungo Gill,
Niall Douglas, Pablo Halpern, Sébastien Bini and many others. Many thanks for
providing a solid building blocks for this proposal, hopefully the resulting
building could stand a breeze.


## Introduction

Move semantic allows to transfer ownership of a resource. However it leaves the
moved-from value in scope and it could be accessed. Also, the moved-from value
provokes the type to have some 'empty state' and default constructor.
That may not match the domain problem:

```cpp
void do_something(resource r) {
  assert(r);  // no way to guarantee non empty state at compile time
  // ...
}
```

To have a strong guarantee and avoid the `assert(r)` there should be
a way to avoid an empty state, while still be able to relocate the object.
 

This could be achieved by by introducing a `relocate` semantics and an optional
customization of that relocation:

```cpp
struct resource {
    relocate resource(resource other) = default;    // Proposed

    resource(int input, int parameters);

    resource() = delete;
    resource(const resource&) = delete;
    resource(resource&&) = delete;
    resource& operator=(const resource&) = delete;
    resource& operator=(resource&&) = delete;

private:
  handle h_;
};


void example() {
  resource r{42, 100500};

  do_something(relocate r);   // Proposed
  // attempt to use `r` result in ill-formed program
}
```


## Semantics

### `relocate` keyword

`relocate` could be used on values with automatic storage duration from current
function:


```cpp
T value{};

T own_value = relocate value;          // Calls `relocate T(T)`, `value` could not be used any more, `~value` not called
T new_own_value = relocate own_value;  // Calls `relocate T(T)`, `own_value` could not be used any more, `~own_value` not called
```


It limits lifetime analysis to the current function only, avoiding the
double-desctruction issue of P2839R0:

```cpp
void foo(T& value) {
  T own_value = relocate value;  // Ill formed, lifetime of `value` is unknown
}
```

However unleashes a way to return owned value, skipping the destructor call:

```cpp
T foo() {
  const T value{};
  return relocate value;  // Calls `relocate T(T)`, `value` could not be used any more, `~value` not called
}
```

`relocate`ed objects do not bind to lvalue/const lvalue/rvalue/const rvalue/... references, object
could be relocated only into an automatic storage duration object:

```cpp
void foo() {
  T x;
  const T& tmp = relocate x;  // Ill formed
}
```

Some of the examples from P2785 apply:

```cpp
void foo(std::string str);
std::string get_string();
std::pair<std::string, std::string> get_strings();

std::string gStr = "static string";

void bar(void)
{
	std::string str = "test string";
	foo(reloc str); // OK: relocation will happen given that std::string has a reloc ctor
	foo(reloc gStr); // ill-formed: gStr does not have local storage

	std::pair p{std::string{}, std::string{}};
	foo(reloc p.first); // ill-formed: p.first is not a complete object, and not the name of variable

	foo(reloc get_string()); // ill-formed: not the name of variable
	foo(reloc get_strings().first); // ill-formed: not a complete object, and not the name of variable
}
```


### `relocate` of parts of an object

Relocation of parts for the type are allowed only in the `relocate constructor`
of the object.
Thus we avoid the problem of partial destruction of an object.


### `relocate` constructor

The idea of the `relocate` constructor is that it ends the lifetime of an
initial object, this giving the full power over optimizing the relocation:

Consider the simplified move constructor of std::unique_ptr:

```cpp
unique_ptr::unique_ptr(unique_ptr&& other)
  : ptr_(other.ptr_)
  , deleter_(std::move(other.deleter_))
{
  other.ptr_ = nullptr;  // Inform the destructor that there's nothing to destroy
}

unique_ptr sample(unique_ptr up) {
  return std::move(up);   // Calls unique_ptr(unique_ptr&& other)
  // destructor of `~up` is called if no copy-elision happened 
}
```

With relocation constructor we can safe some CPU cycles:

```cpp
relocate unique_ptr::unique_ptr(unique_ptr other) {
  ptr_ = relocate other.ptr_;
  deleter_ = relocate other.deleter_;

  // Destructor of `other` is not called
}

unique_ptr sample(unique_ptr up) {
  return relocate up;  // Calls `relocate unique_ptr(unique_ptr other)`
  // destructor of `~up` is **NOT** called 
}
```

### `relocate` constructor safety

* To avoid issues with failing to relocate some of the members the relocation
constructor requires every member of `other` to be explicitly relocated.

* To avoid long standing issue with members order initilization the relocation
constructor allows to initilize the member only in the order of declaration.

* To void long standing issue with suboptimal code due to the default
construction of members, the lifetime of a member in relocation constructor
starts only at the point of first initialization

* The relocation constructor is always `noexcept`

* The relocation constructor is implicitly `constexpr` and its body should be visible at the point of usage

Note that most of the time the relocation constructor should be defaulted by
user, so the above features are only required for corner cases of non trivial
relocation, where the object references itself or holds a reference/iterator/pointer
to one of the members.

To demonstrate all the features, let's take the a sample from P2839:

```cpp
struct S {
    small_vector<T>           d_v;
    small_vector<T>::iterator d_it;  // points to `d_v`

    relocate S(S src);
}
```

The relocation constructor code:
```cpp
relocate S::S(S src) {
  auto index = src.d_it - src.d_v.begin();
  d_v = relocate src.d_v;     // Starts the lifetime of d_v
  d_it(d_v.begin() + idx);    // Starts the lifetime of d_it
  relocate src.d_it;
  // no `~src` call
}
```

Some examples of ill formed relocation constructor of that type:

1.
```cpp
relocate S::S(S src) {
  auto index = src.d_it - src.d_v.begin();
  d_v = relocate src.d_v;
  d_it(d_v.begin() + idx);

  // Ill formed, `src.d_it` was not explicitly relocated
}
```

2.
```cpp
relocate S::S(S src) {
  d_it = relocate src.d_it;   // Ill formed, lifetime of `d_it` started before the lifetime of `d_v`
  d_v = relocate src.d_v;     
  
}
```

3.
```cpp
relocate S::S(S src) {
  S src2 = relocate src;   // Ill formed, `src` can not be relocated inside it's own relocation constructor
}
```

4.
```cpp
relocate S::S(S src) : d_v(relocate src.d_v) {  // Ill formed, initializer lists are not allowed in the relocating constructor
  // Ill formed, lifetime of `d_it` was not started
}
```

5.
```cpp
relocate S::S(S src) {
  d_v = relocate src.d_v;     
  // Ill formed, lifetime of `d_it` was not started
}
```


5.
```cpp
// `relocate S::S(S src);` not defined but declared
S x{};
S v = relocate x; // Ill formed
// ^ body or relocation constructor is forced to be "visible" to the compiler
// for better optimization (or not?)
```


Now to the allocation sample from P2839:

```cpp
struct T {
    allocator_type d_alloc;
    S              d_s;

    relocate T(allocator_type alloc, T src);
};
```


```cpp
relocate T::T(allocator_type alloc, T src) {
    // No need in `reloc_begin_destruction src;` the destructor is skipped

    d_alloc = relocate alloc;        // Starts the lifetime of `d_alloc`;
    d_s = S(alloc, reloc src.d_s);   // Starts the lifetime of `d_s`;
}
```

### `relocate` constructors overload resolution

When the compiler sees constructor invocation with a relocate variable of the type being
constructed (for example, `T v; T x(42, 14, relocate v)` then the compiler does the
overload resolution only on constructors that were marked as `relocate`, treating
all the variables as prvalue. If no
constructor found - the program is ill formed. Otherwise, constructors marked
with `relocate` do not participate in overload resolution.


### implicit `relocate` constructors

The papers does not propose defaulting a relocate constructors. Defaulting those
is a separate issue that requires separate investigation, due to many nontrivial
cases:

```cpp
// Empty class that could not be relocated

struct fast_mutex {
  void lock()   { futex_wait(reinterpret_cast<std::uintprt_t>(this), LOCK); }
  void unlock() { futex_wait(reinterpret_cast<std::uintprt_t>(this), UNLOCK); }

};
```  

Implicit relocation constructors are left for further proposals. Many of
`std::is_trivially_relocatable` like proposals already do a good job of
researching the problem.


### defaulted `relocate` constructors

Relocate constructor of the object could be defaulted iff every non static member
of the class has relocate constructor or each of the members is a trivial type. 


## std::vector relocation problem

The core idea of most of the proposals on relocation is to optimize the std::vector
logic of moving+destroying the elements. 

This proposal is different! If concentrates on security/usability issues of the
language, solves some of the basic performance issues and provides a foundation
for proposals that propose `std::is_trivially_relocatable` like traits. The
latter could check for relocate constructors triviality from this paper, and
provide an answer depending on that.


### `relocate` in libraries

`relocate` works only on local variables, so passing an object by reference or
even a forwarding reference prevents relocation:

```cpp
template <class T>
void push_back(T&& value) {
  auto v = relocate value;  // Ill formed
}
```

To deal with that issue a concept `would_relocate<T, Args...>` could be
implemented and additional function is added to the library:

```cpp
template <class T>
  requires would_relocate<T, T>
void push_back(T value) {
  auto v = relocate value;  // Fine
}


template <class T>
void push_back(T&& value) {
  auto v = relocate value;  // Ill formed
}
```

This option would burden library developers.

There is a less invasive (for the library developers) solution. Change the
forwarding reference rules, so that if the type was relocated, the forwarding
reference results "in a pass by copy":

```cpp
// Equivalent to `void push_back(T value)` signature if called as `push_back(relocate x)`
template <class T>
void push_back(T&& value) { 
  auto v = std::forward<T>(value);  // Old code is fine
}


template <class... Args>
void push_back(Args&&... value) { 
  new (storage ) T(try relocate value...);  // New code could relocate types
}
```

In the above example `try relocate` uses `relocate` if it is possible to relocate
the type and does `std::forward` like logic otherwise.

The second option also preserves the ABI stability as an explicit `relocate`
is required to instantiate a new signature.

Anyway, the new `try relocate` has multiple design solutions and could be
investigated and extended in separate papers.


## Comparison with other relocation proposals

|                         Feature           | This proposal |   P2839       |   D2785       |   N4158   |   P0023 |  P1144R7 and P2786R0 |  P1029 |
| ----------------------------------------- | ------------- | ------------- | ------------- | --------- | ------- | -------------------- |------- |
| Does not introduce new user-visible reference types    |          ✅  |    ❌        |     ❌        |     ✅   |  ✅     |  ✅                 |  ✅    |
| Safe automatic storage objects in function |         ✅  | ✅         | ✅              | ❌       | ❌      |  ❌                 | ❌   |
| Avoids double destruction                 |          ✅  |    ❌       |          ❌    | ✅       | ✅      |  ✅                 | ✅   |
| Does not introduce partial destruction    |          ✅  |    ❌        |     ✅        |     ✅   |  ✅     |  ✅                 | ✅     |
| Allows non-nullable types                 |          ✅  |    ✅        |     ✅        |     ❌   |  ❌     |  ❌                 | ❌     |
| Allows relocation customization           |          ✅  |    ✅        |     ❌        |     ❌   |  ❌     |  ❌                 | ❌     |
| Allows relocation customization           |          ✅  |    ✅        |     ❌        |     ❌   |  ❌     |  ❌                 | ❌     |
| Solves the std:vector relocation problem  | ✅ with std::is_trivially_relocatable like proposals |    ✅        |     ✅        |     ✅   |  ✅     |  ✅                 | ✅     |

TODO: double check the proposals, I'm definitely screwed something up ^



## Ideas for separate papers

* implicit default relocating constructors
* `std::is_trivially_relocatable`
* extension for function local references
* extension for external references

The last idea would require additional lifetime analysis, that ensures that
after leaving the function references have objects with started lifetime and the
objects have no const members and so forth:

```cpp
template <class T>
  requires NothingBadHappensDuringRelocation<T>
void swap(T& x, T& y) noexcept {
  T tmp = relocate x;         // Calls T(T~), `x` could not be used any more, it's lifetime is ended
  new (&x) T(relocate y);     // Calls T(T~), starts lifetime of `x`; `y` could not be used any more, it's lifetime is ended
  new (&y) T(relocate tmp);   // Calls T(T~), starts lifetime of `y`
}
```

Because of that possible extension, this paper forces the relocation constructor to
be noexcept. The noexcpetness of the constructor could be relaxed later,
if there's interest in such feature.


