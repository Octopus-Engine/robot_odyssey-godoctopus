#pragma once

#include "scene/3d/node_3d.h"

namespace godot {

class VatLibrary : public Node3D {
	GDCLASS(VatLibrary, Node3D)

public:
	~VatLibrary() {}

protected:
	void _notification(int p_notification);
};

}
