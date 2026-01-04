#include "EditorUI.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

// Начало нового кадра ImGui — нужно вызывать каждый рендер-цикл
void EditorUI::beginFrame()
{
	ImGui_ImplOpenGL3_NewFrame(); // подготавливаем OpenGL3 бэкенд
	ImGui_ImplGlfw_NewFrame(); // подготавливаем GLFW бэкенд
	ImGui::NewFrame(); // создаем новый кадр ImGui
}

// Отрисовка UI и вывод на экран
void EditorUI::render()
{
	drawPrimitivesWindow(); // рисуем окно с выбором примитивов

	ImGui::Render(); // подготавливаем отрисовку
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // отрисовываем через OpenGL
}

// Создание окна с кнопками для выбора примитивов
void EditorUI::drawPrimitivesWindow()
{
    // Устанавливаем позицию окна один раз (при первом создании)
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);

    ImGui::Begin(
        "Primitives",
        nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize
    );

    ImVec2 buttonSize(120, 40); // размер кнопок

    // Кнопка "Cube"
    if (ImGui::Button("Cube", buttonSize))
        requestedPrimitive = PrimitiveType::Cube;
    // Кнопка "Pyramid"
    if (ImGui::Button("Pyramid", buttonSize))
        requestedPrimitive = PrimitiveType::Pyramid;
    // Кнопка "Sphere"
    if (ImGui::Button("Sphere", buttonSize))
        requestedPrimitive = PrimitiveType::Sphere;

    ImGui::End(); // закрываем окно
}

// Метод для получения запроса на примитив из UI
// После вызова сбрасывает запрос на None
PrimitiveType EditorUI::consumePrimitiveRequest()
{
    PrimitiveType result = requestedPrimitive;
    requestedPrimitive = PrimitiveType::None;
    return result;
}