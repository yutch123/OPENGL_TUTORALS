#include "Sphere.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "Mesh.h" // где определён Vertex

// Конструктор сферы: создает вершины, индексы и настраивает OpenGL объекты
Sphere::Sphere(float radius, unsigned int sectorCount, unsigned int stackCount)
{
    std::vector<Vertex> vertices; // Вершины сферы
    std::vector<unsigned int> indices; // Индексы для EBO (отрисовка через glDrawElements)

    float x, y, z, xy; // координаты вершин
    float nx, ny, nz, lengthInv = 1.0f / radius; // нормализованные координаты нормалей
    float s, t; // текстурные координаты

    // Шаг углов для секторов и стэков
    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    // Генерация вершин
    for (unsigned int i = 0; i <= stackCount; ++i)
    {
        stackAngle = glm::half_pi<float>() - i * stackStep; // от +π/2 до -π/2
        xy = radius * cosf(stackAngle); // проекция на XY-плоскость
        z = radius * sinf(stackAngle); // координата Z

        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep; // угол вокруг Z

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            // нормализуем вектор нормали
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            // вычисляем текстурные координаты
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            // создаем вершину и добавляем в массив
            Vertex vertex;
            vertex.Position = glm::vec3(x, y, z);
            vertex.Normal = glm::normalize(glm::vec3(nx, ny, nz));
            vertex.TexCoords = glm::vec2(s, t);
            vertices.push_back(vertex);
        }
    }

    // Генерация индексов для треугольников
    for (unsigned int i = 0; i < stackCount; ++i)
    {
        unsigned int k1 = i * (sectorCount + 1); // начало текущего стэка
        unsigned int k2 = k1 + sectorCount + 1; // начало следующего стэка

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0) // верхняя граница стэка
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) // нижняя граница стэка
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    indexCount = static_cast<unsigned int>(indices.size()); // сохраняем количество индексов

    // Создаем VAO, VBO и EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Загружаем вершины в VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Загружаем индексы в EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Настройка атрибутов вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // отвязываем VAO
}

// Деструктор освобождает ресурсы
Sphere::~Sphere()
{
    destroy();
}

// Метод отрисовки сферы
void Sphere::draw() const
{
    glBindVertexArray(VAO);
    // Используем индексы для отрисовки
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Освобождение GPU-ресурсов
void Sphere::destroy()
{
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    VBO = EBO = VAO = 0;
    indexCount = 0;
}
