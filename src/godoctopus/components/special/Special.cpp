#include "Special.h"

octopus::Fixed get_special_value(flecs::entity e) {
	auto special = e.try_get<Special>();
	if(special) {
		return special->value * special->affinity;
	} else {
		return octopus::Fixed(0);
	}
}
