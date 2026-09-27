#pragma once

#include "scene/2d/node_2d.h"

#include "godot_tools.h"

namespace godot {

class DelaunayTriangulationApiNode;

class DelaunayTriangulationDisplayNode : public Node2D {
	GDCLASS(DelaunayTriangulationDisplayNode, Node2D)

	SET_GET_NODE_PATH(DelaunayTriangulationApiNode, triangulation_api);

	static void _bind_methods();

protected:
	void _notification(int p_notification);

private:
	void _draw();
};

} // namespace godot
