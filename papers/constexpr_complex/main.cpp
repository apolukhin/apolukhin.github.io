#include <constexpr_complex.hpp>


template <class T>
constexpr bool test_complex_impl2(T v1, T v2) {
    using std::complex;

    complex<T> c1{v1, v2};
    auto c2 = c1 * c1;

    complex<T> cc1{v2, v1};
    auto c3 = cc1 * c2 / c1;
    c3 /= cc1;

    return (c3 - c1) == complex<T>{};
}

template <class T>
constexpr bool test_complex_impl() {
    return test_complex_impl2<T>(1.0, 0.0)
        && test_complex_impl2<T>(1.0, 1.0)
        && test_complex_impl2<T>(0.0, 1.0)

        && test_complex_impl2<T>(-1.0, 0.0)
        && test_complex_impl2<T>(-1.0, -1.0)
        && test_complex_impl2<T>(0.0, -1.0)

        && test_complex_impl2<T>(-1.0, 1.0)
        && test_complex_impl2<T>(1.0, -1.0)

        && test_complex_impl2<T>(1024.0, 0.0)
        && test_complex_impl2<T>(1024.0, 1024.0)
        && test_complex_impl2<T>(0.0, 1024.0)

        && test_complex_impl2<T>(-1024.0, 0.0)
        && test_complex_impl2<T>(-1024.0, -1024.0)
        && test_complex_impl2<T>(0.0, -1024.0)

        && test_complex_impl2<T>(-1024.0, 1024.0)
        && test_complex_impl2<T>(1024.0, -1024.0)

        && test_complex_impl2<T>(-1024.0, 4.0)
        && test_complex_impl2<T>(4.0, -1024.0)

        && test_complex_impl2<T>(2048.0, 4096.0)
        && test_complex_impl2<T>(1.0e2, 4096.0)
    ;
}

constexpr bool test_complex() {
    return test_complex_impl<float>()
        && test_complex_impl<double>()
        && test_complex_impl<long double>()
    ;
}

#include <iostream>
int main() {
    constexpr bool res = test_complex();
    static_assert(res, "");

    constexpr std::complex<double> c1 { 1.0, 0.0 };
    constexpr std::complex<double> c2 {};

    // Failures:
    // * std::find is not constexpr<span class="changed-deleted">
    // * std::array::rbegin(), std::array::rend() are not constexpr
    // * std::array::reverse_iterator is not constexpr</span>
    constexpr auto c3 = -c1 + c2 / 100.0;
    static_assert(norm(-c3) == norm(c1), "");

    {
        using std::complex;
        constexpr auto v1 = 1.0e25;
        constexpr auto v2 = 2.0e50;
        constexpr complex<double> c1{v1, v2};
        constexpr auto c2 = c1 * c1;

        constexpr complex<double> cc1{v2, v1};
        constexpr auto c3 = (cc1 * c2) / (c1 * cc1);
        std::cout << c1 << '\n';
        std::cout << c3 << '\n';
    }
    constexpr bool b = test_complex_impl2<double>(1.0e25, 2.0e50);
    //static_assert(b, ""); // std::numeric_limits<double>::epsilon() rounding error
    (void)b;
}
