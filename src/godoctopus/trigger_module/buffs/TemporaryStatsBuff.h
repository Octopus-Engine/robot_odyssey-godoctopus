#pragma once

#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"

// Temporary Armor Buff component - applied with time duration
template<typename RuneType>
struct TemporaryArmorBuff
{
	using ComponentType = octopus::Armor;
	octopus::Fixed bonus;

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
template<typename RuneType>
struct TemporaryDamageBuff
{
	using ComponentType = octopus::Attack;
	octopus::Fixed bonus;

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
template<typename RuneType>
struct TemporaryAttackSpeedBuff
{
	using ComponentType = octopus::Attack;
	octopus::Fixed bonus;

	void apply(octopus::Attack &atk) const
	{
		atk.cst.reload_time -= bonus.to_int();
	}

	void revert(octopus::Attack &atk) const
	{
		atk.cst.reload_time += bonus.to_int();
	}
};
