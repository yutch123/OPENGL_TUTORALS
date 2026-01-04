#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// Структура для одной вершины
struct Vertex
{
	glm::vec3 Position; // координаты вершины (x, y, z)
	glm::vec3 Normal; // нормаль вершины (для освещения)
	glm::vec2 TexCoords; // текстурные координаты (u, v)

	Vertex() {} // пустой конструктор нужен для std::vector, чтобы можно было создавать массив вершин

	Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex)
		: Position(pos), Normal(norm), TexCoords(tex) {} // конструктор с инициализацией всех полей
};

// Структура для меша (набор вершин и GPU объекты)
struct Mesh
{
	unsigned int VAO = 0; // Vertex Array Object — хранит привязки VBO и атрибутов
	unsigned int VBO = 0; // Vertex Buffer Object — массив вершин в видеопамяти GPU
	unsigned int EBO = 0; // Element Buffer Object — массив индексов (для glDrawElements)

	unsigned int vertexCount = 0; // количество вершин (для glDrawArrays)
	unsigned int indexCount; // количество индексов (для glDrawElements)

	void draw() const; // метод отрисовки меша
	void destroy(); // метод очистки GPU-ресурсов
};

// Функция создания меша из вектора вершин
// Возвращает структуру Mesh с VAO, VBO и количеством вершин
Mesh createMesh(const std::vector<Vertex>& vertices);
