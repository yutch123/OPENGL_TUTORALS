#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cfloat>  // для FLT_MAX
#include "Mesh.h"      // для Mesh и Texture
#include "Shader.h"    // для Shader
#include <assimp/scene.h>  // для aiNode, aiScene, aiMesh, aiMaterial, aiTextureType

class Model
{
	public:
		Model(const std::string& path)
		{
			loadModel(path);
			calculateBoundingBox(); // вычисляем размеры сразу после загрузк
		}

		void Draw(Shader& shader);

		glm::vec3 getSize() const { return maxBounds - minBounds; }
		void setScale(float s) { scale = s; }
		float getScale() const { return scale; }  

	private:
		// model data
		std::vector<Mesh> meshes;
		std::vector<glm::vec3> meshColors;
		std::string directory;
		std::vector<Texture> textures_loaded;

		float scale = 1.0f;

		glm::vec3 minBounds = glm::vec3(FLT_MAX);
		glm::vec3 maxBounds = glm::vec3(-FLT_MAX);

		void loadModel(const std::string& path);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);

		void calculateBoundingBox()
    {
        for (const auto& mesh : meshes)
        {
            for (const auto& v : mesh.vertices)
            {
                minBounds = glm::min(minBounds, v.Position);
                maxBounds = glm::max(maxBounds, v.Position);
            }
        }
    }

};


