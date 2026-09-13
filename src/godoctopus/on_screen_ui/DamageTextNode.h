#pragma once

#include "scene/gui/control.h"
#include "scene/3d/camera_3d.h"
#include "scene/resources/font.h"
#include "core/math/random_number_generator.h"

#include "godoctopus/game/GameNode.h"
#include "godot_tools.h"
#include "smart_list/smart_list.h"

#include "octopus/components/basic/hitpoint/HitPoint.hh"

#include <mutex>
#include <vector>

// Trigger event specific to DamageTextNode: entities carrying this component and emitting
// this event will have a floating damage/heal number displayed above them.
namespace trigger_module
{
	/// @brief when a floating damage/heal number should be displayed above a unit (event pushed to the unit to display on)
	struct DamageText
	{
		typedef DamageText component;
		octopus::Fixed amount;
		bool is_heal = false;
	};

	/// @brief emit a DamageText event on the given entity so that DamageTextNode can show a
	/// floating damage/heal number above it.
	inline void emit_damage_text_trigger(flecs::entity const &target, octopus::Fixed const &amount, bool is_heal) {
		flecs::world ecs = target.world();
		auto *damage_text = target.try_get_mut<DamageText>();
		if (damage_text) {
			damage_text->amount = amount;
			damage_text->is_heal = is_heal;
			ecs.event<DamageText>()
				.id<DamageText>()
				.entity(target)
				.emit();
		}
	}
}

namespace godot {

// A single floating damage/heal text instance and its animation state.
struct DamageTextItem {
	String text;
	Vector3 world_pos;
	Vector2 offset;
	// Per-item swirl parameters (see DamageTextNode::_process for the motion formula).
	double ray = 0.;
	double dist_x = 1.;
	double dist_y = 1.;
	double offset_x = 0.;
	double offset_y = 0.;
	Color color = Color(1, 1, 1, 1);
	double age = 0.;
	double lifespan = 1.;
	double scale = 0.;
};

// Displays animated, "popping" floating text (damage/heal numbers) above world entities.
// Spawned items animate for a configurable lifespan, then are discarded.
class DamageTextNode : public Control {
	GDCLASS(DamageTextNode, Control)

	SET_GET_NODE_PATH(Camera3D, camera);
	SET_GET_NODE_PATH(GameNode, game_node);
	SET_GET_PARAM(Ref<Font>, font);
	SET_GET_PARAM_DEF(int, font_size, 16);
	SET_GET_PARAM_DEF(double, ray_min, 10.);
	SET_GET_PARAM_DEF(double, ray_max, 25.);
	SET_GET_PARAM_DEF(double, dist_x, 1.);
	SET_GET_PARAM_DEF(double, dist_y, 1.);
	SET_GET_PARAM_DEF(double, horizontal_jitter, 10.);
	SET_GET_PARAM_DEF(double, pop_duration_ratio, 0.25);
	SET_GET_PARAM_DEF(double, fade_out_ratio, 0.7);
public:
	~DamageTextNode();
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods() {
		BIND_NODE_PATH(DamageTextNode, Camera3D, camera);
		BIND_NODE_PATH(DamageTextNode, GameNode, game_node);
		ADD_OBJECT_PROP(DamageTextNode, Font, font);
		ADD_SIMPLE_PROP(DamageTextNode, INT, font_size);
		ADD_SIMPLE_PROP(DamageTextNode, FLOAT, ray_min);
		ADD_SIMPLE_PROP(DamageTextNode, FLOAT, ray_max);
		ADD_SIMPLE_PROP(DamageTextNode, FLOAT, dist_x);
		ADD_SIMPLE_PROP(DamageTextNode, FLOAT, dist_y);
		ADD_SIMPLE_PROP(DamageTextNode, FLOAT, horizontal_jitter);
		ADD_SIMPLE_PROP(DamageTextNode, FLOAT, pop_duration_ratio);
		ADD_SIMPLE_PROP(DamageTextNode, FLOAT, fade_out_ratio);

		ClassDB::bind_method(D_METHOD("spawn_damage_text", "world_pos", "text", "color", "lifespan"), &DamageTextNode::spawn_damage_text);
	}
	// All nodes
	void init_nodes();

	// Spawns a new floating text item anchored at a 3D world position.
	void spawn_damage_text(Vector3 world_pos, String text, Color color, double lifespan);

	void _process(double delta);

	std::mutex mutex;
protected:
	void _notification(int p_notification);
private:
	void setup();
	// Renders every active item using draw_string. Not wired to NOTIFICATION_DRAW yet.
	void _draw();

	static double _ease_out_back(double t);

	smart_list<DamageTextItem> items;
	Ref<RandomNumberGenerator> rng;
};

}
