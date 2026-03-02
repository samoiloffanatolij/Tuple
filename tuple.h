// Solution folder: https://gitlab.manytask.org/mipt-cpp/students-2025-spring/samoilov.av/-/tree/1a4caa9c28e36691d11dcaaa5ad65024bc48c16e/tuple
// Job link: https://gitlab.manytask.org/mipt-cpp/students-2025-spring/samoilov.av/-/jobs/946670

#ifndef TUPLE_H
#define TUPLE_H

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

template <typename... Types>
class Tuple;

template <class... Types>
struct std::tuple_size<Tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template <size_t Ind, class T>
struct tuple_element;

template <size_t Ind, class Head, class... Tail>
struct tuple_element<Ind, Tuple<Head, Tail...>> : tuple_element<Ind - 1, Tuple<Tail...>> {};

template <class Head, class... Tail>
struct tuple_element<0, Tuple<Head, Tail...>> {
    using type = Head;
};

template <size_t Ind, class T>
using tuple_element_t = typename tuple_element<Ind, T>::type;

namespace detail {
template <typename T>
concept empty_braces_initializable = requires(T t) { t = {}; };

template <std::size_t, typename T>
struct TupleLeaf {
    T value;
};

template <typename IndexSequence, typename... Types>
struct TupleBase;

template <std::size_t... Indices, typename... Types>
struct TupleBase<std::index_sequence<Indices...>, Types...> : TupleLeaf<Indices, Types>... {
    constexpr TupleBase() = default;

    template <typename... ArgTypes>
    constexpr TupleBase(ArgTypes&&... args)
        : TupleLeaf<Indices, Types>{std::forward<ArgTypes>(args)}... {
    }
};
}  // namespace detail

template <typename... Types>
class Tuple : detail::TupleBase<std::make_index_sequence<sizeof...(Types)>, Types...> {
    using Base = detail::TupleBase<std::make_index_sequence<sizeof...(Types)>, Types...>;

    template <typename ArgType, std::size_t... Indices>
    constexpr Tuple(ArgType&& other, std::index_sequence<Indices...>)
        : Base(get<Indices>(std::forward<ArgType>(other))...) {
    }

public:
    template <std::size_t Ind, typename... ElementTypes>
    friend constexpr tuple_element_t<Ind, Tuple<ElementTypes...>>& get(
        Tuple<ElementTypes...>& tuple) noexcept;
    template <std::size_t Ind, typename... ElementTypes>
    friend constexpr const tuple_element_t<Ind, Tuple<ElementTypes...>>& get(
        const Tuple<ElementTypes...>& tuple) noexcept;
    template <std::size_t Ind, typename... ElementTypes>
    friend constexpr tuple_element_t<Ind, Tuple<ElementTypes...>>&& get(
        Tuple<ElementTypes...>&& tuple) noexcept;
    template <std::size_t Ind, typename... ElementTypes>
    friend constexpr const tuple_element_t<Ind, Tuple<ElementTypes...>>&& get(
        const Tuple<ElementTypes...>&& tuple) noexcept;

    constexpr explicit((!detail::empty_braces_initializable<Types> || ...)) Tuple()
        requires((std::is_default_constructible_v<Types> && ...))
    = default;

    constexpr explicit(!(... && std::is_convertible_v<const Types&, Types>))
        Tuple(const Types&... args)
        requires(sizeof...(Types) >= 1 && (std::is_copy_constructible_v<Types> && ...))
        : Base(args...) {
    }

    template <typename... ArgTypes>
    constexpr explicit(!(... && std::is_convertible_v<ArgTypes&&, Types>)) Tuple(ArgTypes&&... args)
        requires(sizeof...(Types) == sizeof...(ArgTypes) && sizeof...(Types) > 1 &&
                 (std::is_constructible_v<Types, ArgTypes &&> && ...))
        : Base(std::forward<ArgTypes>(args)...) {
    }

    Tuple(const Tuple&)
        requires((std::is_copy_constructible_v<Types> && ...))
    = default;
    Tuple(Tuple&&)
        requires((std::is_move_constructible_v<Types> && ...))
    = default;

    template <typename... ArgTypes>
    constexpr explicit(!(... && std::is_convertible_v<const ArgTypes&, Types>))
        Tuple(const Tuple<ArgTypes...>& other)
        requires(
            sizeof...(Types) == sizeof...(ArgTypes) &&
            (std::is_constructible_v<Types, const ArgTypes&> && ...) &&
            (sizeof...(Types) != 1 ||
             !(std::is_convertible_v<const Tuple<ArgTypes...>&, tuple_element_t<0, Tuple>> &&
               std::is_constructible_v<tuple_element_t<0, Tuple>, const Tuple<ArgTypes...>&> &&
               std::is_same_v<tuple_element_t<0, Tuple>, tuple_element_t<0, Tuple<ArgTypes...>>>)))
        : Tuple(other, std::make_index_sequence<sizeof...(Types)>{}) {
    }

    template <typename... ArgTypes>
    constexpr explicit(!(... && std::is_convertible_v<ArgTypes&&, Types>))
        Tuple(Tuple<ArgTypes...>&& other)
        requires(
            sizeof...(Types) == sizeof...(ArgTypes) &&
            (std::is_constructible_v<Types, ArgTypes &&> && ...) &&
            (sizeof...(Types) != 1 ||
             !(std::is_convertible_v<Tuple<ArgTypes...> &&, tuple_element_t<0, Tuple>> &&
               std::is_constructible_v<tuple_element_t<0, Tuple>, Tuple<ArgTypes...> &&> &&
               std::is_same_v<tuple_element_t<0, Tuple>, tuple_element_t<0, Tuple<ArgTypes...>>>)))
        : Tuple(std::move(other), std::make_index_sequence<sizeof...(Types)>{}) {
    }

    template <typename LeftType, typename RightType>
    constexpr Tuple(const std::pair<LeftType, RightType>& pair)
        requires(sizeof...(Types) == 2 &&
                 std::is_constructible_v<tuple_element_t<0, Tuple>, const LeftType&> &&
                 std::is_constructible_v<tuple_element_t<1, Tuple>, const RightType&>)
        : Base(pair.first, pair.second) {
    }

    template <typename LeftType, typename RightType>
    constexpr Tuple(std::pair<LeftType, RightType>&& pair)
        requires(sizeof...(Types) == 2 &&
                 std::is_constructible_v<tuple_element_t<0, Tuple>, LeftType &&> &&
                 std::is_constructible_v<tuple_element_t<1, Tuple>, RightType &&>)
        : Base(std::forward<LeftType>(pair.first), std::forward<RightType>(pair.second)) {
    }

    Tuple& operator=(const Tuple& other)
        requires((std::is_copy_assignable_v<Types> && ...))
    {
        if (this != &other) {
            [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
                (..., (get<Indices>(*this) = get<Indices>(other)));
            }(std::make_index_sequence<sizeof...(Types)>{});
        }
        return *this;
    }

    Tuple& operator=(Tuple&& other) noexcept
        requires((std::is_move_assignable_v<Types> && ...))
    {
        [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (..., (get<Indices>(*this) =
                       std::forward<tuple_element_t<Indices, Tuple>>(get<Indices>(other))));
        }(std::make_index_sequence<sizeof...(Types)>{});
        return *this;
    }

    template <typename... ArgTypes>
    Tuple& operator=(const Tuple<ArgTypes...>& other)
        requires(sizeof...(Types) == sizeof...(ArgTypes) &&
                 (std::is_assignable_v<Types&, const ArgTypes&> && ...))
    {
        [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (..., (get<Indices>(*this) = get<Indices>(other)));
        }(std::make_index_sequence<sizeof...(Types)>{});
        return *this;
    }

    template <typename... ArgTypes>
    Tuple& operator=(Tuple<ArgTypes...>&& other)
        requires(sizeof...(Types) == sizeof...(ArgTypes) &&
                 (std::is_assignable_v<Types&, ArgTypes &&> && ...))
    {
        [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (..., (get<Indices>(*this) = get<Indices>(std::move(other))));
        }(std::make_index_sequence<sizeof...(Types)>{});
        return *this;
    }
};

template <typename LeftType, typename RightType>
Tuple(std::pair<LeftType, RightType>) -> Tuple<LeftType, RightType>;

template <std::size_t Ind, typename... Types>
constexpr tuple_element_t<Ind, Tuple<Types...>>& get(Tuple<Types...>& tuple) noexcept {
    using ElemType = tuple_element_t<Ind, Tuple<Types...>>;
    return static_cast<detail::TupleLeaf<Ind, ElemType>&>(tuple).value;
}

template <std::size_t Ind, typename... Types>
constexpr const tuple_element_t<Ind, Tuple<Types...>>& get(const Tuple<Types...>& tuple) noexcept {
    using ElemType = tuple_element_t<Ind, Tuple<Types...>>;
    return static_cast<const detail::TupleLeaf<Ind, ElemType>&>(tuple).value;
}

template <std::size_t Ind, typename... Types>
constexpr tuple_element_t<Ind, Tuple<Types...>>&& get(Tuple<Types...>&& tuple) noexcept {
    using ElemType = tuple_element_t<Ind, Tuple<Types...>>;
    return std::forward<ElemType>(static_cast<detail::TupleLeaf<Ind, ElemType>&>(tuple).value);
}

template <std::size_t Ind, typename... Types>
constexpr const tuple_element_t<Ind, Tuple<Types...>>&& get(
    const Tuple<Types...>&& tuple) noexcept {
    using ElemType = tuple_element_t<Ind, Tuple<Types...>>;
    return std::forward<const ElemType>(
        static_cast<const detail::TupleLeaf<Ind, ElemType>&>(tuple).value);
}

namespace detail {
template <typename T, typename Tuple>
struct IndexOfType;

template <typename T, typename... Types>
struct IndexOfType<T, Tuple<T, Types...>> {
    static constexpr std::size_t kValue = 0;
};

template <typename T, typename Head, typename... Tail>
struct IndexOfType<T, Tuple<Head, Tail...>> {
    static constexpr std::size_t kValue = 1 + IndexOfType<T, Tuple<Tail...>>::kValue;
};

template <typename T, typename... Types>
constexpr std::size_t kCountType = (std::is_same_v<T, Types> + ...);
}  // namespace detail

template <typename T, typename... Types>
constexpr T& get(Tuple<Types...>& tuple) noexcept
    requires(detail::kCountType<T, Types...> == 1)
{
    return get<detail::IndexOfType<T, Tuple<Types...>>::kValue>(tuple);
}

template <typename T, typename... Types>
constexpr const T& get(const Tuple<Types...>& tuple) noexcept
    requires(detail::kCountType<T, Types...> == 1)
{
    return get<detail::IndexOfType<T, Tuple<Types...>>::kValue>(tuple);
}

template <typename T, typename... Types>
constexpr T&& get(Tuple<Types...>&& tuple) noexcept
    requires(detail::kCountType<T, Types...> == 1)
{
    return get<detail::IndexOfType<T, Tuple<Types...>>::kValue>(std::move(tuple));
}

template <typename T, typename... Types>
constexpr const T&& get(const Tuple<Types...>&& tuple) noexcept
    requires(detail::kCountType<T, Types...> == 1)
{
    return get<detail::IndexOfType<T, Tuple<Types...>>::kValue>(std::move(tuple));
}

template <typename... Args>
constexpr Tuple<Args&...> tie(Args&... args) noexcept {
    return Tuple<Args&...>(args...);
}

template <typename... Args>
constexpr Tuple<Args&&...> forwardAsTuple(Args&&... args) noexcept {
    return Tuple<Args&&...>(std::forward<Args>(args)...);
}

template <typename... Args>
constexpr auto makeTuple(Args&&... args) {
    return Tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
}

namespace detail {

template <size_t Ind, typename Tuple, typename... Tuples>
constexpr decltype(auto) get_ith_element(Tuple&& first, Tuples&&... others) {
    if constexpr (constexpr size_t kSize = std::tuple_size_v<std::decay_t<Tuple>>; Ind < kSize) {

        return get<Ind>(std::forward<Tuple>(first));
    } else {

        return get_ith_element<Ind - kSize>(std::forward<Tuples>(others)...);
    }
}

template <typename... Tuples, size_t... Indices>
constexpr auto tupleCatBase(std::index_sequence<Indices...>, Tuples&&... tuples) {

    return makeTuple(get_ith_element<Indices>(std::forward<Tuples>(tuples)...)...);
}

}  // namespace detail

template <typename... Tuples>
constexpr auto tupleCat(Tuples&&... tuples) {

    constexpr size_t kTotalSize = (0 + ... + std::tuple_size_v<std::decay_t<Tuples>>);

    return detail::tupleCatBase(std::make_index_sequence<kTotalSize>{},
                                std::forward<Tuples>(tuples)...);
}

namespace detail {
template <std::size_t Ind, typename... LeftType, typename... RightType>
constexpr bool are_tuples_equal(const Tuple<LeftType...>& lhs, const Tuple<RightType...>& rhs) {
    if constexpr (Ind == sizeof...(LeftType)) {
        return true;
    } else {
        return get<Ind>(lhs) == get<Ind>(rhs) && are_tuples_equal<Ind + 1>(lhs, rhs);
    }
}
template <std::size_t Ind, typename... LeftType, typename... RightType>
constexpr bool is_tuple_less(const Tuple<LeftType...>& lhs, const Tuple<RightType...>& rhs) {
    if constexpr (Ind == sizeof...(LeftType)) {
        return false;
    } else {
        if (get<Ind>(lhs) < get<Ind>(rhs)) {
            return true;
        }
        if (get<Ind>(lhs) > get<Ind>(rhs)) {
            return false;
        }
        return is_tuple_less<Ind + 1>(lhs, rhs);
    }
}
}  // namespace detail

template <typename... LeftType, typename... RightType>
constexpr bool operator==(const Tuple<LeftType...>& lhs, const Tuple<RightType...>& rhs)
    requires(sizeof...(LeftType) == sizeof...(RightType))
{
    return detail::are_tuples_equal<0>(lhs, rhs);
}

template <typename... LeftType, typename... RightType>
constexpr bool operator!=(const Tuple<LeftType...>& lhs, const Tuple<RightType...>& rhs) {
    return !(lhs == rhs);
}

template <typename... LeftType, typename... RightType>
constexpr bool operator<(const Tuple<LeftType...>& lhs, const Tuple<RightType...>& rhs)
    requires(sizeof...(LeftType) == sizeof...(RightType))
{
    return detail::is_tuple_less<0>(lhs, rhs);
}

template <typename... LeftType, typename... RightType>
constexpr bool operator>(const Tuple<LeftType...>& lhs, const Tuple<RightType...>& rhs) {
    return rhs < lhs;
}

template <typename... LeftType, typename... RightType>
constexpr bool operator<=(const Tuple<LeftType...>& lhs, const Tuple<RightType...>& rhs) {
    return !(lhs > rhs);
}

template <typename... LeftType, typename... RightType>
constexpr bool operator>=(const Tuple<LeftType...>& lhs, const Tuple<RightType...>& rhs) {
    return !(lhs < rhs);
}

#endif