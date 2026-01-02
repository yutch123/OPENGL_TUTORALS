#define GLM_ENABLE_EXPERIMENTAL // разрешаем использовать экспериментальные расширени€ GLM (на вс€кий случай, если будут нужны)

#include "Arcball.h"

#include <glm/gtx/quaternion.hpp> // дл€ работы с кватернионами
#include <algorithm>              // дл€ std::clamp
#include <cmath>                  // дл€ std::acos, std::sqrt

//  онструктор Arcball Ч инициализаци€ ширины и высоты окна
Arcball::Arcball(int width, int height)
    : m_width(width), m_height(height)
{
}

// ќбработка изменени€ размера окна
void Arcball::onResize(int width, int height)
{
    m_width = width;
    m_height = height;
}

// ќбработка нажати€ кнопки мыши
void Arcball::onMouseButton(int button, int action, double x, double y)
{
    if (button != 0) // реагируем только на левую кнопку мыши (GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action == 1) // GLFW_PRESS
    {
        m_dragging = true; // начинаем "т€нуть" мышью
        m_lastPos = screenToArcball((float)x, (float)y); // сохран€ем начальную позицию на виртуальном шаре
    }
    else if (action == 0) // GLFW_RELEASE
    {
        m_dragging = false; // закончили перетаскивание
    }
}

// ќбработка движени€ курсора мыши
void Arcball::onCursorMove(double x, double y)
{
    if (!m_dragging)
        return; // если мышь не зажата, вращение не происходит

    glm::vec3 currPos = screenToArcball((float)x, (float)y); // текуща€ позици€ на виртуальном шаре

    // ¬ычисл€ем угол между последней и текущей позицией
    float dot = glm::clamp(glm::dot(m_lastPos, currPos), -1.0f, 1.0f);
    float angle = std::acos(dot); // угол в радианах

    // ќсь вращени€ = векторное произведение последней и текущей позиции
    glm::vec3 axis = glm::cross(m_lastPos, currPos);

    if (glm::length(axis) > 0.0001f) // если ось не нулева€
    {
        axis = glm::normalize(axis); // нормализуем ось
        glm::quat delta = glm::angleAxis(angle, axis); // создаем кватернион вращени€
        m_rotation = delta * m_rotation; // обновл€ем текущее вращение
    }

    m_lastPos = currPos; // сохран€ем текущую позицию дл€ следующего кадра
}

// ѕолучение матрицы вращени€ дл€ OpenGL
glm::mat4 Arcball::getRotationMatrix() const
{
    return glm::toMat4(m_rotation); // преобразуем кватернион в матрицу 4x4
}

// —брос вращени€
void Arcball::reset()
{
    m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // единичный кватернион Ч отсутствие вращени€
}

// ѕреобразование координат экрана в координаты на виртуальном шаре
glm::vec3 Arcball::screenToArcball(float x, float y) const
{
    // Ќормализаци€ координат в диапазон [-1, 1]
    float nx = (2.0f * x - m_width) / m_width;
    float ny = (m_height - 2.0f * y) / m_height;

    float length2 = nx * nx + ny * ny;

    if (length2 <= 1.0f) // внутри круга
        return glm::vec3(nx, ny, std::sqrt(1.0f - length2)); // на сфере
    else // снаружи круга
        return glm::normalize(glm::vec3(nx, ny, 0.0f)); // на границе сферы
}
