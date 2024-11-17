#include "GUI.h"

GUI::GUI(Chip8 *CoreInterpreter, GLuint DisplayTexture, GLubyte *DisplayPixels) {
    this->CoreInterpreter = CoreInterpreter;
	this->DisplayTexture = DisplayTexture;
	this->DisplayPixels = DisplayPixels;
	LastTimer = HighResolutionClock::now();
}
constexpr void GUI::Tick() {
	if (ImGui::IsWindowFocused()) {
		for (int i = 0; i < 16; ++i) {
			CoreInterpreter->KeyState[i] = ImGui::IsKeyDown(static_cast<ImGuiKey>(KeyMap[i]));
		}
	}

	NumberOfTicks++;
	CoreInterpreter->Tick();
}

constexpr void GUI::RenderDisplay(float FrameRate) {
	ImGui::Begin("Chip 8 Display", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(, ))
}
