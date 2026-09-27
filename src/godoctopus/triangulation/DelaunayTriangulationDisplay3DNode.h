#pragma once

#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/immediate_mesh.h"
#include "scene/resources/material.h"

#include "godot_tools.h"

namespace godot {

class DelaunayTriangulationApiNode;

class DelaunayTriangulationDisplay3DNode : public MeshInstance3D {
	GDCLASS(DelaunayTriangulationDisplay3DNode, MeshInstance3D)

	SET_GET_NODE_PATH(DelaunayTriangulationApiNode, triangulation_api);

public:
	static void _bind_methods();

protected:
	void _notification(int p_notification);

private:
	struct RenderTriangle {
		Vector2 points[3];
		bool hole;
	};

	void _process(double delta);
	void refresh_mesh();

	Ref<ImmediateMesh> _immediate_mesh;
	Ref<StandardMaterial3D> _normal_fill_material;
	Ref<StandardMaterial3D> _hole_fill_material;
	Ref<StandardMaterial3D> _normal_outline_material;
	Ref<StandardMaterial3D> _hole_outline_material;
};

} // namespace godot
