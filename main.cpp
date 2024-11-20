#include <iostream>
#include <cstdlib>
#include <ctime>

#include <glad/glad.h>
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
		return EXIT_FAILURE;
	}

	srand(static_cast<unsigned>(time(nullptr)));

	glfwSetErrorCallback(GLFWErrorCallback);
	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	const char *GLSLVersion = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *Window = glfwCreateWindow(2880, 1620, "Chip 8", nullptr, nullptr);
	if (Window == nullptr) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(Window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &IO = ImGui::GetIO();
	(void)IO;
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	auto &Style = ImGui::GetStyle();
	Style.FrameRounding = 2;
	Style.WindowRounding = 4;
	Style.WindowPadding = ImVec2(16, 12);
	Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.078f, 0.078f, 0.082f, 1.0f);

	ImGui_ImplGlfw_InitForOpenGL(Window, true);
	ImGui_ImplOpenGL3_Init(GLSLVersion);

	GLubyte DisplayPixels[64 * 32 * 3];
	for (int x = 0; x < 2048; ++x) {
		DisplayPixels[x * 3 + 0] = 0;
		DisplayPixels[x * 3 + 1] = 0;
		DisplayPixels[x * 3 + 2] = 0;
	}

	GLuint DisplayTexture;
	glGenTextures(1, &DisplayTexture);
	glBindTexture(GL_TEXTURE_2D, DisplayTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Nearest for pixelated look
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevent artifacts
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 64, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, DisplayPixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	auto ClearColor = ImVec4(0.039f, 0.039f, 0.039f, 1.0f);

	Chip8 CoreInterpreter;
	CoreInterpreter.Reset();

	if (!CoreInterpreter.LoadProgram(argv[1])) {
		std::cerr << "Unable to Load" << argv[1] << std::endl;
		return -1;
	}

	GUI gui(&CoreInterpreter, DisplayTexture, DisplayPixels);

	while (!glfwWindowShouldClose(Window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		gui.Render();

		ImGui::Render();
		i32 DisplayWidth, DisplayHeight;
		glfwGetFramebufferSize(Window, &DisplayWidth, &DisplayHeight);
		glViewport(0, 0, DisplayWidth, DisplayHeight);
		glClearColor(ClearColor.x * ClearColor.w, ClearColor.y * ClearColor.w, ClearColor.z * ClearColor.w, ClearColor.w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render Additional Platforms Windows
		if (IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow *BackupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(BackupCurrentContext);
		}

		glfwSwapBuffers(Window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteTextures(1, &DisplayTexture);

	glfwDestroyWindow(Window);
	glfwTerminate();

    return 0;
}
