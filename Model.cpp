#include "Model.h"
#include "Mesh.h"
#include <iostream>
#include "stb_image.h"
#include <assimp/Importer.hpp> // Assimp::Importer
#include <assimp/postprocess.h> // флаги импорта

Model::Model(const std::string& path)
{
    loadModel(path);
}

void Model::Draw(Shader& shader, bool drawModel)
{
    if (!drawModel) return; // теперь drawModel есть
    for (auto& mesh : meshes)
        mesh.Draw();
}

void Model::loadModel(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of("/\\"));
    processNode(scene->mRootNode, scene);

    if (meshes.empty())
        std::cout << "Model has 0 meshes!" << std::endl;
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // Обрабатываем все меши в узле
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // Рекурсивно обрабатываем дочерние узлы
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Вершины
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.Position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };

        vertex.Normal = mesh->HasNormals() ?
            glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) :
            glm::vec3(0.0f);

        if (mesh->mTextureCoords[0]) // Проверка наличия UV
        {
            vertex.TexCoords = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f);
        }

        vertices.push_back(vertex);
    }

    // Индексы
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // Создаем Mesh (VAO/VBO/EBO)
    return createMesh(vertices, indices);
}
