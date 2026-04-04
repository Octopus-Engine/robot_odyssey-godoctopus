#include "NotWaveFunctionCollapseNode.h"

#include "variable/domain/BitsetDomain.hh"
#include "state/State.hh"
#include "pickers/value/WeightBasedValuePicker.hh"
#include "pickers/variable/ValueOrientedVariablePicker.hh"
#include "constraint/bitset/AllDifferentBitset.hh"
#include "constraint/bitset/CompatibilityBitset.hh"
#include "constraint/bitset/CardinalityBitset.hh"
#include "constraint/bitset/ManhattanIncompatibilityBitset.hh"
#include "variable/layout/2d/GridLayout.hh"

namespace godot {

void NotWaveFunctionCollapseNode::grid_setup(int domain_size, int x, int y) {
	state = std::make_unique<nwfc::State>();
	grid = std::make_unique<nwfc::GridLayout>(x, y);
	for (std::size_t i = 0; i < grid->size(); ++i) {
		state->domains.push_back(nwfc::BitsetDomain{std::vector<bool>(domain_size, true), "var" + std::to_string(i)});
	}
	state->init();
}

static std::vector<size_t> convert(TypedArray<int> array) {
	std::vector<std::size_t> vec;
	for(int i = 0 ; i < array.size() ; ++i) {
		vec.push_back(array[i]);
	}
	return vec;
}

void NotWaveFunctionCollapseNode::add_distance_constraint(int distance, int value, TypedArray<int> incomp) {
	state->constraints.emplace_back(create_manhattan_incompatibility_constraint(*grid, distance, static_cast<std::size_t>(value), convert(incomp)));
}
void NotWaveFunctionCollapseNode::add_lower_cardinality(TypedArray<int> vars, int value, int lb) {
	state->constraints.emplace_back(nwfc::CardinalityBitset::newLowerCardinality(convert(vars), value, lb));
}
void NotWaveFunctionCollapseNode::add_upper_cardinality(TypedArray<int> vars, int value, int lb) {
	state->constraints.emplace_back(nwfc::CardinalityBitset::newUpperCardinality(convert(vars), value, lb));
}
void NotWaveFunctionCollapseNode::add_compatibility_constraint(int variable, int value, int affected_var, int affected_value) {
	// Todo
}

TypedArray<int> NotWaveFunctionCollapseNode::get_row(int row) {
	TypedArray<int> res;

	for (size_t i = 0 ; i < grid->width ; ++i ) {
		size_t idx = row * grid->width + i;
		if (nwfc::is_assigned(*state, idx)) {
			res.append(nwfc::get_assigned_value(state->domains[idx]));
		} else {
			res.append(-1);
		}
	}

	return res;
}

void NotWaveFunctionCollapseNode::add_value_oriented_variable_picker(int count, int value) {
	pickers.push_back(std::make_unique<nwfc::ValueOrientedVariablePicker>(count, value));
}

bool NotWaveFunctionCollapseNode::has_pickers() {
	return !pickers.empty();
}

void NotWaveFunctionCollapseNode::run() {
	if (!state || _thread.joinable()) {
		return;
	}
	_solving_done = false;
	_thread = std::thread([this]() {
		while (true) {
			bool done;
			{
				std::lock_guard<std::mutex> lock(_mutex);
				done = advance();
			}
			if (done) {
				_solving_done = true;
				break;
			}
		}
	});
}

float NotWaveFunctionCollapseNode::get_progress() const {
	if (!state) {
		return 0.0f;
	}
	std::lock_guard<std::mutex> lock(_mutex);
	std::size_t assigned_count = std::count(state->assigned.begin(), state->assigned.end(), true);
	return static_cast<float>(assigned_count) / static_cast<float>(state->domains.size());
}

bool NotWaveFunctionCollapseNode::is_done() const {
	return _solving_done.load();
}

bool NotWaveFunctionCollapseNode::advance() {
	std::size_t var = greedy_pick_variable(*state);
	while (!pickers.empty()) {
		auto &picker = pickers.front();
		if (picker->is_done(*state)) {
			pickers.pop_front();
		} else {
			var = picker->pick(*state);
			break;
		}
	}
	if (var == state->domains.size()) {
		// over
		return true;
	}
	std::size_t val = random_pick_value(*state, var);
	if (!pickers.empty() && pickers.front()->get_value_picker()) {
		val = pickers.front()->get_value_picker()->pick(*state, var);
	}
	progress(*state, var, val);
	return false;
}

void NotWaveFunctionCollapseNode::set_seed(int seed) {
	state->generator.seed(seed);
}

// Will be called by Godot when the class is registered
// Use this to add properties to your class
void NotWaveFunctionCollapseNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &NotWaveFunctionCollapseNode::set_seed);
	ClassDB::bind_method(D_METHOD("grid_setup", "domain_size", "x", "y"), &NotWaveFunctionCollapseNode::grid_setup);
	ClassDB::bind_method(D_METHOD("add_distance_constraint", "dist", "val", "incomp"), &NotWaveFunctionCollapseNode::add_distance_constraint);
	ClassDB::bind_method(D_METHOD("add_lower_cardinality", "vars", "value", "bound"), &NotWaveFunctionCollapseNode::add_lower_cardinality);
	ClassDB::bind_method(D_METHOD("add_upper_cardinality", "vars", "value", "bound"), &NotWaveFunctionCollapseNode::add_upper_cardinality);
	ClassDB::bind_method(D_METHOD("add_compatibility_constraint", "var", "value", "affected_var", "affected_val"), &NotWaveFunctionCollapseNode::add_compatibility_constraint);
	ClassDB::bind_method(D_METHOD("get_row", "row"), &NotWaveFunctionCollapseNode::get_row);
	ClassDB::bind_method(D_METHOD("add_value_oriented_variable_picker", "count", "value"), &NotWaveFunctionCollapseNode::add_value_oriented_variable_picker);
	ClassDB::bind_method(D_METHOD("has_pickers"), &NotWaveFunctionCollapseNode::has_pickers);

	ClassDB::bind_method(D_METHOD("run"), &NotWaveFunctionCollapseNode::run);
	ClassDB::bind_method(D_METHOD("advance"), &NotWaveFunctionCollapseNode::advance);
	ClassDB::bind_method(D_METHOD("get_progress"), &NotWaveFunctionCollapseNode::get_progress);
	ClassDB::bind_method(D_METHOD("is_done"), &NotWaveFunctionCollapseNode::is_done);
}

void NotWaveFunctionCollapseNode::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_EXIT_TREE: {
			if (_thread.joinable()) {
				_thread.join();
			}
		} break;
		case NOTIFICATION_PROCESS: {
			// _process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			// init_nodes();
		} break;
	}
}

}
