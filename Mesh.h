#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>

#define MAX_BONE_INFLUENCE 4

class Shader;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;

	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh {
	public:
		// mesh data
		std::vector<Vertex>			vertices;
		std::vector<unsigned int>	indices;
		std::vector<Texture>		textures;

		Mesh(
			std::vector<Vertex> vertices,
			std::vector<unsigned int> indices,
			std::vector<Texture> textures
		);
		
		void Draw(Shader& shader);
		void DrawForPicking(Shader& shader, const glm::vec3& color);

		int pickingID;

	private:
		// render data
		unsigned int VAO = 0;
		unsigned int VBO = 0;
		unsigned int EBO = 0;

		void setupMesh();
};

