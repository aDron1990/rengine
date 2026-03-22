#include "Mesh.hpp"

#include "tiny_obj_loader.h"

Mesh::Mesh(const std::string& objPath) {
	loadObj(objPath);
}

void Mesh::draw() const noexcept {
	m_vao.bind();
	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
}

void Mesh::loadObj(const std::string& objPath) {
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	tinyobj::attrib_t attrib;

	std::string warn;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objPath.c_str())) {
		throw std::runtime_error(err);
	}

	m_vertices.reserve(attrib.vertices.size() + attrib.texcoords.size());
	for (const auto& shape : shapes) {
		for (const auto& idx : shape.mesh.indices) {

			Vertex vertex{};

			vertex.position.x = attrib.vertices[3 * idx.vertex_index + 0];
			vertex.position.y = attrib.vertices[3 * idx.vertex_index + 1];
			vertex.position.z = attrib.vertices[3 * idx.vertex_index + 2];

			vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
			vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
			vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];

			if (idx.texcoord_index >= 0) {
				vertex.tex_coords.x = attrib.texcoords[2 * idx.texcoord_index + 0];
				vertex.tex_coords.y = attrib.texcoords[2 * idx.texcoord_index + 1];
			}
			else {
				vertex.tex_coords.x = 0.0f;
				vertex.tex_coords.y = 0.0f;
			}

			m_vertices.push_back(vertex);
		}
	}

	m_vao.bind();
	m_vbo = VertexBuffer{ m_vertices.data(), m_vertices.size() * sizeof(Vertex) };

	// position attribute 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute 
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}
