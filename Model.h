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

		void setRotationMatrix(const glm::mat4& rot);

		glm::vec3 getSize() const { return maxBounds - minBounds; }
		void setScale(float s) { scale = s; }
		float getScale() const { return scale; }  

		void selectMesh(int index);

		void drawForPicking(Shader& shader);

		void setPickingEnabled(bool enabled) { pickingEnabled = enabled; }

		size_t getMeshCount() const;

	private:
		// model data
		bool pickingEnabled = true; // по умолчанию включено

		std::vector<Mesh> meshes;

		int selectedMeshIndex = -1;

		std::vector<glm::vec3> meshColors;
		std::vector<bool> meshVisible; // какие меши видимы
		std::string directory;
		std::vector<Texture> textures_loaded;

		float scale = 1.0f;
		glm::vec3 position = glm::vec3(0.0f);   // позиция модели
		glm::mat4 rotationMatrix = glm::mat4(1.0f); // вращение (Arcball или любое другое)

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


