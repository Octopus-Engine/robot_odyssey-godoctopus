#pragma once

#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"

// Temporary Health Buff component - applied with time duration
template<int32_t base_quantity, int64_t duration_ticks>
struct TemporaryHealthBuff
{
	static constexpr int64_t DURATION_TICKS = duration_ticks;
	int32_t health_bonus = base_quantity;
	int32_t hitpoint_max_bonus = base_quantity;

	void apply(octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const
	{
		hp.qty += health_bonus;
		hp_max.qty += hitpoint_max_bonus;
	}

	void revert(octopus::HitPoint &hp, octopus::HitPointMax &hp_max) const
	{
		hp_max.qty -= hitpoint_max_bonus;
	}
};

// Temporary Armor Buff component - applied with time duration
template<int32_t base_quantity, int64_t duration_ticks>
struct TemporaryArmorBuff
{
	static constexpr int64_t DURATION_TICKS = duration_ticks;
	int32_t bonus = base_quantity;

	void apply(octopus::Armor &armor) const
	{
		armor.qty += octopus::Fixed(bonus) / 10;
	}

	void revert(octopus::Armor &armor) const
	{
		armor.qty -= octopus::Fixed(bonus) / 10;
	}
};

// Temporary Damage Buff component - applied with time duration
template<int32_t base_quantity, int64_t duration_ticks>
struct TemporaryDamageBuff
{
	static constexpr int64_t DURATION_TICKS = duration_ticks;
	int32_t bonus = base_quantity;

	void apply(octopus::Attack &atk) const
	{
		atk.cst.damage += bonus;
	}

	void revert(octopus::Attack &atk) const
	{
		atk.cst.damage -= bonus;
	}
};

// Temporary Attack Speed Buff component (reduces reload time) - applied with time duration
template<int32_t base_quantity, int64_t duration_ticks>
struct TemporaryAttackSpeedBuff
{
	static constexpr int64_t DURATION_TICKS = duration_ticks;
	int32_t bonus = base_quantity;

	void apply(octopus::Attack &atk) const
	{
		atk.cst.reload_time -= bonus;
	}

	void revert(octopus::Attack &atk) const
	{
		atk.cst.reload_time += bonus;
	}
};
