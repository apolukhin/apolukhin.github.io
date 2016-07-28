#include <string>
#include <cstring>

// Simple constexpr char trait
template <class T>
struct ce_char_trait : std::char_traits<T> {
    using base_t = std::char_traits<T>;
    using size_t = std::size_t;
    using char_type = typename base_t::char_type;

    static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept {
        for (size_t i = 0; i < n; ++i) {
            if (!base_t::eq(s1[i], s2[i])){
                return base_t::eq(s1[i], s2[i]) ? -1 : 1;
            }
        }

        return 0;
    }

    static constexpr std::size_t length(const char_type* s) noexcept {
        const char_type* const a = s;
        while (!base_t::eq(*s, char_type{})) {
            ++s;
        }

        return s - a;
    }

    static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept {
        const char_type* const end = s + n;
        for (; s != end; ++s) {
            if (base_t::eq(*s, a)){
                return s;
            }
        }

        return nullptr;
    }
};

// constexpr char trait that uses intrinsics
template <class T>
struct ce_tuned_char_trait : ce_char_trait<T> {
    using base_t = std::char_traits<T>;
    using size_t = std::size_t;
    using char_type = typename base_t::char_type;

    static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept {
        return __builtin_memcmp(s1, s2, n);
    }

    static constexpr std::size_t length(const char_type* s) noexcept {
        return __builtin_strlen(s);
    }

    static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept {
        return static_cast<const char_type*>(__builtin_memchr(s, a, n));
    }
};

// non-constexpr char trait that uses C funtion calls
template <class T>
struct char_trait_c : std::char_traits<T> {
    using base_t = std::char_traits<T>;
    using size_t = std::size_t;
    using char_type = typename base_t::char_type;

    static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept {
        return std::memcmp(s1, s2, n);
    }

    static constexpr std::size_t length(const char_type* s) noexcept {
        return std::strlen(s);
    }

    static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept {
        return (const char_type*)std::memchr(s, base_t::to_int_type(a), n);
    }
};


////////////////////////// Testing performance //////////////////////////////////////////

#include <random>
#include <vector>
#include <memory>
#include <chrono>
#include <iostream>
#include <atomic>
#include <iomanip>
#include <cassert>
#include <algorithm>

template <class CharTrait>
struct length_tester {
    const std::size_t string_length;

    template <class Vector>
    inline std::size_t operator()(const Vector& v, std::size_t i) const noexcept {
        return CharTrait::length(v[i].get());
    }
};

template <class CharTrait>
struct compare_tester {
    const std::size_t string_length;

    template <class Vector>
    inline std::size_t operator()(const Vector& v, std::size_t i) const noexcept {
        return !CharTrait::compare(v[i].get(), v[i % 10000000].get(), string_length);
    }
};

template <class CharTrait>
struct find_tester {
    const std::size_t string_length;

    template <class Vector>
    inline std::size_t operator()(const Vector& v, std::size_t i) const noexcept {
        return !!(CharTrait::find(v[i].get(), string_length, v[i][string_length - 1]) == 0);
    }
};


template <class CharTrait>
struct find2_tester {
    const std::size_t string_length;

    template <class Vector>
    std::size_t operator()(const Vector& v, std::size_t i) const noexcept {
        return !!(CharTrait::find(v[i].get(), string_length, typename CharTrait::char_type{}) == 0);
    }
};

class performance_measure {
public:
    std::size_t vector_length = 100000;
    std::size_t measures_count = 250;
    std::size_t warmups_count = 5;
    std::size_t string_length_max = 100000;
    static constexpr std::size_t loop_unroll_count = 20;

private:
#define SYMBOLS_ARRAY(...) __VA_ARGS__ ## "qwertyuiop[]asdfghjkl;'\\<zxcvbnm,./1234567890-=`~!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"|>ZXCVBNM<>?"
    static constexpr const char*     symbols(char) noexcept { return SYMBOLS_ARRAY(); }
    static constexpr const wchar_t*  symbols(wchar_t) noexcept { return SYMBOLS_ARRAY(L); }
    static constexpr const char16_t* symbols(char16_t) noexcept { return SYMBOLS_ARRAY(u); }
    static constexpr const char32_t* symbols(char32_t) noexcept { return SYMBOLS_ARRAY(U); }
    static constexpr std::size_t symbols_count = sizeof(SYMBOLS_ARRAY()) - 1;
#undef SYMBOLS_ARRAY

    static void start_table() {
    }
    static void end_table() {
        std::cout << std::endl;
    }

    template <class Strlen, class MedianLeft, class MedianRight, class Relation, class Deviation>
    static void print_row(Strlen strlen_, MedianLeft median_left_, MedianRight median_right_, Relation relation_, Deviation dev) {
        std::cout << std::left << std::setw(15) << strlen_
                  << std::left << std::setw(35) << median_left_
                  << std::left << std::setw(35) << median_right_
                  << std::left << std::setw(10) << std::fixed << std::setprecision(2) << relation_
                  << std::left << " +/-" << std::setprecision(2) << dev << "\n"
        //          << std::ends
        ;
    }



    template <class Char>
    auto generate_data(std::size_t string_length) const {
        std::vector<std::unique_ptr<Char[]>> res{vector_length};
        assert(vector_length % loop_unroll_count == 0);

        const Char* printables = symbols(Char{});

        std::mt19937 gen{0xFEED};
        std::uniform_int_distribution<> dis_char(0, symbols_count);

        for (auto& u: res) {
            const std::size_t length = string_length;
            u.reset(new Char[length]);
            for (std::size_t i = 0; i < length; ++ i) {
                u[i] = printables[dis_char(gen)];
            }
            u[length - 1] = Char{};
        }

        return res;
    }

    template <class Tester, class Char>
    std::pair<long long, long long> get_execution_median_and_meandev(Tester tester, const std::vector<std::unique_ptr<Char[]>>& v, std::size_t& ethalon) const {
        std::vector<long long> times;
        times.reserve(measures_count);

        const auto size = v.size();
        for (unsigned measure = 0; measure < measures_count + warmups_count; ++ measure) {
            const auto begin = std::chrono::high_resolution_clock::now();
            std::atomic_thread_fence(std::memory_order_seq_cst);

            std::size_t res = 0;
            for (std::size_t i = 0; i < size; ++i) {
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);

                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);


                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);

                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i++);
                res += tester(v, i);
            }

            std::atomic_thread_fence(std::memory_order_seq_cst);
            const auto end = std::chrono::high_resolution_clock::now();

            if (measure >= warmups_count)
                times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() );

            if (!ethalon) {
                ethalon = res;
            }
            assert(ethalon == res);
        }

        const auto it_median = times.begin() + times.size() / 2;
        std::nth_element(times.begin(), it_median, times.end());

        long double meandev = 0;
        for (auto& val: times) {
            meandev += std::abs(val - *it_median);
        }
        meandev = meandev / times.size();

        return {*it_median, meandev};
    }

    template <template <class> class Tester, template <class> class CharTrait1, template <class> class CharTrait2, class Char>
    void test(std::size_t string_length) const {
        const std::vector<std::unique_ptr<Char[]>> v = generate_data<Char>(string_length);
        std::size_t ethalon = 0;

        const auto median_div1 = get_execution_median_and_meandev(Tester<CharTrait1<Char> >{string_length}, v, ethalon);
        const auto median1 = median_div1.first;
        const auto meandev1 = median_div1.second;
        const auto median_div2 = get_execution_median_and_meandev(Tester<CharTrait2<Char> >{string_length}, v, ethalon);
        const auto median2 = median_div2.first;
        const auto meandev2 = median_div2.second;

        const auto relation = double(median2) / double(median1);
        print_row(string_length, median1, median2, relation, std::max(double(meandev1) / median1, double(meandev2) / median2));
    }

    static std::size_t next(std::size_t string_length) noexcept {
        if (string_length >= 50)
            return string_length * 10;
        else {
            return string_length + 10;
        }
    }

public:

    template <template <class> class Tester>
    void measure_different_traits() const {
        start_table();
        print_row("String length", "intrinsics<char>", "c_calls<char>", "Relation", "deviation");
        for (std::size_t string_length = 10; string_length < string_length_max; string_length = next(string_length))
            test<Tester, ce_tuned_char_trait, char_trait_c, char>     (string_length);
        end_table();

        start_table();
        print_row("String length", "intrinsics<char>", "hand-made<char>", "Relation", "deviation");
        for (std::size_t string_length = 10; string_length < string_length_max; string_length = next(string_length))
            test<Tester, ce_tuned_char_trait, ce_char_trait, char>     (string_length);
        end_table();

        start_table();
        print_row("String length", "hand-made<wchar_t>", "std::char_traits<wchar_t>", "Relation", "deviation");
        for (std::size_t string_length = 10; string_length < string_length_max; string_length = next(string_length))
            test<Tester, ce_char_trait,       std::char_traits, wchar_t>  (string_length);
        end_table();



        std::cout << "----------- misc... -------------\n";
        start_table();
        print_row("String length", "intrinsics<char>", "std::char_traits<char>", "Relation", "deviation");
        for (std::size_t string_length = 10; string_length < string_length_max; string_length = next(string_length))
            test<Tester, ce_tuned_char_trait, std::char_traits, char>     (string_length);
        end_table();

        start_table();
        print_row("String length", "hand-made<char16_t>", "std::char_traits<char16_t>", "Relation", "deviation");
        for (std::size_t string_length = 10; string_length < string_length_max; string_length = next(string_length))
            test<Tester, ce_char_trait,       std::char_traits, char16_t> (string_length);
        end_table();

        start_table();
        print_row("String length", "hand-made<char32_t>", "std::char_traits<char32_t>", "Relation", "deviation");
        for (std::size_t string_length = 10; string_length < string_length_max; string_length = next(string_length))
            test<Tester, ce_char_trait,       std::char_traits, char32_t> (string_length);
        end_table();

        start_table();
        print_row("String length", "hand-made<char>", "std::char_traits<char>", "Relation", "deviation");
        for (std::size_t string_length = 10; string_length < string_length_max; string_length = next(string_length))
            test<Tester, ce_char_trait,       std::char_traits, char>     (string_length);
        end_table();

        start_table();
        print_row("String length", "hand-made<char>", "c_calls<char>", "Relation", "deviation");
        for (std::size_t string_length = 10; string_length < string_length_max; string_length = next(string_length))
            test<Tester, ce_char_trait,       char_trait_c, char>         (string_length);
        end_table();

    }
};

int main() {
    performance_measure m;
    m.measures_count = 500;
    m.vector_length = 10000;

    std::cout << "Running with measures count = " << m.measures_count << " and strings count per test = " << m.vector_length << "\n\n";
    std::cout << "\nTypeTrait::length\n";
    m.measure_different_traits<length_tester>();

    std::cout << "\nTypeTrait::find\n";
    m.measure_different_traits<find_tester>();

    std::cout << "\nTypeTrait::find2\n";
    m.measure_different_traits<find2_tester>();

    std::cout << "\nTypeTrait::compare\n";
    m.measure_different_traits<compare_tester>();
}
