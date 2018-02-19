// Copyright Antony Polukhin, 2018.
//
// Distibuted as Public domain. Use as you wish.
//
// Also distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


// This file is a small test for LWG issue on string_view assignments.

#include <initializer_list>

template <class Char, class Traits, class Allocator>
struct basic_string;

template<class charT, class traits>
struct basic_string_view {

    // [string.view.template]

    // 24.4.2.1, construction and assignment
    basic_string_view() noexcept;
    basic_string_view(const basic_string_view&) noexcept = default;
    basic_string_view& operator=(const basic_string_view&) noexcept = default;
    template <class A>
    basic_string_view& operator=(const basic_string<charT, traits, A>&&) = delete;
    basic_string_view(const charT* str);
    basic_string_view(const charT* str, unsigned len);
};

template <class charT, class traits, class Allocator>
struct basic_string {
    using size_type = unsigned;

    // 24.3.2.2, construct/copy/destroy
    basic_string() noexcept(noexcept(Allocator())) : basic_string(Allocator()) { }
    explicit basic_string(const Allocator& a) noexcept;
    basic_string(const basic_string& str);
    basic_string(basic_string&& str) noexcept;
    basic_string(const basic_string& str, size_type pos, const Allocator& a = Allocator());
    basic_string(const basic_string& str, size_type pos, size_type n, const Allocator& a = Allocator());

    template<class T>
    basic_string(const T& t, size_type pos, size_type n, const Allocator& a = Allocator());
    explicit basic_string(basic_string_view<charT, traits> sv, const Allocator& a = Allocator());
    basic_string(const charT* s, size_type n, const Allocator& a = Allocator());
    basic_string(const charT* s, const Allocator& a = Allocator());
    basic_string(size_type n, charT c, const Allocator& a = Allocator());
    template<class InputIterator>
    basic_string(InputIterator begin, InputIterator end, const Allocator& a = Allocator());
    basic_string(std::initializer_list<charT>, const Allocator& = Allocator());
    basic_string(const basic_string&, const Allocator&);
    basic_string(basic_string&&, const Allocator&);
    ~basic_string();
    basic_string& operator=(const basic_string& str);
    basic_string& operator=(basic_string&& str) noexcept;
    basic_string& operator=(basic_string_view<charT, traits> sv);
    basic_string& operator=(const charT* s);
    basic_string& operator=(charT c);
    basic_string& operator=(std::initializer_list<charT>);

    operator basic_string_view<charT, traits>() const noexcept;
};

struct fake_char_traits{};
struct fake_alloc{};

using string_view = basic_string_view<char, fake_char_traits>;
using string = basic_string<char, fake_char_traits, fake_alloc>;

////////// Tests:

string foo();
const string foo_const();
void bar(string_view sw);

int main () {
    // Checks on char* and string_view assignment/construction
    const char* char_data = "Hello word";
    char nonconst_data[] = "Hello word";
    string_view sw = "Hello";
    sw = "world";
    sw = {"Hello", 3};
    sw = {char_data};
    sw = {char_data, 3};
    sw = sw;
    sw = nonconst_data;
    sw = {nonconst_data};
    sw = {nonconst_data, 3};
    sw = static_cast<string_view&&>(sw);
    string_view{sw};
    string_view{static_cast<string_view&&>(sw)};


    // Checks on std::string assignment/construction
    const string& s = string{};
    sw = s;
    bar(foo());                 // Must compile! This is a valid use case.
    bar(foo_const());           // Must compile! This is a valid use case.
    {   // Must compile, however those are not valid usages.
        string_view sw10{s};
        string_view sw11{string{}};      
        string_view sw12{foo()};
        string_view sw13{foo_const()};
        string_view sw14{string{}};
        string_view sw15 = string{};
        string_view sw16{foo_const()};
        string_view sw17 = foo_const();
    }

    // Must fail at compile time:
    //sw = string{};
    //sw = foo();
    //sw = foo_const();
    //bar(sw = foo());
}
