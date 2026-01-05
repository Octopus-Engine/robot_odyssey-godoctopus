#pragma once

#include "scene/3d/multimesh_instance_3d.h"
#include "vat/VatAnimationTrack.h"
#include "vat/VatInstanceData.h"
#include "godot_tools.h"

#include "smart_list/smart_list.h"
#include <mutex>

namespace godot {

class VatMultiMeshInstance : public MultiMeshInstance3D {
	GDCLASS(VatMultiMeshInstance, MultiMeshInstance3D)
public:
	~VatMultiMeshInstance() {}

	SET_GET_PARAM(Ref<VatAnimationTrack>, track);
	SET_GET_PARAM_DEF(int, instance_count, 0);
	/// @brief expected duration of a timestep
	SET_GET_PARAM_DEF(double, time_step, 0.01);

public:
	void _ready();
	void _process(double delta);

	int add_instance();
	void free_instance(int instance_id);

	void update_instance_track(int instance_id, int track_number, bool one_shot);
	void set_pickable_color(int instance_id, Color color);
	void set_speed(int instance_id, float speed);
	void set_alt_texture(int instance_id, bool alt_texture);

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

	double total_elapsed = 0.;

	smart_list<VatInstanceData> data;
	std::vector<Transform3D> old_transform;
	std::vector<Transform3D> new_transform;
};

}
