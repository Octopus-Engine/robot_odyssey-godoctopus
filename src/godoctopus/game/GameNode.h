#pragma once

#include "scene/3d/node_3d.h"

#include <atomic>

#include "octopus/world/WorldContext.hh"
#include "octopus/systems/input/Input.hh"

#include "vat/SmartMMeshLibrary.h"
#include "vat/VatLibrary.h"
#include "godoctopus/display/particule/ParticuleSmartMMesh.h"
#include "godoctopus/game/prefabs/UnitPrefab.h"
#include "godoctopus/pickable/PickerNode.h"

#include "godot_tools.h"
#include "octopus_types.h"

namespace godot {

class GameNode;
class ResourceNodeEventBus;

class LevelNode : public Node {
	GDCLASS(LevelNode, Node)
public:
	virtual void setup(Dictionary const &meta_data, GameNode &game)
	{
		print_line("Not implemented setup method called!");
	}
	virtual void system_setup(Dictionary const &meta_data, GameNode &game)
	{
		print_line("Not implemented system_setup method called!");
	}
};

class GameNode : public Node {
	GDCLASS(GameNode, Node)
public:
	~GameNode();

	void set_paused(bool paused) { _paused = paused; }
	bool is_paused() const { return _paused; }

	void init_world(Dictionary const &meta_data, std::function<void(Dictionary const &, GameNode&)> const &setup = [](Dictionary const &, GameNode&){});

	void init_load(String file_name, Dictionary const &meta_data);
	void init_from_level(Dictionary const &meta_data);

	void stop();
	void tick(int32_t ticks=1) { _ticks += ticks; while(_ticks > 0) { std::this_thread::yield(); } }

	// All nodes
	void init_nodes();

	SET_GET_NODE_PATH(SmartMMeshLibrary, smart_mmesh_library);
	SET_GET_NODE_PATH(ParticuleSmartMMesh, particules);
	SET_GET_NODE_PATH(VatLibrary, vat_library);
	SET_GET_NODE_PATH(PickerNode, picker_node);
	SET_GET_NODE_PATH(ResourceNodeEventBus, resource_node_event_bus);
	SET_GET_PARAM_DEF(LevelNode*, level_node, nullptr);
	SET_GET_PARAM(TypedArray<UnitPrefab>, unit_prefabs);
public:
	double get_avg_engine_times();
	int get_entity_count() const { return _entity_count.load(); }
	int get_particle_count() const;
	int get_timestamp() const { return _timestamp.load(); }

	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();

	void _process(double delta_p);

	octopus::WorldContext<custom_step_manager>& get_world() { return _world; }
	octopus::DefaultStepContext<custom_variant>& get_step_context() { return step_context; }
	octopus::Input<custom_variant, custom_step_manager> * get_input_controller() { return _input_container; }
	Ref<UnitPrefab> get_prefab(const String &prefab_name) const;
	std::mutex &get_progress_mutex() { return _progress_mutex; }

	bool init_done() const { return _init; }
protected:
	void _notification(int p_notification);
private:
	void loop();

	octopus::WorldContext<custom_step_manager> _world;
	octopus::DefaultStepContext<custom_variant> step_context;
	// octopus::Grid *_grid = nullptr;

	// EntityPayload<flecs::entity> * _entity_payload = nullptr;
	octopus::Input<custom_variant, custom_step_manager> * _input_container = nullptr;

	bool _init = false;
	/// @brief atomic ticks used in loop and process (1 to always play setup)
	std::atomic_int32_t _ticks = 0;
	double _elapsed = 0.;
	double _time_step = 0.02;

	std::thread * _loop_thread = nullptr;
	bool _over = false;
	bool _paused = false;

	/// @brief mutex used to lock when the engine is progressing
	std::mutex _progress_mutex;

	/// @brief mutex used to avoid data race when querying
	/// average engine time
	std::mutex _engine_time_mutex;
	/// @brief list of last engine times
	std::list<double> _last_engine_times;
	std::atomic_int _entity_count = 0;
	std::atomic_int _timestamp = 0;
};

}
