#define GLM_ENABLE_EXPERIMENTAL
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Arcball
{
public:
	Arcball(int width, int height);

	// input
	void onMouseButton(int button, int action, double x, double y);
	void onCursorMove(double x, double y);
	void onResize(int width, int height);

	// result
	glm::mat4 getRotationMatrix() const;
	void reset();

private:
	glm::vec3 screenToArcball(float x, float y) const;

private:
	int m_width;
	int m_height;

	bool m_dragging = false;

	glm::vec3 m_lastPos{};
	glm::quat m_rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

};
