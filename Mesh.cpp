#include "Mesh.h"

// Функция создания меша из вектора вершин
Mesh createMesh(const std::vector<Vertex>& vertices) {
    
    Mesh mesh;
    mesh.vertexCount = vertices.size(); // сохраняем количество вершин

    // Генерация объекта Vertex Array Object (VAO) и Vertex Buffer Object (VBO)
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);

    // Привязываем VAO — все последующие настройки буферов будут связаны с этим VAO
    glBindVertexArray(mesh.VAO);

    // Привязываем VBO как массив вершин
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    // Загружаем данные вершин в GPU
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Настройка атрибутов вершин для шейдера
    // Атрибут 0 — позиция (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Атрибут 1 — нормали (x, y, z)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    
    // Атрибут 2 — текстурные координаты (u, v)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    // Отвязываем VAO, чтобы случайно не изменить его настройки
    glBindVertexArray(0);

    return mesh;

}

// Метод класса Mesh для отрисовки
void Mesh::draw() const
{
    glBindVertexArray(VAO); // привязываем VAO
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);  // рисуем все вершины как треугольники
    glBindVertexArray(0); // отвязываем VAO
}

// Метод очистки ресурсов GPU
void Mesh::destroy()
{
    if (VBO) glDeleteBuffers(1, &VBO); // удаляем VBO
    if (VAO) glDeleteVertexArrays(1, &VAO); // удаляем VAO

    // Сбрасываем значения, чтобы избежать повторного удаления
    VBO = 0;
    VAO = 0;
    vertexCount = 0;
}
