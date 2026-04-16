#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

Model::Model(const std::string& path)
{
    load(path);
}

void Model::load(const std::string& path)
{
    Assimp::Importer importer;
    importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.f);

    const auto* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices | aiProcess_GlobalScale);

    if (!scene || !scene->mRootNode) {
        throw std::runtime_error(importer.GetErrorString());
    }

    processNode(scene->mRootNode, scene);

    glm::vec3 center = (m_aabb.min + m_aabb.max) * 0.5f;

    m_offset = center;

    m_localAABB.min = m_aabb.min - center;
    m_localAABB.max = m_aabb.max - center;
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned i = 0; i < node->mNumMeshes; i++) {
        aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh mesh = processMesh(ai_mesh, scene);
        updateAABB(mesh);
        m_meshes.push_back(std::move(mesh));
    }

    for (unsigned i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    Mesh result;

    result.vertices.reserve(mesh->mNumVertices);
    result.indices.reserve(mesh->mNumFaces * 3);

    BoundingBox aabb;
    aabb.min = { FLT_MAX, FLT_MAX, FLT_MAX };
    aabb.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (unsigned i = 0; i < mesh->mNumVertices; i++) {
        Vertex v { };

        v.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };

        aabb.min.x = std::min(aabb.min.x, v.position.x);
        aabb.min.y = std::min(aabb.min.y, v.position.y);
        aabb.min.z = std::min(aabb.min.z, v.position.z);

        aabb.max.x = std::max(aabb.max.x, v.position.x);
        aabb.max.y = std::max(aabb.max.y, v.position.y);
        aabb.max.z = std::max(aabb.max.z, v.position.z);

        if (mesh->HasNormals()) {
            v.normal = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            };
        }

        if (mesh->HasTextureCoords(0)) {
            v.tex_coords = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        } else {
            v.tex_coords = { 0.0f, 0.0f };
        }

        result.vertices.push_back(v);
    }

    for (unsigned i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; j++) {
            result.indices.push_back(face.mIndices[j]);
        }
    }

    result.aabb = aabb;

    return result;
}

void Model::updateAABB(const Mesh& mesh) noexcept
{
    for (const auto& v : mesh.vertices) {
        glm::vec3 p = v.position;

        m_aabb.min.x = std::min(m_aabb.min.x, p.x);
        m_aabb.min.y = std::min(m_aabb.min.y, p.y);
        m_aabb.min.z = std::min(m_aabb.min.z, p.z);

        m_aabb.max.x = std::max(m_aabb.max.x, p.x);
        m_aabb.max.y = std::max(m_aabb.max.y, p.y);
        m_aabb.max.z = std::max(m_aabb.max.z, p.z);
    }
}

BoundingBox Model::getAABB() const noexcept
{
    return m_aabb;
}

BoundingBox Model::getLocalAABB() const noexcept
{
    return m_localAABB;
}

const std::vector<Mesh>& Model::getMeshes() const noexcept
{
    return m_meshes;
}

std::vector<Mesh>& Model::getMeshes() noexcept
{
    return m_meshes;
}