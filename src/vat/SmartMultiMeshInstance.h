#pragma once

#include "scene/3d/multimesh_instance_3d.h"
#include "godot_tools.h"

#include "smart_list/smart_list.h"
#include <mutex>

namespace godot {


class SmartMultiMeshInstance : public MultiMeshInstance3D {
	GDCLASS(SmartMultiMeshInstance, MultiMeshInstance3D)
public:
	~SmartMultiMeshInstance() {}

	/// @brief expected duration of a timestep
	SET_GET_PARAM_DEF(double, time_step, 0.01);

public:
	void _ready();
	void _process(double delta);

	int add_instance();
	void free_instance(int instance_id);

	int size() const {
		return data.size();
	}

	void set_color(int instance_id, Color color);
	void set_outline_color(int instance_id, Color color);
	void set_instance_transform(int instance_id, Transform3D const &transform);
	void set_new_instance_transform(int instance_id, Transform3D const &transform);
	Transform3D get_old_instance_transform(int instance_id) const;
	Transform3D get_current_instance_transform(int instance_id) const;
	void swap_transforms();

	// helpers
	void set_instance_translation(int instance_id, Vector3 const &translation);
	void set_instance_new_position(int instance_id, Vector3 const &new_position);
	void snap_rotation(int instance_id, Vector3 direction);

	std::mutex *_mutex = nullptr;
	void lock() { if(_mutex) { _mutex->lock(); } }
	void unlock() { if(_mutex) { _mutex->unlock(); } }
protected:
	static void _bind_methods();
	void _notification(int p_notification);

	/// @brief time since last position update
	double elapsed_time = 0.;

	struct MultiMeshData {
		Color color;
		Color outline_color;
	};
	smart_list<MultiMeshData> data;
	std::vector<Transform3D> old_transform;
	std::vector<Transform3D> new_transform;
};

}
