#pragma once

#include "scene/main/node.h"

#include "octopus_types.h"
#include "octopus/systems/input/Input.hh"

#include "godoctopus/entity_group/EntityGroup.h"
#include "godoctopus/game/GameNode.h"
#include "InputStatusProxyData.h"

#include <mutex>
#include <string>
#include <unordered_map>

namespace godot {

class InputStatusProxyNode : public Node {
	GDCLASS(InputStatusProxyNode, Node)

	SET_GET_NODE_PATH(GameNode, game_node);
public:
	static void _bind_methods();

	void setup();
	Ref<InputStatusResource> get_cast_status_from_group(Ref<EntityGroup> group, String const &cast_name);
	Ref<InputStatusResource> get_production_status_from_group(Ref<EntityGroup> group, String const &production_name);

	void drop_cast_query(Ref<EntityGroup> group, String const &cast_name);
	void drop_production_query(Ref<EntityGroup> group, String const &production_name);
	void drop_all_cast_queries();
	void drop_all_production_queries();

	void init_nodes();
protected:
	void _notification(int p_notification);
private:
	struct CastQuery {
		Ref<EntityGroup> group;
		std::string cast_name;
		Ref<InputStatusResource> status;
	};

	struct ProductionQuery {
		Ref<EntityGroup> group;
		std::string production_name;
		Ref<InputStatusResource> status;
	};

	static std::string _build_query_key(char const *prefix, uint64_t group_id, String const &name);
	static Ref<InputStatusResource> _duplicate_status_resource(Ref<InputStatusResource> const &resource);
	void _refresh_cast_queries(flecs::world &ecs);
	void _refresh_production_queries(flecs::world &ecs);

	mutable std::mutex _mutex;
	std::unordered_map<std::string, CastQuery> _cast_queries;
	std::unordered_map<std::string, ProductionQuery> _production_queries;
	bool _setup_done = false;
};

} // namespace godot
