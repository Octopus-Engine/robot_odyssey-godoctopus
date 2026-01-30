#include "GameNode.h"

// octopus
#include "octopus/serialization/queue/CommandQueueSupport.hh"
#include "octopus/serialization/components/BasicSupport.hh"
#include "octopus/serialization/components/AdvancedSupport.hh"
#include "octopus/serialization/commands/CommandSupport.hh"
#include "octopus/systems/Systems.hh"
#include "octopus/world/path/PathFindingCache.hh"
#include "octopus/commands/queue/CommandQueue.hh"
#include "octopus/components/basic/armor/Armor.hh"
#include "octopus/components/basic/attack/Attack.hh"
#include "octopus/components/basic/hitpoint/HitPoint.hh"
#include "octopus/components/basic/hitpoint/HitPointMax.hh"
#include "octopus/components/basic/position/Move.hh"
#include "octopus/components/basic/position/Position.hh"

#include "godoctopus/death/DeathParticle.h"
#include "godoctopus/display/vat/VatLibraryHandle.h"
#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"
#include "godoctopus/health_bar/HealthBarNode.h"
#include "godoctopus/projectile/CustomBasicProjectile.h"
#include "godoctopus/components/types/Types.h"
#include "godoctopus/components/rune_load/RuneLoad.h"
#include "godoctopus/components/special/Special.h"
#include "godoctopus/trigger_module/TriggerDeclaration.h"

namespace godot
{

struct PrefabDeclarer {
	flecs::world &ecs;

	template<typename BotType>
	void operator()() const {
		ecs.prefab(BotType::naming())
			.template add<BotType>()
			.template auto_override<RuneLoad<DefaultRune>>()
		;
	}
};

GameNode::~GameNode() {
	stop();
}

static void declare_unit_prefab(flecs::world &ecs, Ref<UnitPrefab> unit_prefab) {

	auto prefab = ecs.prefab(unit_prefab->get_prefab_name().utf8().get_data())
		.auto_override<custom_queue>()
		.auto_override<Selected>()
		.set_auto_override<octopus::Move>({unit_prefab->get_speed_x10()/10./TICK_RATE})
		.set_auto_override<octopus::HitPoint>({unit_prefab->get_hitpoint()})
		.set_auto_override<octopus::HitPointMax>({unit_prefab->get_hitpoint()})
		.set_auto_override<octopus::Armor>({unit_prefab->get_armor()})
		.set_auto_override<Special>({unit_prefab->get_special_x10()/10., unit_prefab->get_affinity_x10()/10.})
		.auto_override<octopus::Destroyable>()
		.set<octopus::Collision>({unit_prefab->get_ray_x100()/100.})
		.auto_override<octopus::PositionInTree>()
		.set_auto_override<octopus::AttackCommand>({flecs::entity()})
		.set_auto_override<octopus::Attack>({{
			unit_prefab->get_windup_x10() * (TICK_RATE / 10),
			unit_prefab->get_reload_x10() * (TICK_RATE / 10),
			unit_prefab->get_damage_x10()/10.,
			unit_prefab->get_range_x10()/10.}})
		.set_auto_override<VatLibraryHandle>({unit_prefab->get_track_idx()})
		.auto_override<Pickable>()
		.set_auto_override<ProjectileTrajectory>({unit_prefab->get_projectile_target()})
		.set_auto_override<HealthBar>({
			unit_prefab->get_health_bar_offset_y(),
			unit_prefab->get_health_bar_width()})
	;

	if (unit_prefab->get_death_particles()) {
		prefab.set_auto_override<DeathParticle>({
			unit_prefab->get_death_particles_color().get_r8(),
			unit_prefab->get_death_particles_color().get_g8(),
			unit_prefab->get_death_particles_color().get_b8(),
			unit_prefab->get_death_particles_count(),
			unit_prefab->get_death_particles_scale(),
			unit_prefab->get_death_particles_effect_id()
		});
	}

	if (unit_prefab->get_basic_projectile()) {
		Color const &color = unit_prefab->get_projectile_color();
		prefab.set_auto_override<octopus::BasicProjectileAttack<CustomBasicProjectile>>({20./TICK_RATE,
			{color.get_r8(),color.get_g8(),color.get_b8(), unit_prefab->get_projectile_origin(), unit_prefab->get_projectile_scale()}});
	}

	if (unit_prefab->get_attack_particle()) {
		prefab.set<AttackParticle>({
			unit_prefab->get_attack_particle_effect(),
			unit_prefab->get_attack_particle_origin().x,
			unit_prefab->get_attack_particle_origin().y,
			unit_prefab->get_attack_particle_origin().z,
			unit_prefab->get_attack_particle_color().get_r8(),
			unit_prefab->get_attack_particle_color().get_g8(),
			unit_prefab->get_attack_particle_color().get_b8(),
			unit_prefab->get_attack_particle_count(),
			unit_prefab->get_attack_particle_scale()
		});
	}

	if (unit_prefab->get_windup_effect()) {
		prefab.set<WindupEffect>({
			unit_prefab->get_windup_effect_loading(),
			unit_prefab->get_windup_effect_incoming(),
			unit_prefab->get_windup_effect_origin().x,
			unit_prefab->get_windup_effect_origin().y,
			unit_prefab->get_windup_effect_origin().z,
			unit_prefab->get_windup_effect_color().get_r8(),
			unit_prefab->get_windup_effect_color().get_g8(),
			unit_prefab->get_windup_effect_color().get_b8(),
			unit_prefab->get_windup_effect_count(),
			unit_prefab->get_windup_effect_scale()
		});
	}
}


void GameNode::init_world(Dictionary const &meta_data, std::function<void(Dictionary const &, GameNode&)> const &setup)
{
	using namespace octopus;
	flecs::world &ecs = _world.ecs;

	// _entity_payload = new EntityPayload<flecs::entity>();

	// _drawer->set_time_step(_time_step);
	_world.attack_retarget_wait = 32;
	auto flock_manager = ecs.entity("flock_manager")
							.add<FlockManager>();
	ecs.add<octopus::Input<custom_variant, custom_step_manager> >();
	_input_container = ecs.try_get_mut<octopus::Input<custom_variant, custom_step_manager> >();
	_input_container->flock_manager = flock_manager;
	_input_container->stack_input();
	ecs.add<StepEntityManager>();

	// ProductionTemplateLibrary
	octopus::ProductionTemplateLibrary<custom_step_manager> prod_library;
	ecs.set(std::move(prod_library));

	// AbilityTemplateLibrary
	octopus::AbilityTemplateLibrary<custom_step_manager> cast_library;
	ecs.set(std::move(cast_library));

	// grid set up
	// delete _grid;
	// _grid = new octopus::Grid(200,200, 4);

	basic_components_support(ecs);

	//
	// Systems
	//
	ecs.add<PathFindingCache>();
	// ecs.try_get_mut<PathFindingCache>()->declare_sync_system(ecs, _grid);
	ecs.try_get_mut<PathFindingCache>()->declare_cache_update_system(ecs, _world.time_stats);

	set_up_systems<DefaultStepContext<custom_variant> >(_world, step_context, 100);

	if (_vat_library) {
		declare_vat_library_systems(ecs, _vat_library);
	}
	if (_smart_mmesh_library) {
		declare_smart_mmesh_library_systems(ecs, _smart_mmesh_library, _vat_library, 1);
	}
	if (_vat_library && _picker_node) {
		declare_pickable_systems(ecs, _vat_library, _picker_node);
	}

	advanced_components_support<custom_step_manager,NoOpCommand,MoveCommand,AttackCommand,CastCommand,SetRallyPointCommand>(ecs);

	// update entity counts
	ecs.system<>().kind(ecs.entity(DisplaySyncPhase)).run([this](flecs::iter &it) {
			_entity_count.store(0);
		});
	ecs.system<VatLibraryHandle>().kind(ecs.entity(DisplaySyncPhase)).run([this](flecs::iter &it) {
		    while (it.next()) { _entity_count.fetch_add(it.count()); }
		});

	//
	// prefab
	//
	// Add unit type component
	for_each_bot_type(PrefabDeclarer{ecs});
	for (Ref<UnitPrefab> unit_prefab : unit_prefabs) {
		declare_unit_prefab(ecs, unit_prefab);
	}

	// load level
	setup(meta_data, *this);

	// set pointers

	// game loop
	_init = true;
	_over = false;
	delete _loop_thread;
	_loop_thread = new std::thread(&GameNode::loop, this);

	emit_signal("init_done");
}

void GameNode::init_load(String file_name, Dictionary const &meta_data)
{
	init_nodes();
	init_world(meta_data, [file_name](Dictionary const &, GameNode &game) {
		// game.get_save_node()->load_from_file(file_name);
	});
}

void GameNode::init_from_level(Dictionary const &meta_data)
{
	init_nodes();
	if(!level_node)
	{
		print_line("Init with no level node set!");
		init_world(meta_data, [](Dictionary const &meta_data_, GameNode &game) {
			print_line("No level node to setup - basic setup");
			game.get_world().ecs.entity().set<octopus::PlayerInfo>({0, 0});
			game.get_world().ecs.entity().set<octopus::PlayerInfo>({1, 1});
			declare_triggers(game.get_world().ecs, game.get_world().position_context);
		});
	}
	else
	{
		print_line("Init from level");
		init_world(meta_data, [this](Dictionary const &meta_data_, GameNode &game) {
			level_node->system_setup(meta_data_, game);
			level_node->setup(meta_data_, game);
		});
	}
}

void GameNode::stop() {
	_over = true;
	if (_loop_thread) {
		_loop_thread->join();
	}
	// To avoid dangling pointer we clean up all
	_world.ecs.query<SmartMMeshLibraryHandle>().each(
		[](flecs::entity e, SmartMMeshLibraryHandle &handle) {
			handle.instance_id = -1;
		}
	);
	delete _loop_thread;
	_loop_thread = nullptr;
}

void GameNode::init_nodes()
{
	INIT_NODE_PATH(SmartMMeshLibrary, smart_mmesh_library);
	INIT_NODE_PATH(ParticuleSmartMMesh, particules);
	INIT_NODE_PATH(VatLibrary, vat_library);
	INIT_NODE_PATH(PickerNode, picker_node);
}

double GameNode::get_avg_engine_times()
{
	std::lock_guard<std::mutex> lock_l(_engine_time_mutex);
	double avg_l = 0.0;
	for(double time_l : _last_engine_times)
	{
		avg_l += time_l/_last_engine_times.size();
	}
	return avg_l;
}

int GameNode::get_particle_count() const {
	if (_particules) {
		return _particules->get_multimesh()->get_instance_count();
	}
	return 0;
}

void GameNode::_bind_methods()
{
	BIND_NODE_PATH(GameNode, SmartMMeshLibrary, smart_mmesh_library);
	BIND_NODE_PATH(GameNode, ParticuleSmartMMesh, particules);
	BIND_NODE_PATH(GameNode, VatLibrary, vat_library);
	BIND_NODE_PATH(GameNode, PickerNode, picker_node);

	BIND_PROP(GameNode, LevelNode, level_node);

	ClassDB::bind_method(D_METHOD("is_paused"), &GameNode::is_paused);
	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &GameNode::set_paused);
	ClassDB::add_property("GameNode", PropertyInfo(Variant::BOOL, "paused"), "set_paused", "is_paused");

	ClassDB::bind_method(D_METHOD("init_load", "file_name", "meta_data"), &GameNode::init_load);
	ClassDB::bind_method(D_METHOD("init_from_level", "meta_data"), &GameNode::init_from_level);
	ClassDB::bind_method(D_METHOD("stop"), &GameNode::stop);

	ClassDB::bind_method(D_METHOD("get_avg_engine_times"), &GameNode::get_avg_engine_times);
	ClassDB::bind_method(D_METHOD("get_entity_count"), &GameNode::get_entity_count);
	ClassDB::bind_method(D_METHOD("get_particle_count"), &GameNode::get_particle_count);
	ClassDB::bind_method(D_METHOD("get_timestamp"), &GameNode::get_timestamp);

	ClassDB::bind_method(D_METHOD("set_unit_prefabs", "unit_prefabs"), &GameNode::set_unit_prefabs);
	ClassDB::bind_method(D_METHOD("get_unit_prefabs"), &GameNode::get_unit_prefabs);
	String arrayType = vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "UnitPrefab");
	ClassDB::add_property("GameNode", PropertyInfo(Variant::ARRAY, "unit_prefabs", PROPERTY_HINT_TYPE_STRING, arrayType, PROPERTY_HINT_ARRAY_TYPE),
		"set_unit_prefabs", "get_unit_prefabs");

    ADD_SIGNAL(MethodInfo("init_done"));

	ADD_GROUP("GameNode", "GameNode_");
}

void GameNode::loop()
{
	bool is_pipeline_pause = false;
	size_t counter = 0;
	size_t it_log = 100;
	_timestamp.store(octopus::get_time_stamp(_world.ecs));
	while(!_over)
	{
		if(is_pipeline_pause != _paused)
		{
			if(_paused)
			{
				octopus::pause_phases(_world.ecs);
			}
			else
			{
				octopus::unpause_phases(_world.ecs);
			}
			is_pipeline_pause = _paused;
		}
		if(_ticks >= 1)
		{
			TimeStats &stats_l = _world.time_stats;
			stats_l = TimeStats();
			const auto start{std::chrono::steady_clock::now()};

			{
				std::lock_guard<std::mutex> lock(_progress_mutex);
				_world.ecs.progress();
			}
			--_ticks;
			_timestamp.store(octopus::get_time_stamp(_world.ecs));

			const auto end{std::chrono::steady_clock::now()};
			const std::chrono::duration<double> elapsed_seconds{end - start};
			if(counter % it_log == 0)
			{
				std::stringstream ss_l;
				ss_l<< elapsed_seconds.count() * 1000. << "ms"; // C++20's chrono::duration operator<<
			}
			++counter;
			std::lock_guard<std::mutex> lock_l(_engine_time_mutex);
			_last_engine_times.push_back(elapsed_seconds.count() * 1000.);
			while(_last_engine_times.size() > TICK_RATE) { _last_engine_times.pop_front(); }
		}
		else
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1ms);
		}
	}
}

void GameNode::_process(double delta_p)
{
	if(!_init)
	{
		return;
	}
	_elapsed += delta_p;
	while(_elapsed >= _time_step)
	{
		++_ticks;
		_elapsed -= _time_step;
	}
}

void GameNode::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			print_line("GameNode ready");
			set_process(true);
		} break;
		case NOTIFICATION_WM_CLOSE_REQUEST: {
			print_line("GameNode close requested");
			stop();
		} break;
	}
}

}
