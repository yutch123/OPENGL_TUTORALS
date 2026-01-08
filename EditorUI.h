#pragma once
#include "imgui.h"

// Класс для пользовательского интерфейса (UI) редактора
class EditorUI
{

public:
	// Начало нового кадра ImGui — нужно вызывать один раз в цикле рендеринга
	void beginFrame();
	// Отрисовка UI и вывод на экран
	void render();

	bool loadModelRequested = false;

private:
	void drawModelWindow(); // новое окно с кнопкой для загрузки модели
};
