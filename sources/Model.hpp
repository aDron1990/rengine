#pragma once

#include "graphics/Mesh.hpp"
#include <assimp/scene.h>
#include <vector>

class Model {
public:
    Model(const std::string& path);
    BoundingBox getAABB() const noexcept;
    BoundingBox getLocalAABB() const noexcept;
    const std::vector<Mesh>& getMeshes() const noexcept;
    std::vector<Mesh>& Model::getMeshes() noexcept;

private:
    void load(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    void updateAABB(const Mesh& mesh) noexcept;

private:
    std::vector<Mesh> m_meshes;
    BoundingBox m_aabb;
    BoundingBox m_localAABB;
    glm::vec3 m_offset { 0.0f };
};