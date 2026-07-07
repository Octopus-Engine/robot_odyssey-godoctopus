#pragma once

#include "core/object/class_db.h"
#include "core/object/callable_mp.h"

#define SET_GET_NODE_PATH(type, var_name) \
public:\
	void set_ref_##var_name(NodePath const &node_path) { _ref_##var_name = node_path; }\
	NodePath get_ref_##var_name() const { return _ref_##var_name; }\
\
	void set_##var_name(type * val) { _##var_name = val; }\
	type * get_##var_name() { return _##var_name; }\
private:\
	NodePath _ref_##var_name;\
	type * _##var_name = nullptr;

#define INIT_NODE_PATH(type, var_name) \
	if(!_ref_##var_name.is_empty())\
		_##var_name = (type*)get_node(_ref_##var_name);

#define BIND_NODE_PATH(class_name, type, var_name ) \
	ClassDB::bind_method(D_METHOD("set_ref_"#var_name, #var_name), &class_name::set_ref_##var_name);\
	ClassDB::bind_method(D_METHOD("get_ref_"#var_name), &class_name::get_ref_##var_name);\
	ClassDB::bind_method(D_METHOD("get_"#var_name), &class_name::get_##var_name);\
	ClassDB::add_property(#class_name, PropertyInfo(Variant::NODE_PATH, #var_name, PROPERTY_HINT_NODE_PATH_VALID_TYPES, #type), "set_ref_"#var_name, "get_ref_"#var_name);

#define BIND_PROP(class_name, type, var_name ) \
	ClassDB::bind_method(D_METHOD("set_"#var_name, #var_name), &class_name::set_##var_name);\
	ClassDB::bind_method(D_METHOD("get_"#var_name), &class_name::get_##var_name);

#define ADD_SIMPLE_PROP(class_name, type, var_name ) \
	ClassDB::bind_method(D_METHOD("set_"#var_name, #var_name), &class_name::set_##var_name);\
	ClassDB::bind_method(D_METHOD("get_"#var_name), &class_name::get_##var_name);\
	ClassDB::add_property(#class_name, PropertyInfo(Variant::type, #var_name), "set_"#var_name, "get_"#var_name);

#define ADD_OBJECT_PROP(class_name, type, var_name ) \
	ClassDB::bind_method(D_METHOD("set_"#var_name, #var_name), &class_name::set_##var_name);\
	ClassDB::bind_method(D_METHOD("get_"#var_name), &class_name::get_##var_name);\
	ClassDB::add_property(#class_name, PropertyInfo(Variant::OBJECT, #var_name, PROPERTY_HINT_RESOURCE_TYPE, #type), "set_"#var_name, "get_"#var_name);

#define ADD_ARRAY_OBJECT_PROP(class_name, type, var_name ) \
	ClassDB::bind_method(D_METHOD("set_"#var_name, #var_name), &class_name::set_##var_name);\
	ClassDB::bind_method(D_METHOD("get_"#var_name), &class_name::get_##var_name);\
	ClassDB::add_property(#class_name, PropertyInfo(Variant::ARRAY, #var_name, PROPERTY_HINT_RESOURCE_TYPE, #type), "set_"#var_name, "get_"#var_name);

#define ADD_ARRAY_PROP(class_name, var_name ) \
	ClassDB::bind_method(D_METHOD("set_"#var_name, #var_name), &class_name::set_##var_name);\
	ClassDB::bind_method(D_METHOD("get_"#var_name), &class_name::get_##var_name);\
	ClassDB::add_property(#class_name, PropertyInfo(Variant::ARRAY, #var_name), "set_"#var_name, "get_"#var_name);

#define SET_GET_PARAM(type, name) \
public:\
	void set_##name(type param) { name = param;}\
	type get_##name() const { return name; }\
	type const &get_ref_##name() const { return name; }\
private:\
	type name;

#define SET_GET_PARAM_DEF(type, name, def) \
public:\
	void set_##name(type param) { name = param;}\
	type get_##name() const { return name; }\
	type const &get_ref_##name() const { return name; }\
private:\
	type name = def;


#define SET_GET_PARAM_DEF_MUTEX(type, name, def, mutex_name) \
public:\
	void set_##name(type param) { std::lock_guard<std::mutex> lock(mutex_name); name = param;}\
	type get_##name() const { std::lock_guard<std::mutex> lock(mutex_name); return name; }\
private:\
	type name = def;
