#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"
#include "godoctopus/components/player/PlayerLoadout.h"

namespace godot {

class RuneInfoResource : public Resource {
	GDCLASS(RuneInfoResource, Resource)

	SET_GET_PARAM_DEF(String, type, "");
	SET_GET_PARAM(Dictionary, data);
public:
	static octopus::fast_map<std::string, int64_t> dictionary_to_fast_map(const Dictionary &dict) {
		octopus::fast_map<std::string, int64_t> fast_map;
		for (const Variant &key : dict.keys()) {
			Variant const &value = dict[key];
			const String key_str = key;
			fast_map[key_str.utf8().get_data()] = static_cast<int64_t>(value);
		}
		return fast_map;
	}

	RuneInfo to_data() const {
		RuneInfo rune_info;
		rune_info.type = type.utf8().get_data();
		rune_info.data = dictionary_to_fast_map(data);
		return rune_info;
	}

	void from_data(const RuneInfo &rune_info) {
		set_type(rune_info.type.c_str());
		data.clear();
		for (const auto &[key, value] : rune_info.data.data()) {
			data[key.c_str()] = value;
		}
	}

	Ref<RuneInfoResource> duplicate() const {
		Ref<RuneInfoResource> copy = Ref<RuneInfoResource>(memnew(RuneInfoResource));
		copy->set_type(get_type());
		copy->data = data;
		return copy;
	}

	static void _bind_methods() {
		ADD_SIMPLE_PROP(RuneInfoResource, STRING, type);
		ADD_SIMPLE_PROP(RuneInfoResource, DICTIONARY, data);
	}
};

}
