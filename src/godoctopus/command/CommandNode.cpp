#include "CommandNode.h"

#include "octopus/world/ability/AbilityTemplateLibrary.hh"

namespace godot {

static int64_t get_reload_time(flecs::world const &ecs, std::string const &cast_name) {

	int64_t reload = 0;
	auto ability_library = ecs.try_get<octopus::AbilityTemplateLibrary<custom_step_manager> >();
	if(ability_library)
	{
		octopus::AbilityTemplate<custom_step_manager> const & cast_template = ability_library->get(cast_name);
		reload = cast_template.reload();
	}
	return reload;
}

static bool can_cast(flecs::world const &ecs,
					 flecs::entity e,
					 std::string const &cast_name,
					 int64_t reload_time) {
	if (!e.is_valid()) {
		return false;
	}
	auto caster = e.try_get<octopus::Caster>();
	return caster
		&& e.has<octopus::Caster>(ecs.component(cast_name.c_str()))
		&& caster->check_timestamp_last_cast(reload_time, octopus::get_time_stamp(ecs), cast_name);
}

static flecs::entity find_best_entity_for_production(
	flecs::world const &ecs,
	std::vector<flecs::entity> const &entities,
	std::string const &production_name_p
) {
	using namespace octopus;
	auto &&prod_library = ecs.try_get<ProductionTemplateLibrary<custom_step_manager> >();
	if(!prod_library) {
		return flecs::entity();
	}
	ProductionTemplate<custom_step_manager> const & prod_template = prod_library->get(production_name_p);

	flecs::entity best_ent;
	int64_t best_end_time = -1;
	for(flecs::entity const &e : entities) {
		if(!e.is_valid()) {
			continue;
		}
		octopus::ProductionQueue const * prod_queue = e.try_get<octopus::ProductionQueue>();
		if(prod_queue
		&& e.has<octopus::ProductionQueue>(ecs.component(production_name_p.c_str()))
		&& prod_template.can_produce(e, ecs)) {
			int64_t const queue_duration = octopus::get_queue_duration(*prod_library, prod_queue->queue);
			int64_t const start = prod_queue->start_timestamp;
			if(best_end_time < 0 || start + queue_duration < best_end_time)
			{
				best_end_time = start + queue_duration;
				best_ent = e;
			}
		}
	}
	return best_ent;
}

static flecs::entity find_best_entity_for_casting(flecs::world const &ecs,
												  Ref<EntityGroup> group,
												  std::string const &cast_name,
												  octopus::Vector const &target_pos) {
	using namespace octopus;

	int64_t reload = get_reload_time(ecs, cast_name);
	/// @todo Tri entre les entités :
	/// - Proximité
	/// Filtre :
	/// - Caster qui peut cast la demande
	/// - Pas de cast du même sort dans la queue (sauf si reload = 0) (sauf si aucun autre candidat)
	/// - Resource check (en prenant en compte tous les cast dans la queue si booléen queued = true [add])
	for(flecs::entity const &e : group->get_entities()) {
		if(can_cast(ecs, e, cast_name, reload)) {
			return e;
		}
	}
	return flecs::entity();
}

void CommandNode::move_command(Ref<EntityGroup> group, Vector3 const &world_target, bool queue) {
    if(!_input_container) { return; }

    octopus::MoveCommand move {{world_target.x/WORLD_SCALE,world_target.z/WORLD_SCALE}};

    octopus::InputCommandFunctor<custom_variant, custom_step_manager> command;
    command.func = [move, group, queue](
        octopus::WorldContext<custom_step_manager> const &world,
        octopus::InputContainer<custom_variant, custom_step_manager>& container)
        -> octopus::InputCommandPackage<custom_variant>
    {
        octopus::InputCommandPackage<custom_variant> command_package;
        command_package.command = move;
        for(flecs::entity const &e : group->get_entities()) {
            if(e.is_valid()) {
			    command_package.entities.push_back(e);
            }
		}
        command_package.front = !queue;
        return command_package;
    };
    _input_container->addFunctorCommand(command);
}

void CommandNode::attack_move_command(Ref<EntityGroup> group, Vector3 const &world_target, bool queue) {
    if(!_input_container) { return; }

    octopus::AttackCommand atk {flecs::entity(), {world_target.x/WORLD_SCALE,world_target.z/WORLD_SCALE}, true};

    octopus::InputCommandFunctor<custom_variant, custom_step_manager> command;
    command.func = [atk, group, queue](
        octopus::WorldContext<custom_step_manager> const &world,
        octopus::InputContainer<custom_variant, custom_step_manager>& container)
        -> octopus::InputCommandPackage<custom_variant>
    {
        octopus::InputCommandPackage<custom_variant> command_package;
        command_package.command = atk;
        for(flecs::entity const &e : group->get_entities()) {
            if(e.is_valid()) {
			    command_package.entities.push_back(e);
            }
		}
        command_package.front = !queue;
        return command_package;
    };
    _input_container->addFunctorCommand(command);
}

void CommandNode::stop_command(Ref<EntityGroup> group) {
    if(!_input_container) { return; }

    octopus::InputCommandFunctor<custom_variant, custom_step_manager> command;
    command.func = [group](
        octopus::WorldContext<custom_step_manager> const &world,
        octopus::InputContainer<custom_variant, custom_step_manager>& container)
        -> octopus::InputCommandPackage<custom_variant>
    {
        octopus::InputCommandPackage<custom_variant> command_package;
        command_package.command = octopus::NoOpCommand();
        for(flecs::entity const &e : group->get_entities()) {
            if(e.is_valid()) {
			    command_package.entities.push_back(e);
            }
		}
        command_package.front = true;
        return command_package;
    };
    _input_container->addFunctorCommand(command);
}

void CommandNode::cast_command(Ref<EntityGroup> group, String const &cast_name, Ref<EntityGroup> target, Vector3 const &world_target, bool queue) {
    if(!_input_container) { return; }
    octopus::InputCommandFunctor<custom_variant, custom_step_manager> command;

    // TODO move to functor!!!
    std::string cast_name_str = cast_name.utf8().get_data();
	flecs::entity entity_target;
	if (target.is_valid() && target->get_entities().size() > 0) {
		entity_target = target->get_entities()[0];
	}
	octopus::CastCommand cast_cmd {cast_name_str, entity_target, {world_target.x/WORLD_SCALE,world_target.z/WORLD_SCALE}};

    command.func = [cast_name_str, cast_cmd, group, queue](
        octopus::WorldContext<custom_step_manager> const &world,
        octopus::InputContainer<custom_variant, custom_step_manager>& container)
        -> octopus::InputCommandPackage<custom_variant>
    {
        octopus::InputCommandPackage<custom_variant> command_package;
        command_package.command = cast_cmd;
		command_package.entities.push_back(find_best_entity_for_casting(world.ecs, group, cast_name_str, cast_cmd.point_target));
        command_package.front = !queue;
        return command_package;
    };
    _input_container->addFunctorCommand(command);
}

void CommandNode::all_cast_command(Ref<EntityGroup> group, String const &cast_name, Ref<EntityGroup> target, Vector3 const &world_target, bool queue) {
    if(!_input_container) { return; }
    octopus::InputCommandFunctor<custom_variant, custom_step_manager> command;

    // TODO move to functor!!!
    std::string cast_name_str = cast_name.utf8().get_data();
	flecs::entity entity_target;
	if (target.is_valid() && target->get_entities().size() > 0) {
		entity_target = target->get_entities()[0];
	}
	octopus::CastCommand cast_cmd {cast_name_str, entity_target, {world_target.x/WORLD_SCALE,world_target.z/WORLD_SCALE}};

    command.func = [cast_name_str, cast_cmd, group, queue](
        octopus::WorldContext<custom_step_manager> const &world,
        octopus::InputContainer<custom_variant, custom_step_manager>& container)
        -> octopus::InputCommandPackage<custom_variant>
    {
        octopus::InputCommandPackage<custom_variant> command_package;
        command_package.command = cast_cmd;

		int64_t reload_time = get_reload_time(world.ecs, cast_name_str);

        for(flecs::entity const &e : group->get_entities()) {
            if(e.is_valid() && can_cast(world.ecs, e, cast_name_str, reload_time)) {
			    command_package.entities.push_back(e);
            }
		}
        command_package.front = !queue;
        return command_package;
    };
    _input_container->addFunctorCommand(command);
}

void CommandNode::add_production(Ref<EntityGroup> group, String const &prod_name_p){
    if(!_input_container || group->empty()) { return; }
    std::string prod_name_str = prod_name_p.utf8().get_data();

    octopus::InputCommandFunctor<custom_variant, custom_step_manager> command;
    command.func = [this, group, prod_name_str](
        octopus::WorldContext<custom_step_manager> const &world,
        octopus::InputContainer<custom_variant, custom_step_manager>& container)
        -> octopus::InputCommandPackage<custom_variant> {
        flecs::entity e = find_best_entity_for_production(world.ecs, group->get_entities(), prod_name_str);

        if(!e.is_valid()) { return octopus::InputCommandPackage<custom_variant>(); }

        // register production
        container.container_add_production.get_front_layer().push_back({e, prod_name_str});

        return octopus::InputCommandPackage<custom_variant>();
    };
    _input_container->addFunctorCommand(command);
}

void CommandNode::cancel_production(Ref<EntityGroup> mono_unit_group, int queue_index) {
    if(!_input_container || mono_unit_group->empty()) { return; }
    flecs::entity e = mono_unit_group->get_entities()[0];
    if(e.is_valid()) {
        _input_container->cancelProduction(octopus::InputCancelProduction {e, queue_index});
    }
}

void CommandNode::queue_production(int player, String const &prod_name) {
    std::string prod_name_str = prod_name.utf8().get_data();

    octopus::InputCommandFunctor<custom_variant, custom_step_manager> command;
    command.func = [this, player, prod_name_str](
        octopus::WorldContext<custom_step_manager> const &world,
        octopus::InputContainer<custom_variant, custom_step_manager>& container)
        -> octopus::InputCommandPackage<custom_variant> {

        // Get all producer for the player
        auto query = world.ecs.query<const octopus::PlayerAppartenance, const octopus::ProductionQueue>();
        std::vector<flecs::entity> entities;
        query.each([&](flecs::entity e, octopus::PlayerAppartenance const &player_app, octopus::ProductionQueue const &prod_queue) {
            if(player_app.idx == (uint32_t)player && e.has<octopus::ProductionQueue>(world.ecs.component(prod_name_str.c_str()))) {
                entities.push_back(e);
            }
        });

        flecs::entity e = find_best_entity_for_production(world.ecs, entities, prod_name_str);

        if(!e.is_valid()) { return octopus::InputCommandPackage<custom_variant>(); }

        // register production
        container.container_add_production.get_front_layer().push_back({e, prod_name_str});

        return octopus::InputCommandPackage<custom_variant>();
    };
    _input_container->addFunctorCommand(command);
}

// Will be called by Godot when the class is registered
// Use this to add properties to your class
void CommandNode::_bind_methods() {
	BIND_NODE_PATH(CommandNode, GameNode, game_node);

	ClassDB::bind_method(D_METHOD("move_command", "group", "world_target", "queue"), &CommandNode::move_command);
	ClassDB::bind_method(D_METHOD("attack_move_command", "group", "world_target", "queue"), &CommandNode::attack_move_command);
	ClassDB::bind_method(D_METHOD("stop_command", "group"), &CommandNode::stop_command);
	ClassDB::bind_method(D_METHOD("cast_command", "group", "cast_name", "target", "world_target", "queue"), &CommandNode::cast_command);
	ClassDB::bind_method(D_METHOD("all_cast_command", "group", "cast_name", "target", "world_target", "queue"), &CommandNode::all_cast_command);
	ClassDB::bind_method(D_METHOD("add_production", "group", "prod_name"), &CommandNode::add_production);
	ClassDB::bind_method(D_METHOD("cancel_production", "group", "queue_index"), &CommandNode::cancel_production);
	ClassDB::bind_method(D_METHOD("queue_production", "player", "prod_name"), &CommandNode::queue_production);
	ClassDB::bind_method(D_METHOD("setup"), &CommandNode::setup);
}

void CommandNode::setup() {
	_input_container = _game_node->get_input_controller();
}

void CommandNode::init_nodes() {
	INIT_NODE_PATH(GameNode, game_node);
	if(_game_node) {
		_game_node->connect("init_done", callable_mp(this, &CommandNode::setup));
	}
}

void CommandNode::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			// _process(get_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			init_nodes();
		} break;
	}
}

}
