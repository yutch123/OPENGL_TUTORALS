#pragma once
#include <glad/glad.h>
#include <vector>

class Sphere
{
public:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    unsigned int indexCount = 0;

    Sphere(float radius = 1.0f, unsigned int sectorCount = 36, unsigned int stackCount = 18);
    ~Sphere();

    void draw() const;
    void destroy();
};
