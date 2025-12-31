#include "GameNode.h"

// octopus
#include "octopus/serialization/queue/CommandQueueSupport.hh"
#include "octopus/serialization/components/BasicSupport.hh"
#include "octopus/serialization/components/AdvancedSupport.hh"
#include "octopus/serialization/commands/CommandSupport.hh"
#include "octopus/systems/Systems.hh"
#include "octopus/world/path/PathFindingCache.hh"

#include "godoctopus/display/vat/VatLibraryHandle.h"
#include "godoctopus/display/vat/SmartMMeshLibraryHandle.h"

namespace godot
{

GameNode::~GameNode() {
	_over = true;
	if (_loop_thread) {
		_loop_thread->join();
	}
	delete _loop_thread;
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
		declare_smart_mmesh_library_systems(ecs, _smart_mmesh_library);
	}
	if (_vat_library && _picker_node) {
		declare_pickable_systems(ecs, _vat_library, _picker_node);
	}

	advanced_components_support<custom_step_manager,NoOpCommand,MoveCommand,AttackCommand,CastCommand,SetRallyPointCommand>(ecs);

	//
	// prefab
	//
	// declare_unit_prefab(ecs, _info_node);

	// load level
	setup(meta_data, *this);

	// set pointers

	// game loop
	_init = true;
	_over = false;
	delete _loop_thread;
	_loop_thread = new std::thread(&GameNode::loop, this);
}

void GameNode::init_from_level(Dictionary const &meta_data)
{
	init_nodes();
	if(!level_node)
	{
		print_line("Cannot init from level with no level");
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

void GameNode::init_load(String file_name, Dictionary const &meta_data)
{
	init_nodes();
	init_world(meta_data, [file_name](Dictionary const &, GameNode &game) {
		// game.get_save_node()->load_from_file(file_name);
	});
}

void GameNode::init_nodes()
{
	INIT_NODE_PATH(SmartMMeshLibrary, smart_mmesh_library);
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

void GameNode::_bind_methods()
{
	BIND_NODE_PATH(GameNode, SmartMMeshLibrary, smart_mmesh_library);
	BIND_NODE_PATH(GameNode, VatLibrary, vat_library);
	BIND_NODE_PATH(GameNode, PickerNode, picker_node);

	BIND_PROP(GameNode, LevelNode, level_node);

	ClassDB::bind_method(D_METHOD("is_paused"), &GameNode::is_paused);
	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &GameNode::set_paused);
	ClassDB::add_property("GameNode", PropertyInfo(Variant::BOOL, "paused"), "set_paused", "is_paused");

	ClassDB::bind_method(D_METHOD("init_load", "file_name", "meta_data"), &GameNode::init_load);
	ClassDB::bind_method(D_METHOD("init_from_level", "meta_data"), &GameNode::init_from_level);
	ClassDB::bind_method(D_METHOD("get_avg_engine_times"), &GameNode::get_avg_engine_times);
}

void GameNode::loop()
{
	bool is_pipeline_pause = false;
	size_t counter = 0;
	size_t it_log = 100;
	int started = 0;
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
			++started;
			TimeStats &stats_l = _world.time_stats;
			stats_l = TimeStats();
			const auto start{std::chrono::steady_clock::now()};

			{
				std::lock_guard<std::mutex> lock(_progress_mutex);
				_world.ecs.progress();
			}
			--_ticks;

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
	}
}

}
