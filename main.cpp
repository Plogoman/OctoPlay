#include <iostream>
#include <cstdlib>
#include <ctime>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Chip8.h"
#include "Type.h"
#include "GUI.h"

static void GLFWErrorCallback(int Error, const char *Description) {
	std::cerr << "GLFW Error " << Error << ": " << Description << std::endl;
}

i32 main(i32 args, char **argv) {
	if (args < 2) {
		std::cerr << "Usage: " << std::endl << argv[0] << " Chip 8 Program Path the ROM is in .ch8/.CH8" << std::endl;
		return 1;
	}

	srand(static_cast<unsigned>(time(nullptr)));

	glfwSetErrorCallback(GLFWErrorCallback);
	if (!glfwInit()) {
		return 1;
	}

	const char *GLSLVersion = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	GLFWwindow *Window = glfwCreateWindow(1280, 720, "Chip 8", nullptr, nullptr);
	if (Window == nullptr) {
		return 1;
	}

	glfwMakeContextCurrent(Window);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &IO = ImGui::GetIO();
	(void)IO;
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	GLubyte DisplayPixels[64 * 32 * 3];

    return 0;
}
