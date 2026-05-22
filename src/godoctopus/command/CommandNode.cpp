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

    std::string cast_name_str = cast_name.utf8().get_data();
	flecs::entity entity_target;
	if (target->get_entities().size() > 0) {
		entity_target = target->get_entities()[0];
	}
	octopus::CastCommand cast_cmd {cast_name_str, entity_target, {world_target.x/WORLD_SCALE,world_target.z/WORLD_SCALE}};
    _input_container->addInputCast({group->get_entities(), cast_cmd, queue});
}

void CommandNode::all_cast_command(Ref<EntityGroup> group, String const &cast_name, Ref<EntityGroup> target, Vector3 const &world_target, bool queue) {
    if(!_input_container) { return; }
    octopus::InputCommandFunctor<custom_variant, custom_step_manager> command;

    std::string cast_name_str = cast_name.utf8().get_data();
	flecs::entity entity_target;
	if (target->get_entities().size() > 0) {
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
    _input_container->newProduction({group->get_entities(), prod_name_str});
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

        // register production
        container.container_production.get_front_layer().push_back({entities, prod_name_str});

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
