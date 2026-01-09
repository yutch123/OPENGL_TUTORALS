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
void EditorUI::render(Model* model)
{
    drawModelWindow(); // новое окно загрузки модели

    ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_FirstUseEver); // ширина 200, высота 80

    // Размер окна Debug
    ImVec2 windowSize(200, 80); // <-- объявляем сначала!

    // Позиция окна в левом нижнем углу
    ImVec2 windowPos(10.0f, ImGui::GetIO().DisplaySize.y - windowSize.y - 10.0f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

    ImGui::Begin("Additional functions:", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove); // окно не ресайзится пользователем
    if (ImGui::Button("Show All Meshes")) // кнопка в UI
    {
        if (model)
            model->selectMesh(-1); // сбрасываем выбор, рисуем все меши
    }

    static bool pickingEnabled = true;
    ImGui::Checkbox("Enable mesh separation", &pickingEnabled);

    if (model)
        model->setPickingEnabled(pickingEnabled);

    ImGui::End();

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