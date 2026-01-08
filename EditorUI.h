#pragma once
#include "imgui.h"
#include <Model.h>

// Класс для пользовательского интерфейса (UI) редактора
class EditorUI
{

public:
	// Начало нового кадра ImGui — нужно вызывать один раз в цикле рендеринга
	void beginFrame();
	// Отрисовка UI и вывод на экран
	void render(Model* model);

	bool loadModelRequested = false;

private:
	void drawModelWindow(); // новое окно с кнопкой для загрузки модели
};
