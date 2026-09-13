#include "DamageTextNode.h"

#include <iostream>

#include "core/math/math_funcs.h"

namespace godot {

DamageTextNode::~DamageTextNode() {
}

void DamageTextNode::spawn_damage_text(Vector3 world_pos, String text, Color color, double lifespan) {
	std::lock_guard<std::mutex> lock(mutex);
	DamageTextItem item;
	item.text = text;
	item.world_pos = world_pos;
	item.color = color;
	item.lifespan = lifespan;
	item.age = 0.;
	item.scale = 0.;
	item.ray = rng.is_valid() ? rng->randf_range(ray_min, ray_max) : ray_min;
	item.dist_x = rng.is_valid() ? rng->randf_range(0.5, 1.) * (rng->randi_range(0,1)>0 ? 1. : -1.) : 1.;
	item.dist_y = rng.is_valid() ? rng->randf_range(0.5, 0.75) : 1.;
	// Random horizontal jitter so overlapping texts don't stack perfectly.
	item.offset_x = rng.is_valid() ? rng->randf_range(-horizontal_jitter, horizontal_jitter) : 0.;
	item.offset_y = rng.is_valid() ? rng->randf_range(0.2, 0.6) : 0.;
	items.new_instance(item);
}

void DamageTextNode::setup() {
	flecs::world &ecs = _game_node->get_world().ecs;
	// Component reflection is specific to this display system, so it is registered here
	// rather than in the shared trigger_module declaration.
	ecs.component<trigger_module::DamageText>()
		.member("amount", &trigger_module::DamageText::amount)
		.member("is_heal", &trigger_module::DamageText::is_heal);

	ecs.observer<octopus::Position const, trigger_module::DamageText const>()
		.template event<trigger_module::DamageText>()
		.each([this](flecs::entity e, octopus::Position const &pos, trigger_module::DamageText const &damage_text) {
			Vector3 world_pos = WORLD_SCALE * Vector3(real_t(octopus::to_double(pos.pos.x)), 0.5, real_t(octopus::to_double(pos.pos.y)));
			const double val = std::abs(octopus::to_double(damage_text.amount));
			const double rounded_val = std::round(val);
			const double lifespan = 0.5;
			String text;
			if (val > 1000) {
				text = String::num(std::round(val/100.)/10., 1) + "k";
			} else if (val > 10) {
				text = String::num(rounded_val, 0);
			} else if (std::abs(rounded_val - val) < 0.1) {
				text = String::num(rounded_val, 0);
			} else {
				text = String::num(val, 1);
			}
			if (damage_text.is_heal) {
				spawn_damage_text(world_pos, "+" + text, Color(0, 1, 0, 1), lifespan);
			} else {
				spawn_damage_text(world_pos, text, Color(1, 0, 0, 1), lifespan);
			}
		});
}

// Elastic-ish "pop" easing: overshoots past 1.0 before settling, giving a lively bounce.
double DamageTextNode::_ease_out_back(double t) {
	const double c1 = 1.70158;
	const double c3 = c1 + 1.;
	double t1 = t - 1.;
	return 1. + c3 * t1 * t1 * t1 + c1 * t1 * t1;
}

void DamageTextNode::_process(double delta) {
	std::lock_guard<std::mutex> lock(mutex);
	std::vector<size_t> expired;
	items.for_each([&](DamageTextItem &item, size_t idx) {
		item.age += delta;
		if (item.age >= item.lifespan) {
			expired.push_back(idx);
			return;
		}

		// Swirl motion: t grows logarithmically, so the swirl spins fast on spawn then
		// decelerates into a slow, alive-feeling sway as the item settles.
		double t = 3.5 * Math::log(1. + item.age);
		item.offset.x = real_t(item.ray * Math::cos(t) * item.dist_x - item.ray * item.dist_x + item.offset_x);
		item.offset.y = - real_t(item.ray * Math::sin(t) * item.dist_y + item.offset_y);

		double ratio = item.age / item.lifespan;
		if (ratio < pop_duration_ratio) {
			item.scale = _ease_out_back(ratio / pop_duration_ratio);
		} else {
			item.scale = 1.;
		}
	});
	// Free expired items after iterating, since smart_list marks slots inactive rather than resizing.
	for (size_t idx : expired) {
		items.free_instance(idx);
	}
	queue_redraw();
}

// Renders every active item using draw_string_outline.
void DamageTextNode::_draw() {
	if (!font.is_valid()) {
		return;
	}
	std::lock_guard<std::mutex> lock(mutex);
	items.for_each_const([&](DamageTextItem const &item) {
		Vector2 screen_pos = item.offset;
		if (_camera) {
			screen_pos += _camera->unproject_position(item.world_pos);
		}

		double t = item.age / item.lifespan;
		double alpha = 1.;
		if (t > fade_out_ratio) {
			alpha = 1. - (t - fade_out_ratio) / (1. - fade_out_ratio);
		}
		Color color = item.color;
		color.a *= real_t(alpha);

		int size = std::max(int(double(font_size) * item.scale), 1);
		draw_string_outline(font, screen_pos, item.text, HORIZONTAL_ALIGNMENT_CENTER, -1, size, 5, Color(0, 0, 0, alpha));
		draw_string(font, screen_pos, item.text, HORIZONTAL_ALIGNMENT_CENTER, -1, size, color);
	});
}

void DamageTextNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	INIT_NODE_PATH(Camera3D, camera);
	if(_game_node) {
		if (_game_node->init_done()) {
			setup();
		} else {
			_game_node->connect("init_done", callable_mp(this, &DamageTextNode::setup));
		}
	}
}

void DamageTextNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			init_nodes();
			rng.instantiate();
			set_process(true);
		} break;
		case NOTIFICATION_DRAW: {
			_draw();
		} break;
	}
}

}
