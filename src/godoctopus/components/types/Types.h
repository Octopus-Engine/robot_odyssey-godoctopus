#pragma once

struct HealBot { static constexpr char const * const naming() { return "healbot"; } };
struct BackcanonBot { static constexpr char const * const naming() { return "backcanon_bot"; } };
struct RamBot { static constexpr char const * const naming() { return "rambot"; } };
struct TallBot { static constexpr char const * const naming() { return "tallbot"; } };
struct EarBot { static constexpr char const * const naming() { return "earbot"; } };
struct EarlargeBot { static constexpr char const * const naming() { return "earlarge_bot"; } };
struct HighEbot { static constexpr char const * const naming() { return "high_ebot"; } };
struct SmallroundEbot { static constexpr char const * const naming() { return "smallround_ebot"; } };
struct TinyEbot { static constexpr char const * const naming() { return "tiny_ebot"; } };

template<typename Callable, typename... Ts>
void for_each_type(Callable&& callable) {
	(callable.template operator()<Ts>(), ...);
}

template<typename Callable>
void for_each_bot_type(Callable&& callable) {
	for_each_type<Callable
		, HealBot
		, BackcanonBot
		, RamBot
		, TallBot
		, EarBot
		, EarlargeBot
		, HighEbot
		, SmallroundEbot
		, TinyEbot
	>(std::move(callable));
}
