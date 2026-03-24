#pragma once

#include "utils.hpp"

class VertexArray {
public:
	VertexArray();

	void bind() const noexcept;
	void unbind() const noexcept;

private:
	GlVertexArray m_vao;

};

