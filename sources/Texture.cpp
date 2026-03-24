#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const std::filesystem::path& filePath) {
	stbi_set_flip_vertically_on_load(true);
	int width, height, channels;
	auto path = filePath.string();
	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
	if (!data)
		throw std::runtime_error{ "Failed to load texture " + filePath.string() };
	
	GLuint texture;
	glGenTextures(1, &texture);
	m_texture.reset(texture);

	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

void Texture::bind() const noexcept {
	glBindTexture(GL_TEXTURE_2D, m_texture.get());
}

void Texture::unbind() const noexcept {
	glBindTexture(GL_TEXTURE_2D, 0);
}
