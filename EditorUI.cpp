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
    drawModelWindow(); // новое окно загрузки модели

	ImGui::Render(); // подготавливаем отрисовку
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // отрисовываем через OpenGL
}

void EditorUI::drawModelWindow()
{
    // Применяем позицию для второго окна
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);

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