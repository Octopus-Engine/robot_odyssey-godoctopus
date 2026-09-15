#pragma once

#include "scene/3d/multimesh_instance_3d.h"
#include "godot_tools.h"

#include "smart_list/smart_list.h"
#include <mutex>

namespace godot {

/// @brief MultiMesh wrapper that interpolates instance transforms over time.
///
/// This node maintains a single Godot MultiMesh for many lightweight instances,
/// keeping both the previous and "new" transforms for each entry so movement can
/// be smoothed between discrete simulation ticks. The instance color and outline
/// color are also stored alongside the transform data, which allows visual
/// feedback such as selection or status highlighting to be updated without
/// recreating the mesh.
class SmartMultiMeshInstance : public MultiMeshInstance3D {
	GDCLASS(SmartMultiMeshInstance, MultiMeshInstance3D)
public:
	~SmartMultiMeshInstance() {}

	/// @brief Duration of a single interpolation step in seconds.
	///
	/// When the node updates, the current transform is computed by blending from
	/// the old transform to the new transform over this interval.
	SET_GET_PARAM_DEF(double, time_step, 0.01);

	/// @brief Disables automatic rotation adjustments during translation updates.
	SET_GET_PARAM_DEF(bool, disable_rotation, false);

	/// @brief Number of process ticks to skip between visual refreshes.
	SET_GET_PARAM_DEF(int, refresh_factor, 1);

public:
	/// @brief Initializes the underlying MultiMesh and prepares instance storage.
	void _ready();

	/// @brief Advances the interpolation state and writes the current transforms to the mesh.
	/// @param delta Elapsed game time since the previous frame.
	void _process(double delta);

	/// @brief Creates a new instance slot and returns its stable index.
	/// @return Index of the newly allocated instance.
	int add_instance();

	/// @brief Releases an instance from the list and clears its storage.
	/// @param instance_id Index of the instance to free.
	void free_instance(int instance_id);

	/// @brief Returns the number of active instances.
	/// @return Active instance count.
	int size() const {
		return data.size();
	}

	/// @brief Sets the base color used for the instance.
	/// @param instance_id Instance being updated.
	/// @param color New visual color.
	void set_color(int instance_id, Color color);

	/// @brief Sets the outline/secondary color associated with the instance.
	/// @param instance_id Instance being updated.
	/// @param color New outline color.
	void set_outline_color(int instance_id, Color color);

	/// @brief Sets both the previous and current transform for an instance.
	/// @param instance_id Instance being updated.
	/// @param transform New transform value stored for both states.
	void set_instance_transform(int instance_id, Transform3D const &transform);

	/// @brief Updates the current target transform without changing the previous one.
	/// @param instance_id Instance being updated.
	/// @param transform Target transform for the next interpolation step.
	void set_new_instance_transform(int instance_id, Transform3D const &transform);

	/// @brief Returns the transform stored before the current interpolation step.
	/// @param instance_id Instance to query.
	/// @return Previous transform value.
	Transform3D get_old_instance_transform(int instance_id) const;

	/// @brief Returns the transform interpolated between old and new states.
	/// @param instance_id Instance to query.
	/// @return Interpolated transform for the current elapsed time.
	Transform3D get_current_instance_transform(int instance_id) const;

	/// @brief Swaps the old and new transform buffers and resets the interpolation timer.
	void swap_transforms();

	/// @brief Moves an instance by a translation delta while keeping its target orientation.
	/// @param instance_id Instance being moved.
	/// @param translation Offset to apply to the new transform origin.
	void set_instance_translation(int instance_id, Vector3 const &translation);

	/// @brief Sets a target world-space position relative to the previous transform.
	/// @param instance_id Instance being moved.
	/// @param new_position Desired final world position.
	void set_instance_new_position(int instance_id, Vector3 const &new_position);

	/// @brief Rotates the instance so that its facing direction aligns with the provided vector.
	/// @param instance_id Instance being rotated.
	/// @param direction Desired forward direction in world space.
	void snap_rotation(int instance_id, Vector3 direction);

	/// @brief Optional mutex used by external threads to protect data access.
	std::mutex *_mutex = nullptr;

	/// @brief Locks the instance mutex when it is available.
	void lock() { if(_mutex) { _mutex->lock(); } }

	/// @brief Unlocks the instance mutex when it is available.
	void unlock() { if(_mutex) { _mutex->unlock(); } }
protected:
	/// @brief Registers the Godot-exposed methods and properties with the engine.
	static void _bind_methods();

	/// @brief Handles Godot lifecycle callbacks for ready and process notifications.
	/// @param p_notification Notification type sent by Godot.
	void _notification(int p_notification);

	/// @brief Time elapsed since the last transform swap was performed.
	double elapsed_time = 0.;

	/// @brief Per-instance visual data stored alongside the transform history.
	struct MultiMeshData {
		Color color;
		Color outline_color;
	};

	/// @brief Active instance data, keyed by stable instance index.
	smart_list<MultiMeshData> data;

	/// @brief Previous transform for each instance in the mesh.
	std::vector<Transform3D> old_transform;

	/// @brief Target transform for each instance in the mesh.
	std::vector<Transform3D> new_transform;
};

}
