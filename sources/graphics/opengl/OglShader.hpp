#pragma once

#include "../GlHandle.hpp"

#include <GL/glew.h>

#include <filesystem>
#include <string>

class OglShader {
public:
    OglShader(const std::filesystem::path vertexFilePath,
        const std::filesystem::path fragmentFilePath);

    void use() const noexcept;

    template <typename T>
    void setUniform(const T& value, const std::string& name) noexcept;

private:
    static GlShader compileShader(const std::string& code, GLenum shaderType);
    void linkProgram(const GlShader& vertex, const GlShader& fragment);

private:
    GlProgram m_program;
};
