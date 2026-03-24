#pragma once

#include "utils.hpp"
#include "Buffer.hpp"
#include "VertexArray.hpp"

class Mesh {
public:
	Mesh(const std::string& objPath);
	
	void draw() const noexcept;

private:
	void loadObj(const std::string& objPath);

private:
	std::vector<Vertex> m_vertices;
	VertexBuffer m_vbo;
	VertexArray m_vao;

};

