#pragma once

#include "core/io/resource.h"

#include "godot_tools.h"
#include "godoctopus/components/player/PlayerLoadout.h"
#include "godoctopus/proxy/data/player/RuneInfoResource.h"

namespace godot {

class UnitRuneSlotResource : public Resource {
	GDCLASS(UnitRuneSlotResource, Resource)

	SET_GET_PARAM(Ref<RuneInfoResource>, rune);
	SET_GET_PARAM_DEF(int, slot_type, 0);
	SET_GET_PARAM_DEF(bool, activated, true);
	SET_GET_PARAM_DEF(bool, has_rune, false);
public:

	static Ref<UnitRuneSlotResource> make_unit_rune_slot(int type, bool activated) {
		Ref<UnitRuneSlotResource> slot_resource = Ref<UnitRuneSlotResource>(memnew(UnitRuneSlotResource));
		slot_resource->set_slot_type(type);
		slot_resource->set_activated(activated);
		slot_resource->set_has_rune(false);
		return slot_resource;
	}

	UnitRuneSlot to_data() const {
		UnitRuneSlot rune_slot;
		if (get_rune().is_valid()) {
			rune_slot.rune = get_rune()->to_data();
		}
		rune_slot.slot_type = get_slot_type();
		rune_slot.activated = get_activated();
		rune_slot.has_rune = get_has_rune();
		return rune_slot;
	}

	void from_data(const UnitRuneSlot &rune_slot) {
		Ref<RuneInfoResource> rune_resource = memnew(RuneInfoResource);
		rune_resource->from_data(rune_slot.rune);
		set_rune(rune_resource);
		set_slot_type(rune_slot.slot_type);
		set_activated(rune_slot.activated);
		set_has_rune(rune_slot.has_rune);
	}

	Ref<UnitRuneSlotResource> duplicate() const {
		Ref<UnitRuneSlotResource> copy = Ref<UnitRuneSlotResource>(memnew(UnitRuneSlotResource));
		copy->set_rune(get_rune()->duplicate());
		copy->set_slot_type(get_slot_type());
		copy->set_activated(get_activated());
		copy->set_has_rune(get_has_rune());
		return copy;
	}

	static void _bind_methods() {
		ADD_OBJECT_PROP(UnitRuneSlotResource, RuneInfoResource, rune);
		ADD_SIMPLE_PROP(UnitRuneSlotResource, INT, slot_type);
		ADD_SIMPLE_PROP(UnitRuneSlotResource, BOOL, activated);
		ADD_SIMPLE_PROP(UnitRuneSlotResource, BOOL, has_rune);
	}
};

}
