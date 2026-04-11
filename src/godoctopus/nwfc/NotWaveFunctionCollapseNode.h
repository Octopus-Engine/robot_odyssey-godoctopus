#pragma once

#include <map>

#include "scene/main/node.h"

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include "state/State.hh"
#include "pickers/variable/VariablePicker.hh"
#include "variable/layout/2d/GridLayout.hh"

namespace godot {

class NotWaveFunctionCollapseNode : public Node {
	GDCLASS(NotWaveFunctionCollapseNode, Node)

public:

	void grid_setup(int domain_size, int x, int y);

	void add_distance_constraint(int distance, int value, TypedArray<int> incomp);
	void add_lower_cardinality(TypedArray<int> vars, int value, int lb);
	void add_upper_cardinality(TypedArray<int> vars, int value, int lb);
	void add_compatibility_constraint(int variable, int value, int affected_var, int affected_value);

	TypedArray<int> get_row(int row);

	TypedArray<int> get_vars_idx_by_distance(int x, int y, int lower_distance, int upper_distance);

	/// @brief Remove values from variables within a certain distance interval from a position
	/// @param x The x-coordinate of the position
	/// @param y The y-coordinate of the position
	/// @param lower_distance The minimum distance from the position (every variable at a distance strictly lower than this will be affected)
	/// @param upper_distance The maximum distance from the position (every variable at a distance strictly greater than this will be affected)
	/// @param values The values to be removed
	void remove_values_by_distance(int x, int y, int lower_distance, int upper_distance, TypedArray<int> values);
	void remove_value_at(int x, int y, int value);

	void add_value_oriented_variable_picker(int count, int value);
	bool has_pickers();

	void run();
	bool advance();
	float get_progress() const;
	bool is_done() const;

	void set_seed(int seed);
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();
protected:
	void _notification(int p_notification);

	std::unique_ptr<nwfc::State> state;
	std::unique_ptr<nwfc::GridLayout> grid;
	std::list<std::unique_ptr<nwfc::VariablePicker>> pickers;

	std::thread _thread;
	mutable std::mutex _mutex;
	std::atomic<bool> _solving_done{false};
	std::atomic<bool> _interrupted{false};
};

}
