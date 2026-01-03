#pragma once
#include <glad/glad.h>

struct Mesh
{
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int vertexCount = 0;

	void draw() const;
	void destroy();
};

// объявление функции 
Mesh createMesh(const float* vertices, size_t vertexFloatCount);
