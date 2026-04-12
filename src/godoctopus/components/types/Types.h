#pragma once

struct ArmorBot { static constexpr char const * const naming() { return "armorbot"; } };
struct BackcanonBot { static constexpr char const * const naming() { return "backcanon_bot"; } };
struct BladeBot { static constexpr char const * const naming() { return "bladebot"; } };
struct EarBot { static constexpr char const * const naming() { return "earbot"; } };
struct EarlargeBot { static constexpr char const * const naming() { return "earlarge_bot"; } };
struct GunBot { static constexpr char const * const naming() { return "gunbot"; } };
struct HealBot { static constexpr char const * const naming() { return "healbot"; } };
struct High_eBot { static constexpr char const * const naming() { return "high_ebot"; } };
struct RamBot { static constexpr char const * const naming() { return "rambot"; } };
struct Smallround_eBot { static constexpr char const * const naming() { return "smallround_ebot"; } };
struct TallBot { static constexpr char const * const naming() { return "tallbot"; } };
struct TankyBot { static constexpr char const * const naming() { return "tankybot"; } };
struct Tiny_eBot { static constexpr char const * const naming() { return "tiny_ebot"; } };
struct HeavyfireBot { static constexpr char const * const naming() { return "heavyfire_bot"; } };
struct SniperBot { static constexpr char const * const naming() { return "sniperbot"; } };

struct PrefabType {
	std::string name;
};

inline void declare_prefab_type(flecs::world &ecs) {
	ecs.component<PrefabType>()
		.member("name", &PrefabType::name);
}

template<typename Callable, typename... Ts>
void for_each_type(Callable&& callable) {
	(callable.template operator()<Ts>(), ...);
}

template<typename Callable>
void for_each_bot_type(Callable&& callable) {
	for_each_type<Callable
		, ArmorBot
		, BackcanonBot
		, BladeBot
		, EarBot
		, EarlargeBot
		, GunBot
		, HealBot
		, High_eBot
		, RamBot
		, Smallround_eBot
		, TallBot
		, TankyBot
		, Tiny_eBot
		, HeavyfireBot
		, SniperBot
	>(std::move(callable));
}
