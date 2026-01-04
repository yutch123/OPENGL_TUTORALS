#define GLM_ENABLE_EXPERIMENTAL
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Arcball
{
public:
	// Конструктор: принимает ширину и высоту окна для нормализации координат мыши
	Arcball(int width, int height);

	// Методы обработки ввода
	void onMouseButton(int button, int action, double x, double y); // нажатие/отпускание кнопки мыши
	void onCursorMove(double x, double y);  // движение мыши
	void onResize(int width, int height);  // изменение размеров окна

	// Методы получения результата
	glm::mat4 getRotationMatrix() const; // возвращает текущую матрицу поворота
	void reset();  // сброс вращения к исходному состоянию

private:
	// Преобразует экранные координаты курсора в координаты на виртуальной сфере Arcball
	glm::vec3 screenToArcball(float x, float y) const;

private:
	int m_width; // ширина окна
	int m_height; // высота окна

	bool m_dragging = false; // флаг, что пользователь тянет мышь

	glm::vec3 m_lastPos{}; // последняя позиция курсора на сфере
	glm::quat m_rotation{ 1.0f, 0.0f, 0.0f, 0.0f }; // текущее вращение в виде кватерниона
};
