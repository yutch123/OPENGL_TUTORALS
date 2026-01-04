#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;

	Vertex() {} // нужен пустой конструктор для std::vector

	Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex)
		: Position(pos), Normal(norm), TexCoords(tex) {}
};

struct Mesh
{
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;

	unsigned int vertexCount = 0;
	unsigned int indexCount;

	void draw() const;
	void destroy();
};

// объявление функции 
Mesh createMesh(const std::vector<Vertex>& vertices);
