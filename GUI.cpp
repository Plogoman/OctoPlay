#include "GUI.h"

#include <iostream>

GUI::GUI(Chip8 *CoreInterpreter, GLuint DisplayTexture, GLubyte *DisplayPixels) {
    this->CoreInterpreter = CoreInterpreter;
	this->DisplayTexture = DisplayTexture;
	this->DisplayPixels = DisplayPixels;
	LastTimer = HighResolutionClock::now();
}
void GUI::Tick() {
	if (ImGui::IsWindowFocused()) {
		for (int i = 0; i < 16; ++i) {
			CoreInterpreter->KeyState[i] = ImGui::IsKeyDown(static_cast<ImGuiKey>(KeyMap[i]));
		}
	}

	NumberOfTicks++;
	CoreInterpreter->Tick();
}
void GUI::RenderDisplay(f32 FrameRate) {
	ImGui::Begin("Display", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(32 + (64 * DISPLAY_SCALE), 32 + (32 * DISPLAY_SCALE)));

	if (SingleStepMode) {
		SingleStepMode = false;
		Tick();
	}

	i32 TicksPerFrame = std::min(static_cast<i32>(ClockSpeed / FrameRate), MAX_TICKS_PER_FRAME);
	for (int i = 0; i < TicksPerFrame; ++i) {
		Tick();
	}

	auto CurrentTime = HighResolutionClock::now();
	auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(CurrentTime - LastTimer);
	if (deltaTime.count() >= (1e6 / 60)) { // Approximately every 16.67ms for 60Hz
		CoreInterpreter->TickTimer();
		LastTimer = CurrentTime;
	}

	if (CoreInterpreter->Redraw) {
		CoreInterpreter->Redraw = false;

		GLubyte ForeGround[3] = {
			static_cast<GLubyte>(ForeGroundColor.x * 255),
			static_cast<GLubyte>(ForeGroundColor.y * 255),
			static_cast<GLubyte>(ForeGroundColor.z * 255)
		};

		GLubyte BackGround[3] = {
			static_cast<GLubyte>(BackGroundColor.x * 255),
			static_cast<GLubyte>(BackGroundColor.y * 255),
			static_cast<GLubyte>(BackGroundColor.z * 255)
		};

		for (int i = 0; i < 64 * 32; ++i) {
			auto SubPixel = CoreInterpreter->Display[i] ? ForeGround : BackGround;

			for (int j = 0; j < 3; ++j) {
				DisplayPixels[i * 3 + j] = SubPixel[j];
			}
		}

		glBindTexture(GL_TEXTURE_2D, DisplayTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RGB, GL_UNSIGNED_BYTE, DisplayPixels);
		glBindTexture(GL_TEXTURE_2D, 0);

		GLenum Error = glGetError();
		if (Error != GL_NO_ERROR) {
			std::cerr << "OpenGL Error in RenderDisplay" << Error << std::endl;
		}
	}

	ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(DisplayTexture)), ImVec2(64 * DISPLAY_SCALE, 32 * DISPLAY_SCALE));
	ImGui::Text("RenderDisplay Debug");
	ImGui::End();
}
void GUI::RenderGeneral(f32 FrameRate) {
	ImGui::Begin("General", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::TextColored(LabelColor, "FPS: ");
	ImGui::SameLine();
	ImGui::Text("%f", FrameRate);

	ImGui::TextColored(LabelColor, "Ticks: ");
	ImGui::SameLine();
	ImGui::Text("%d", NumberOfTicks);

	ImGui::TextColored(LabelColor, "Display Scale: ");
	ImGui::SameLine();
	ImGui::Text("%d", DISPLAY_SCALE);

	ImGui::TextColored(LabelColor, "Clock: ");
	ImGui::SameLine();
	ImGui::InputInt("Hz", &ClockSpeed);

	ImGui::ColorEdit3("ForeGround Color", reinterpret_cast<float *>(&ForeGroundColor));
	ImGui::ColorEdit3("BackGround Color", reinterpret_cast<float *>(&BackGroundColor));

	ImGui::SliderInt("Clock Speed (Hz)", &ClockSpeed, 60, 1000);

	static i32 MaxTicks = MAX_TICKS_PER_FRAME;
	ImGui::SliderInt("Max Ticks per Frame", &MaxTicks, 1, 50);
	MAX_TICKS_PER_FRAME = MaxTicks;

	ImGui::End();
}
void GUI::Render() {
	auto FrameRate = ImGui::GetIO().Framerate;

	RenderDisplay(FrameRate);
	RenderGeneral(FrameRate);
}
