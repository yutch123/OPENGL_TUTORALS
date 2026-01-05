#pragma once

#include <string>
#include <vector>
#include "Mesh.h"
#include "Shader.h"
#include <assimp/scene.h>
#include <assimp/mesh.h>


class Model
{
public:
	// Конструктор - создает модель из файла
	Model(const std::string& path);

	// Отрисовка модели
	void Draw(Shader& shader, bool drawModel = true);

private:
	std::vector<Mesh> meshes; // все меши модели
	std::string directory; // путь к папке с моделью

	// методы для загрузки модели
	void loadModel(const std::string& path);

	// Рекурсивная обработка узлов Assimp
	void processNode(aiNode* node, const aiScene* scene);

	// Преобразование aiMesh в наш Mesh
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};