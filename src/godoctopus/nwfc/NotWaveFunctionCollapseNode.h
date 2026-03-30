#pragma once

#include <map>

#include "scene/main/node.h"

#include <list>
#include <memory>
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

	void add_value_oriented_variable_picker(int count, int value);
	bool has_pickers();

	void run();
	bool advance();

	void set_seed(int seed);
	// Will be called by Godot when the class is registered
	// Use this to add properties to your class
	static void _bind_methods();
protected:
	void _notification(int p_notification);

	std::unique_ptr<nwfc::State> state;
	std::unique_ptr<nwfc::GridLayout> grid;
	std::list<std::unique_ptr<nwfc::VariablePicker>> pickers;
};

}
