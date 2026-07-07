#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"
#include "godoctopus/components/player/PlayerLoadout.h"
#include "godoctopus/proxy/data/player/UnitRuneSlotResource.h"

namespace godot {

class UnitLoadoutResource : public Resource {
	GDCLASS(UnitLoadoutResource, Resource)

	SET_GET_PARAM_DEF(String, prefab_name, "");
	SET_GET_PARAM(TypedArray<UnitRuneSlotResource>, slots);
public:

	UnitLoadout to_data() const {
		UnitLoadout loadout;
		loadout.prefab_name = get_prefab_name().utf8().get_data();
		for (int i = 0; i < slots.size(); ++i) {
			Ref<UnitRuneSlotResource> const slot_resource = slots[i];
			loadout.slots.push_back(slot_resource->to_data());
		}
		return loadout;
	}

	void from_data(const UnitLoadout &loadout) {
		set_prefab_name(loadout.prefab_name.c_str());
		slots.clear();
		slots.resize((int)loadout.slots.size());
		for (int i = 0; i < (int)loadout.slots.size(); ++i) {
			Ref<UnitRuneSlotResource> slot_resource = Ref<UnitRuneSlotResource>(memnew(UnitRuneSlotResource));
			slot_resource->from_data(loadout.slots[i]);
			slots[i] = slot_resource;
		}
	}

	Ref<UnitLoadoutResource> duplicate() const {
		Ref<UnitLoadoutResource> copy = Ref<UnitLoadoutResource>(memnew(UnitLoadoutResource));
		copy->set_prefab_name(get_prefab_name());
		for (int i = 0; i < slots.size(); ++i) {
			Ref<UnitRuneSlotResource> const slot_resource = slots[i];
			copy->slots.append(slot_resource->duplicate());
		}
		return copy;
	}

	static void _bind_methods() {
		ADD_SIMPLE_PROP(UnitLoadoutResource, STRING, prefab_name);
		ADD_ARRAY_OBJECT_PROP(UnitLoadoutResource, UnitRuneSlotResource, slots);
	}
};

}
