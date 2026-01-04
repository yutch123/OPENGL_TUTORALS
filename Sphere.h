#pragma once
#include <glad/glad.h>

class Sphere
{
public:
    // OpenGL объекты
    unsigned int VAO = 0; // Vertex Array Object Ч хранит состо€ние вершинных атрибутов
    unsigned int VBO = 0; // Vertex Buffer Object Ч хранит вершины сферы в виде массива
    unsigned int EBO = 0; // Element Buffer Object Ч хранит индексы дл€ отрисовки с помощью glDrawElements
    unsigned int indexCount = 0; //  оличество индексов, используемых дл€ отрисовки сферы

    //  онструктор сферы
    // radius Ч радиус сферы
    // sectorCount Ч количество "секторов" (по горизонтали)
    // stackCount Ч количество "стеков" (по вертикали)
    Sphere(float radius = 1.0f, unsigned int sectorCount = 36, unsigned int stackCount = 18);
    // ƒеструктор сферы
    ~Sphere();

    // ћетод дл€ отрисовки сферы
    void draw() const;

    // ћетод дл€ удалени€ ресурсов OpenGL, св€занных с этой сферой
    void destroy();
};
