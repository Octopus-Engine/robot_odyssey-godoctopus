#pragma once

#include "scene/main/node.h"

// octopus2
#include "octopus/world/WorldContext.hh"
#include "octopus_types.h"

#include "godoctopus/game/GameNode.h"

#include <mutex>
#include "smart_list/smart_list.h"

namespace godot {

struct CustomUnitData {
	float x = 0.f;
	float y = 0.f;
	float visibility_range = 0.f;
};

class VisionNode : public Node {
	GDCLASS(VisionNode, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:

	static void _bind_methods();

	void setup();

	/// Returns a flat PackedFloat32Array of [x, y, visibility_range] triples:
	/// flecs-tracked Explorator entities first, then custom units appended.
	PackedFloat32Array get_unit_vision_data() {
		PackedFloat32Array result;
		{
			std::lock_guard<std::mutex> lock(_mutex);
			result = _unit_vision_data;
		}
		_custom_units.for_each_const([&result](CustomUnitData const &unit_data) {
			result.push_back(unit_data.x);
			result.push_back(unit_data.y);
			result.push_back(unit_data.visibility_range);
		});
		return result;
	}

	/// Adds a custom unit and returns its assigned ID.
	int add_custom_unit(float x, float y, float visibility_range);

	/// Updates an existing custom unit. No-op if the ID does not exist.
	void update_custom_unit(int id, float x, float y, float visibility_range);

	/// Removes a custom unit. No-op if the ID does not exist.
	void remove_custom_unit(int id);

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	std::mutex _mutex;
	PackedFloat32Array _unit_vision_data;

	// Custom units are accessed from the main thread only — no locking required.
	smart_list<CustomUnitData> _custom_units;
};

}
