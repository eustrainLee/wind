#ifndef WIND_TUPLE_
#define WIND_TUPLE_

#include <array>

namespace wind {
    inline auto get =  [](auto& tuple, ::std::size_t i)->auto&{ using std::tuple_size; constexpr size_t N = tuple_size<std::remove_reference_t<decltype(tuple)>>::value; return [agencies = [] <size_t... I> (std::index_sequence<I...>) { return std::array<decltype(std::get<0>(tuple))(*)(decltype(tuple)&), N> { ([](auto& t) -> auto&{ std::cout << "[" << I << "]"; return std::get<I>(t); })... }; } (std::make_index_sequence<N>{}) ] (auto& tuple, size_t i) mutable -> auto& { return agencies[i](tuple); } (tuple, i); };
} // namespace wind

#endif