#pragma once
#include "imgui.h"

// Перечисление для типов примитивов, которые пользователь может выбрать
enum class PrimitiveType
{
	None, // ничего не выбрано
	Cube, // куб
	Pyramid, // пирамида
	Sphere // сфера
};

// Класс для пользовательского интерфейса (UI) редактора
class EditorUI
{
public:
	// Начало нового кадра ImGui — нужно вызывать один раз в цикле рендеринга
	void beginFrame();
	// Отрисовка UI и вывод на экран
	void render();

	// Метод для получения запроса пользователя на выбор примитива
	// После вызова сбрасывает запрос на None
	PrimitiveType consumePrimitiveRequest();

private:
	// Создает окно с кнопками для выбора примитивов
	void drawPrimitivesWindow();

private:
	PrimitiveType requestedPrimitive = PrimitiveType::None; // хранит текущий запрос от UI
};
