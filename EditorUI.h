#pragma once
#include "imgui.h"

enum class PrimitiveType
{
	None,
	Cube,
	Pyramid,
	Sphere
};

class EditorUI
{
public:
	void beginFrame();
	void render();

	PrimitiveType consumePrimitiveRequest();

private:
	void drawPrimitivesWindow();

private:
	PrimitiveType requestedPrimitive = PrimitiveType::None;
};
