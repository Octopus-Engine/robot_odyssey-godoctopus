#pragma once

struct HealBot { static constexpr char const * const naming()  { return "HealBot"; } };
struct BackcanonBot { static constexpr char const * const naming()  { return "backcanon_bot"; } };

template<typename Callable, typename... Ts>
void for_each_type(Callable&& callable) {
	(callable.template operator()<Ts>(), ...);
}

template<typename Callable>
void for_each_bot_type(Callable&& callable) {
	for_each_type<Callable, HealBot, BackcanonBot>(std::move(callable));
}
