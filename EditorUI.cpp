#include "EditorUI.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

bool primitivesCollapsed = true;

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
    drawModelWindow(); // новое окно загрузки модели

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

    // Проверяем collapsed state
    primitivesCollapsed = ImGui::IsWindowCollapsed();

    ImGui::End(); // закрываем окно
}

void EditorUI::drawModelWindow()
{
    float offsetX = 10.0f;
    float offsetY = 30.0f; // базовая позиция сверху

    if (!primitivesCollapsed)
    {
        // Если первое окно открыто — под ним
        offsetY += 3 * 40 + 3 * 5 + 10.0f;
        // 3 кнопки * 40px + 3 промежутка по 5px + 10px отступ
    }
    else
    {
        // Если первое окно свернуто — просто небольшой отступ сверху
        offsetY += 10.0f;
    }

    // Применяем позицию для второго окна
    ImGui::SetNextWindowPos(ImVec2(offsetX, offsetY), ImGuiCond_Always);

    ImGui::Begin(
        "Model Loader",
        nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize
    );

    if (ImGui::Button("3D_Model", ImVec2(120, 40)))
        loadModelRequested = true;

    ImGui::End();
}

// Метод для получения запроса на примитив из UI
// После вызова сбрасывает запрос на None
PrimitiveType EditorUI::consumePrimitiveRequest()
{
    PrimitiveType result = requestedPrimitive;
    requestedPrimitive = PrimitiveType::None;
    return result;
}