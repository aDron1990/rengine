#include "OglShader.hpp"
#include "utils/utils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

OglShader::OglShader(const std::filesystem::path vertexFilePath,
    const std::filesystem::path fragmentFilePath)
    : m_program { glCreateProgram() }
{
    auto vertexCode = loadFile(vertexFilePath);
    auto fragmentCode = loadFile(fragmentFilePath);

    auto vertex = compileShader(vertexCode, GL_VERTEX_SHADER);
    auto fragment = compileShader(fragmentCode, GL_FRAGMENT_SHADER);

    linkProgram(vertex, fragment);
}

GlShader OglShader::compileShader(const std::string& code, GLenum shaderType)
{
    int success;
    char infoLog[512];
    auto* codePtr = code.c_str();

    GLuint shader;
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &codePtr, nullptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        glDeleteShader(shader);
        throw std::runtime_error { infoLog };
    }

    return GlShader { shader };
}

void OglShader::linkProgram(const GlShader& vertex, const GlShader& fragment)
{
    int success;
    char infoLog[512];

    glAttachShader(m_program.get(), vertex.get());
    glAttachShader(m_program.get(), fragment.get());
    glLinkProgram(m_program.get());
    glGetProgramiv(m_program.get(), GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_program.get(), 512, NULL, infoLog);
        throw std::runtime_error { infoLog };
    }
}

void OglShader::use() const noexcept { glUseProgram(m_program.get()); }

template <>
void OglShader::setUniform<glm::mat4>(const glm::mat4& value,
    const std::string& name) noexcept
{
    auto ptr = glGetUniformLocation(m_program.get(), name.c_str());
    glUniformMatrix4fv(ptr, 1, GL_FALSE, glm::value_ptr(value));
}

template <>
void OglShader::setUniform<glm::vec3>(const glm::vec3& value,
    const std::string& name) noexcept
{
    auto ptr = glGetUniformLocation(m_program.get(), name.c_str());
    glUniform3fv(ptr, 1, glm::value_ptr(value));
}

template <>
void OglShader::setUniform<float>(const float& value,
    const std::string& name) noexcept
{
    auto ptr = glGetUniformLocation(m_program.get(), name.c_str());
    glUniform1f(ptr, value);
}

template <>
void OglShader::setUniform<int>(const int& value,
    const std::string& name) noexcept
{
    auto ptr = glGetUniformLocation(m_program.get(), name.c_str());
    glUniform1i(ptr, value);
}