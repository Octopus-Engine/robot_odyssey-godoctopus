#include "BuildingFootprintGrid.h"

#include <algorithm>
#include <cmath>

#include "core/variant/array.h"
#include "godoctopus/action/ActionNode.h"
#include "scene/resources/multimesh.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/resources/material.h"

namespace godot {

namespace {

uint64_t grid_cell_key(int32_t x, int32_t y) {
	return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) |
		static_cast<uint32_t>(y);
}

}

void BuildingFootprintGrid::_bind_methods() {
	ADD_SIMPLE_PROP(BuildingFootprintGrid, STRING, prefab_name);
	ADD_SIMPLE_PROP(BuildingFootprintGrid, FLOAT, floor_y);
	ADD_SIMPLE_PROP(BuildingFootprintGrid, FLOAT, refresh_interval);
	ADD_SIMPLE_PROP(BuildingFootprintGrid, VECTOR2, candidate_position);
	ADD_SIMPLE_PROP(BuildingFootprintGrid, BOOL, candidate_is_visible);
	ADD_SIMPLE_PROP(BuildingFootprintGrid, BOOL, candidate_is_valid);
	ADD_SIMPLE_PROP(BuildingFootprintGrid, STRING, candidate_reason);

	ClassDB::bind_method(D_METHOD("set_placement_active", "active"), &BuildingFootprintGrid::set_placement_active);
	ClassDB::bind_method(D_METHOD("update_candidate", "world_position"), &BuildingFootprintGrid::update_candidate);
	ClassDB::bind_method(D_METHOD("get_visible_cell_bounds"), &BuildingFootprintGrid::get_visible_cell_bounds);
}

void BuildingFootprintGrid::resolve_nodes() {
	Node *game_scene = nullptr;
	for (Node *ancestor = this; ancestor != nullptr; ancestor = ancestor->get_parent()) {
		if (ancestor->get_name() == "GameScene") {
			game_scene = ancestor;
			break;
		}
	}
	if (game_scene == nullptr) {
		ERR_PRINT("BuildingFootprintGrid requires a GameScene ancestor.");
		return;
	}

	_action_node = Object::cast_to<ActionNode>(game_scene->get_node_or_null(
		NodePath("MainViewport/SubViewport/GameLogic/GameNode/ActionNode")));
	_world_input = Object::cast_to<Control>(game_scene->get_node_or_null(
		NodePath("CanvasLayer/WorldInput")));
	if (_action_node == nullptr || _world_input == nullptr) {
		ERR_PRINT("BuildingFootprintGrid could not resolve ActionNode or WorldInput under GameScene.");
	}
}

void BuildingFootprintGrid::set_placement_active(bool active) {
	placement_active = active;
	set_visible(active);
	set_process(active);
	refresh_elapsed = refresh_interval;
	if (!active) {
		has_candidate = false;
		candidate_cells.clear();
		last_snapshot.clear();
		if (get_multimesh().is_valid()) {
			get_multimesh()->set_instance_count(0);
		}
		return;
	}
	refresh_grid();
}

void BuildingFootprintGrid::update_candidate(Vector2 const &world_position) {
	if (!placement_active || _action_node == nullptr) {
		return;
	}
	Vector2i const snapped_center = _action_node->snap_building_position(world_position);
	if (has_candidate && snapped_center == candidate_center) {
		return;
	}

	has_candidate = true;
	candidate_center = snapped_center;
	Dictionary const preview = _action_node->get_building_placement_preview(prefab_name, world_position);
	candidate_position = preview.get("position", Vector2(snapped_center));
	candidate_radius = preview.get("radius", 0.0);
	candidate_cells.clear();
	Array const cells = preview.get("cells", Array());
	candidate_cells.reserve(cells.size());
	for (int i = 0; i < cells.size(); ++i) {
		candidate_cells.push_back(Vector2i(cells[i]));
	}
	preview_valid = preview.get("valid", false).booleanize();
	preview_reason = preview.get("reason", "placement preview unavailable");
	update_candidate_visibility();
	last_snapshot.clear();
}

Rect2i BuildingFootprintGrid::get_visible_cell_bounds() const {
	if (_world_input == nullptr) {
		return Rect2i();
	}
	Rect2 const viewport_rect(Vector2(), _world_input->get_viewport_rect().size);
	Array const corners = _world_input->call("get_world_corners", viewport_rect);
	if (corners.is_empty()) {
		return Rect2i();
	}

	Vector3 first_corner = corners[0];
	real_t min_x = first_corner.x;
	real_t max_x = first_corner.x;
	real_t min_y = first_corner.z;
	real_t max_y = first_corner.z;
	for (int i = 1; i < corners.size(); ++i) {
		Vector3 const corner = corners[i];
		min_x = std::min(min_x, corner.x);
		max_x = std::max(max_x, corner.x);
		min_y = std::min(min_y, corner.z);
		max_y = std::max(max_y, corner.z);
	}

	int const padding = static_cast<int>(std::ceil(candidate_radius)) + 1;
	Vector2i const min_cell(
		static_cast<int32_t>(std::floor(min_x)) - padding,
		static_cast<int32_t>(std::floor(min_y)) - padding);
	Vector2i const max_cell(
		static_cast<int32_t>(std::ceil(max_x)) + padding,
		static_cast<int32_t>(std::ceil(max_y)) + padding);
	return Rect2i(min_cell, max_cell - min_cell + Vector2i(1, 1));
}

void BuildingFootprintGrid::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			resolve_nodes();

			if (!get_multimesh().is_valid()) {
				Ref<PlaneMesh> quad;
				quad.instantiate();
				quad->set_size(Vector2(1.0, 1.0));

				Ref<MultiMesh> instances;
				instances.instantiate();
				instances->set_transform_format(MultiMesh::TRANSFORM_3D);
				instances->set_use_colors(true);
				instances->set_mesh(quad);
				set_multimesh(instances);
			}

			Ref<StandardMaterial3D> material;
			material.instantiate();
			material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
			material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
			material->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
			material->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
			set_material_override(material);
			set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
			set_visible(false);
			set_process(false);
		} break;
		case NOTIFICATION_PROCESS: {
			refresh_elapsed += get_process_delta_time();
			if (refresh_elapsed < refresh_interval) {
				break;
			}
			refresh_elapsed = 0.0;
			refresh_grid();
		} break;
	}
}

void BuildingFootprintGrid::refresh_grid() {
	if (!placement_active || !is_ready() || _action_node == nullptr) {
		return;
	}
	update_candidate_visibility();
	Rect2i const bounds = get_visible_cell_bounds();
	if (bounds.size.x <= 0 || bounds.size.y <= 0) {
		return;
	}

	Vector2i const min_cell = bounds.position;
	Vector2i const max_cell = bounds.position + bounds.size - Vector2i(1, 1);
	Array const occupied_array = _action_node->get_building_occupied_cells(min_cell, max_cell);
	GridCellSet occupied_cells;
	occupied_cells.reserve(occupied_array.size());
	for (int i = 0; i < occupied_array.size(); ++i) {
		Vector2i const cell = occupied_array[i];
		occupied_cells.insert(grid_cell_key(cell.x, cell.y));
	}
	refresh_candidate_occupancy(occupied_cells);
	update_candidate_visibility();

	GridCellSet candidate_cell_set;
	candidate_cell_set.reserve(candidate_cells.size());
	if (candidate_is_visible) {
		for (Vector2i const &cell : candidate_cells) {
			candidate_cell_set.insert(grid_cell_key(cell.x, cell.y));
		}
	}

	std::vector<int32_t> snapshot;
	snapshot.reserve(static_cast<size_t>(bounds.size.x) * static_cast<size_t>(bounds.size.y) * 3);
	for (int32_t y = min_cell.y; y <= max_cell.y; ++y) {
		for (int32_t x = min_cell.x; x <= max_cell.x; ++x) {
			uint64_t const key = grid_cell_key(x, y);
			int32_t status = occupied_cells.count(key) > 0 ? 1 : 0;
			if (candidate_cell_set.count(key) > 0) {
				status = candidate_is_valid ? 2 : 3;
			}
			snapshot.push_back(x);
			snapshot.push_back(y);
			snapshot.push_back(status);
		}
	}

	if (snapshot == last_snapshot) {
		return;
	}
	last_snapshot = std::move(snapshot);
	render_snapshot(last_snapshot);
}

void BuildingFootprintGrid::refresh_candidate_occupancy(GridCellSet const &occupied_cells) {
	if (candidate_cells.empty()) {
		return;
	}
	if (!preview_valid && preview_reason != "building footprint cells are occupied") {
		return;
	}
	for (Vector2i const &cell : candidate_cells) {
		if (occupied_cells.count(grid_cell_key(cell.x, cell.y)) > 0) {
			preview_valid = false;
			preview_reason = "building footprint cells are occupied";
			return;
		}
	}
	preview_valid = true;
	preview_reason = String();
}

void BuildingFootprintGrid::update_candidate_visibility() {
	candidate_is_visible = !candidate_cells.empty();
	candidate_is_valid = candidate_is_visible && preview_valid;
	candidate_reason = preview_reason;
}

void BuildingFootprintGrid::render_snapshot(std::vector<int32_t> const &snapshot) {
	Ref<MultiMesh> instances = get_multimesh();
	if (instances.is_null()) {
		return;
	}
	int const count = static_cast<int>(snapshot.size() / 3);
	instances->set_instance_count(count);
	instances->set_visible_instance_count(count);
	Vector3 const scale(0.96, 1.0, 0.96);
	for (int index = 0; index < count; ++index) {
		size_t const offset = static_cast<size_t>(index) * 3;
		Vector2i const cell(snapshot[offset], snapshot[offset + 1]);
		int32_t const status = snapshot[offset + 2];
		Color color(0.15, 0.78, 0.42, 0.16);
		if (status == 1) {
			color = Color(0.95, 0.18, 0.12, 0.42);
		} else if (status == 2) {
			color = Color(0.12, 1.0, 0.34, 0.52);
		} else if (status == 3) {
			color = Color(1.0, 0.18, 0.12, 0.58);
		}
		Transform3D const cell_transform(
			Basis().scaled(scale),
			Vector3(cell.x, floor_y, cell.y));
		instances->set_instance_transform(index, cell_transform);
		instances->set_instance_color(index, color);
	}
}

} // namespace godot
