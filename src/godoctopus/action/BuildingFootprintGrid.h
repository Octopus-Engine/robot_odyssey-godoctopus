#pragma once

#include "core/math/rect2i.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/gui/control.h"

#include "godot_tools.h"

#include <cstdint>
#include <utility>
#include <unordered_set>
#include <vector>

namespace godot {

class ActionNode;

class BuildingFootprintGrid : public MultiMeshInstance3D {
	GDCLASS(BuildingFootprintGrid, MultiMeshInstance3D)

	SET_GET_PARAM_DEF(String, prefab_name, "building_slot");
	SET_GET_PARAM_DEF(double, floor_y, -0.12);
	SET_GET_PARAM_DEF(double, refresh_interval, 0.25);
	SET_GET_PARAM_DEF(Vector2, candidate_position, Vector2());
	SET_GET_PARAM_DEF(bool, candidate_is_visible, false);
	SET_GET_PARAM_DEF(bool, candidate_is_valid, false);
	SET_GET_PARAM_DEF(String, candidate_reason, String());

public:
	static void _bind_methods();

	void set_placement_active(bool active);
	void update_candidate(Vector2 const &world_position);
	Rect2i get_visible_cell_bounds() const;

protected:
	void _notification(int p_notification);

private:
	using GridCellSet = std::unordered_set<uint64_t>;

	void resolve_nodes();
	void refresh_grid();
	void refresh_candidate_occupancy(GridCellSet const &occupied_cells);
	void update_candidate_visibility();
	void render_snapshot(std::vector<int32_t> const &snapshot);

	ActionNode *_action_node = nullptr;
	Control *_world_input = nullptr;

	bool placement_active = false;
	Vector2i candidate_center;
	std::vector<Vector2i> candidate_cells;
	double candidate_radius = 0.0;
	bool has_candidate = false;
	bool preview_valid = false;
	String preview_reason;
	double refresh_elapsed = 0.0;
	std::vector<int32_t> last_snapshot;
};

} // namespace godot
