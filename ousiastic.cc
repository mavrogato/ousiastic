
#include <algorithm>
#include <iostream>
#include <span>
#include <ranges>
#include <filesystem>
#include <tuple>

namespace aux
{
    template <size_t N>
    struct literal : public std::array<char, N> {
    public:
        constexpr literal(char const (&src)[N]) noexcept {
            std::copy(std::ranges::begin(src), std::ranges::end(src), this->begin());
        }
        template <size_t M>
        constexpr auto operator<=>(literal<M> const& rhs) const noexcept {
            return std::lexicographical_compare_three_way(this->begin(), this->end(),
                                                          rhs.begin(), rhs.end());
        }
        template <size_t M>
        constexpr auto operator==(literal<M> const& rhs) const noexcept {
            return std::ranges::equal(*this, rhs);
        }
        template <class Ch, class Tr>
        friend auto& operator<<(std::basic_ostream<Ch, Tr>& output, literal const& s) noexcept {
            return output.write(s.data(), s.size());
        }
    };

    template <literal L, class T>
    struct ousiastic {
        static constexpr auto name = L;
        using type = T;
    };

    template <class... T>
    struct ousiastic_tuple : public std::tuple<typename T::type...> {
        static constexpr std::tuple<literal<T::name.size()>...> labels = { T::name... };
        template <size_t N, size_t I = 0>
        static constexpr size_t index(literal<N> key) noexcept {
            if constexpr (I < sizeof... (T)) {
                if (std::get<I>(labels) == key) return I;
                return index<N, I+1>(key);
            }
            else return std::numeric_limits<size_t>::max();
        }
        using std::tuple<typename T::type...>::tuple; // constructor deligation
    };

    template <literal key, class... T>
    decltype(auto) get(ousiastic_tuple<T...>& t) noexcept {
        return std::get<ousiastic_tuple<T...>::index(key)>(t);
    }
    template <literal key, class... T>
    decltype(auto) get(ousiastic_tuple<T...> const& t) noexcept {
        return std::get<ousiastic_tuple<T...>::index(key)>(t);
    }
    template <literal key, class... T>
    decltype(auto) get(ousiastic_tuple<T...>&& t) noexcept {
        return std::get<ousiastic_tuple<T...>::index(key)>(t);
    }

} // ::aux

int main(int argc, char const* argv[]) {
    auto args = std::span<char const*>(argv, argc)
        | std::views::drop(1)
        | std::views::filter([](auto arg) { return std::filesystem::exists(arg); });
    for (auto arg : args) {
        std::cout << std::filesystem::canonical(arg).c_str() << std::endl;
    }
    aux::ousiastic_tuple<
        aux::ousiastic<"universe", int>,
        aux::ousiastic<"pi", double>,
        aux::ousiastic<"salutation", char const*>> test{42, 3.14, "Hi"};
    std::cout << aux::get<"universe">(test) << std::endl;
    std::cout << aux::get<"pi">(test) << std::endl;
    std::cout << aux::get<"salutation">(test) << std::endl;
    return 0;
}
