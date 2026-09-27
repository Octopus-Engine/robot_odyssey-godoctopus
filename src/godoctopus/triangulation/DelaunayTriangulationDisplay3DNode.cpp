#include "DelaunayTriangulationDisplay3DNode.h"

#include "core/object/class_db.h"
#include "DelaunayTriangulationApiNode.h"

namespace godot {

void DelaunayTriangulationDisplay3DNode::_process(double) {
	refresh_mesh();
}

void DelaunayTriangulationDisplay3DNode::refresh_mesh() {
	if (!_triangulation_api || _immediate_mesh.is_null()) {
		return;
	}

	std::vector<RenderTriangle> triangles;
	for (DelaunayTriangulationApiNode::RenderTriangle const &source : _triangulation_api->get_render_triangles()) {
		RenderTriangle triangle;
		triangle.hole = source.hole;
		for (size_t i = 0; i < 3; ++i) {
			triangle.points[i] = source.points[i];
		}
		triangles.push_back(triangle);
	}

	_immediate_mesh->clear_surfaces();
	auto add_surface = [this, &triangles](bool hole, Mesh::PrimitiveType primitive, Ref<StandardMaterial3D> const &material) {
		bool has_geometry = false;
		for (RenderTriangle const &tri : triangles) {
			if (tri.hole == hole) {
				has_geometry = true;
				break;
			}
		}
		if (!has_geometry) {
			return;
		}

		_immediate_mesh->surface_begin(primitive, material);
		for (RenderTriangle const &tri : triangles) {
			if (tri.hole != hole) {
				continue;
			}
			for (size_t i = 0; i < 3; ++i) {
				size_t const next = (i + 1) % 3;
				real_t const y0 = primitive == Mesh::PRIMITIVE_LINES ? static_cast<real_t>(0.001) : static_cast<real_t>(0);
				real_t const y1 = y0;
				Vector3 const p0(static_cast<real_t>(tri.points[i].x), y0, static_cast<real_t>(tri.points[i].y));
				Vector3 const p1(static_cast<real_t>(tri.points[next].x), y1, static_cast<real_t>(tri.points[next].y));
				if (primitive == Mesh::PRIMITIVE_TRIANGLES) {
					_immediate_mesh->surface_add_vertex(p0);
				} else {
					_immediate_mesh->surface_add_vertex(p0);
					_immediate_mesh->surface_add_vertex(p1);
				}
			}
		}
		_immediate_mesh->surface_end();
	};

	add_surface(false, Mesh::PRIMITIVE_TRIANGLES, _normal_fill_material);
	add_surface(true, Mesh::PRIMITIVE_TRIANGLES, _hole_fill_material);
	add_surface(false, Mesh::PRIMITIVE_LINES, _normal_outline_material);
	add_surface(true, Mesh::PRIMITIVE_LINES, _hole_outline_material);
}

void DelaunayTriangulationDisplay3DNode::_bind_methods() {
	BIND_NODE_PATH(DelaunayTriangulationDisplay3DNode, DelaunayTriangulationApiNode, triangulation_api);
}

void DelaunayTriangulationDisplay3DNode::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			INIT_NODE_PATH(DelaunayTriangulationApiNode, triangulation_api);
			if (!_triangulation_api) {
				ERR_PRINT("DelaunayTriangulationDisplay3DNode requires a DelaunayTriangulationApiNode path.");
				set_process(false);
				break;
			}

			_immediate_mesh.instantiate();
			_normal_fill_material.instantiate();
			_hole_fill_material.instantiate();
			_normal_outline_material.instantiate();
			_hole_outline_material.instantiate();

			_normal_fill_material->set_albedo(Color(0.0f, 1.0f, 0.0f, 0.35f));
			_hole_fill_material->set_albedo(Color(1.0f, 0.0f, 0.0f, 0.35f));
			_normal_outline_material->set_albedo(Color(0.0f, 1.0f, 0.0f, 1.0f));
			_hole_outline_material->set_albedo(Color(1.0f, 0.0f, 0.0f, 1.0f));
			for (Ref<StandardMaterial3D> material : {
					 _normal_fill_material,
					 _hole_fill_material,
					 _normal_outline_material,
					 _hole_outline_material }) {
				material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
				material->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
			}
			_normal_fill_material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
			_hole_fill_material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);

			set_mesh(_immediate_mesh);
			refresh_mesh();
			set_process(true);
		} break;
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
	}
}

} // namespace godot
