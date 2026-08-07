#include "StatsModifiers.test.h"
#include "tests/test_macros.h"

#include "godoctopus/components/stats/StatsModifier.h"
#include "godoctopus/components/stats/StatsModifierRegister.h"
#include "godoctopus/components/stats/StatsModifierRecorder.h"
#include "godoctopus/components/stats/StatsUpdateSystems.h"
#include "octopus/systems/Systems.hh"
#include "octopus/serialization/utils/UtilsSupport.hh"
#include "octopus_types.h"

struct TestModifierA {};
struct TestModifierB {};

static flecs::entity create_test_entity(flecs::world &ecs) {
	auto e = ecs.entity();
	e.set<godoctopus::BaseStats>({{{250, 10, 5, 0, 0, 0, 0, 1, 0}}});
	e.set<godoctopus::CurrentStats>({{{250, 10, 5, 0, 0, 0, 0, 1, 0}}});
	e.add<godoctopus::StatsModifierRegister>();
	octopus::AttackConstants attack_cst {100, 10, octopus::Fixed(5), octopus::Fixed(10)};
	e.set<octopus::Attack>({attack_cst});
	e.set<octopus::HitPoint>({{100}});
	e.set<octopus::HitPointMax>({{100}});
	return e;
}

using AddTestModifierA = octopus::AddComponentStep<godoctopus::StatsModifierRecorder<TestModifierA>>;
using RemoveTestModifierA = octopus::RemoveComponentStep<godoctopus::StatsModifierRecorder<TestModifierA>>;
using AddTestModifierB = octopus::AddComponentStep<godoctopus::StatsModifierRecorder<TestModifierB>>;
using RemoveTestModifierB = octopus::RemoveComponentStep<godoctopus::StatsModifierRecorder<TestModifierB>>;

template<typename T>
void apply(octopus::DefaultStepContext<custom_variant>& step_context, flecs::entity e, T const &step) {
	step_context.step_manager.get_last_component_layer().back().add_step(e, step);
}

typedef std::variant<AddTestModifierA,
	RemoveTestModifierA,
	AddTestModifierB,
	RemoveTestModifierB> StatsModifierStepVariant;

void test_stats_modifiers() {
	octopus::WorldContext<custom_step_manager> world;
	octopus::DefaultStepContext<custom_variant> step_context;
	flecs::world &ecs = world.ecs;

	octopus::utils_support(ecs);
	godoctopus::declare_stateupdate_systems(ecs);
	godoctopus::declare_modifier_register<TestModifierA>(ecs);
	godoctopus::declare_modifier_register<TestModifierB>(ecs);
	octopus::set_up_systems(world, step_context);

	auto e = create_test_entity(ecs);
	ecs.progress(); // run initialization phase

	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(10));
	CHECK(e.get<octopus::Attack>().cst.reload_time == 5000);
	CHECK(e.get<octopus::HitPoint>().qty == octopus::Fixed(250));
	CHECK(e.get<octopus::HitPointMax>().qty == octopus::Fixed(250));

	std::vector<StatsModifierStepVariant> steps;
	ecs.system<>()
		.kind(ecs.entity(EndUpdatePhase))
		.with<godoctopus::StatsModifierRegister>()
		.each([&step_context, &steps](flecs::entity e) {
			for (auto const &step : steps) {
				std::visit([&step_context, &e](auto&& arg) { apply(step_context, e, arg); }, step);
			}
			steps.clear();
		});

	/// First test

	steps.push_back(AddTestModifierA {
		{godoctopus::StatsType::Damage, octopus::Fixed(5), {0, 0, 0, 0, 0, 0, 0, 0, 0}},
	});
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(15));

	steps.push_back(AddTestModifierA {
		{godoctopus::StatsType::Damage, octopus::Fixed(5), {0, 0, 0, 0, 0, 0, 0, 0, 0}},
	});
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(15));

	steps.push_back(RemoveTestModifierA());
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(10));

	/// Second test

	steps.push_back(AddTestModifierA {
		{godoctopus::StatsType::MechanicalPower, octopus::Fixed(100), {0, 0, 0, 0, 0, 0, 0, 0, 0}},
	});
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(10));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(100));

	steps.push_back(AddTestModifierB {
		{godoctopus::StatsType::Damage, octopus::Fixed(0), {0, 0, 0, 1/octopus::Fixed(10), 0, 0, 0, 0, 0}},
	});
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(20));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(100));

	steps.push_back(RemoveTestModifierB());
	steps.push_back(RemoveTestModifierA());
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(10));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(0));

	// Third test

	steps.push_back(AddTestModifierB {
		{godoctopus::StatsType::Damage, octopus::Fixed(0), {0, 0, 0, 1/octopus::Fixed(10), 0, 0, 0, 0, 0}},
	});
	steps.push_back(AddTestModifierA {
		{godoctopus::StatsType::MechanicalPower, octopus::Fixed(100), {0, 0, 0, 0, 0, 0, 0, 0, 0}},
	});
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(10));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(100));

	steps.push_back(RemoveTestModifierB());
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(10));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(100));

	steps.push_back(AddTestModifierB {
		{godoctopus::StatsType::Damage, octopus::Fixed(0), {0, 0, 0, 1/octopus::Fixed(10), 0, 0, 0, 0, 0}},
	});
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(20));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(100));

	steps.push_back(RemoveTestModifierB());
	steps.push_back(RemoveTestModifierA());
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(10));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(0));

	// Fourth test

	steps.push_back(AddTestModifierB {
		{godoctopus::StatsType::Damage, octopus::Fixed(0), {0, 0, 0, 1/octopus::Fixed(10), 0, 0, 0, 0, 0}, 2},
	});
	steps.push_back(AddTestModifierA {
		{godoctopus::StatsType::MechanicalPower, octopus::Fixed(100), {0, 0, 0, 0, 0, 0, 0, 0, 0}, 1},
	});
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(20));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(100));

	steps.push_back(RemoveTestModifierB());
	steps.push_back(RemoveTestModifierA());
	ecs.progress(); // run step & validate
	CHECK(e.get<octopus::Attack>().cst.damage == octopus::Fixed(10));
	CHECK(e.get<godoctopus::CurrentStats>().stats.values[godoctopus::StatsType::MechanicalPower] == octopus::Fixed(0));
}
