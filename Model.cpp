#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <stb_image.h>

void Model::Draw(Shader & shader)
{
    // Активируем шейдер
    shader.use();

    // 2. Создаём матрицу модели
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, position);
    modelMat = modelMat * rotationMatrix;
    modelMat = glm::scale(modelMat, glm::vec3(scale)); // применяем scale


    // 3. Передаём матрицу шейдеру
    shader.setMat4("model", modelMat);

    for (size_t i = 0; i < meshes.size(); ++i)
    {

        if (!meshVisible[i])
            continue; // этот меш скрыт, пропускаем

        // проверяем, есть ли цвет для текущего меша
        if (i >= meshColors.size())
            meshColors.push_back(glm::vec3(1.0f)); // белый по умолчанию

        // если меш не имеет текстур, используем цвет
        if (meshes[i].textures.empty())
        {
            shader.setVec3("objectColor", meshColors[i]);
        }

        meshes[i].Draw(shader);
    }

}

void Model::loadModel(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of("/\\"));
    processNode(scene->mRootNode, scene);

    // Инициализируем массив видимости после загрузки всех мешей
    meshVisible.resize(meshes.size(), true);
}

size_t Model::getMeshCount() const {
    return meshes.size();
}

void Model::drawForPicking(Shader& shader)
{
    if (!pickingEnabled)
        return; // защита от случайного выбора: ничего не делаем

    shader.use();

    for (size_t i = 0; i < meshes.size(); ++i)
    {
        if (!meshVisible[i])
            continue; // пропускаем скрытые меши

        unsigned int id = i + 1; // ID для picking
        glm::vec3 pickColor(
            (id & 0xFF) / 255.0f,
            ((id >> 8) & 0xFF) / 255.0f,
            ((id >> 16) & 0xFF) / 255.0f
        );

        shader.setVec3("pickingColor", pickColor);
        meshes[i].DrawForPicking(shader, pickColor);
    }
}

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
        {
            std::cerr << "Unknown number of channels: " << nrComponents << std::endl;
            stbi_image_free(data);
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // настройки фильтрации и обёртки
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];

        // создаём Mesh один раз
        Mesh newMesh = processMesh(aiMesh, scene);
        meshes.push_back(newMesh);

        // сохраняем цвет для этого меша
        glm::vec3 meshColor(1.0f); // по умолчанию белый
        if (aiMesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
            aiColor3D color(1.0f, 1.0f, 1.0f);
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
                meshColor = glm::vec3(color.r, color.g, color.b);
        }
        meshColors.push_back(meshColor);
    }

    // Рекурсивная обработка дочерних узлов
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Вершины
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

        if (mesh->mTextureCoords[0])
            vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        else
            vertex.TexCoords = { 0.0f, 0.0f };

        vertices.push_back(vertex);
    }

    // Индексы
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // Материалы
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D color;

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (textures_loaded[j].path == str.C_Str())
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }

        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory, false);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // add to loaded textures
        }
    }

    return textures;
}

void Model::setRotationMatrix(const glm::mat4& rot)
{
    rotationMatrix = rot;
}

// Выбор меша

void Model::selectMesh(int index)
{
    if (index < 0 || index >= (int)meshes.size())
    {
        selectedMeshIndex = -1; // сброс выбора

        for (size_t i = 0; i < meshVisible.size(); ++i)
            meshVisible[i] = true;

        return;
    }

    // Выбор конкретного меша
    selectedMeshIndex = index;

    for (size_t i = 0; i < meshVisible.size(); ++i)
        meshVisible[i] = (i == index);

}

