#include <experimental/string_view>

using namespace std::experimental;


//  Compile time error:
//  > error: constexpr variable 'service' must be initialized by a constant expression
//  > constexpr string_view service = "HELLO WORD SERVICE";
//  >                       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  > experimental/string_view:110:39: note: non-constexpr function 'length' cannot be used in a constant expression
//
//constexpr string_view service = "HELLO WORD SERVICE";
constexpr string_view service_manual0{"HELLO WORD SERVICE", 18};


template <class T>
constexpr string_view StringView(const T& v) noexcept {
    return { v, sizeof(v) / sizeof(v[0]) - 1};
}

constexpr string_view service_manual = StringView("HELLO WORD SERVICE");


template <class T>
struct ce_char_triat : std::char_traits<T> {
    using char_type = typename std::char_traits<T>::char_type;

    static constexpr std::size_t length(const char_type* s) noexcept {
        return __builtin_strlen(s);
    }
};

// OK
using ce_string_view = basic_string_view<char, ce_char_triat<char>>;
constexpr ce_string_view service = "HELLO WORD SERVICE";


#include <cassert>
#include <string>

int main() {
    assert(service.length() == service_manual.length());
    assert(service.length() == service_manual.length());
    assert(service_manual0 == service_manual);
    assert(std::string{service.data()} == std::string{service_manual.data()});
}
