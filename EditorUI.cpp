#include "EditorUI.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

void EditorUI::beginFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void EditorUI::render()
{
	drawPrimitivesWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorUI::drawPrimitivesWindow()
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);

    ImGui::Begin(
        "Primitives",
        nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize
    );

    ImVec2 buttonSize(120, 40);

    if (ImGui::Button("Cube", buttonSize))
        requestedPrimitive = PrimitiveType::Cube;

    if (ImGui::Button("Pyramid", buttonSize))
        requestedPrimitive = PrimitiveType::Pyramid;

    if (ImGui::Button("Sphere", buttonSize))
        requestedPrimitive = PrimitiveType::Sphere;

    ImGui::End();
}

PrimitiveType EditorUI::consumePrimitiveRequest()
{
    PrimitiveType result = requestedPrimitive;
    requestedPrimitive = PrimitiveType::None;
    return result;
}