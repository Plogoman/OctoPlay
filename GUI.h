#ifndef GUI_H
#define GUI_H

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Type.h"
#include "Chip8.h"

class GUI {
	GLuint KeyMap[16] = {
		GLFW_KEY_X, // X(0)
		GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, // 1(1), 2(2), 3(3)
		GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E, // Q(4), W(5), E(6)
		GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, // A(7), S(8), D(9)
		GLFW_KEY_Z, GLFW_KEY_C,             // Z(A), C(B)
		GLFW_KEY_4,                         // 4(C)
		GLFW_KEY_R,							// R(D)
		GLFW_KEY_F,                         // F(E)
		GLFW_KEY_V							// V(F)
	};

	ImVec4 ForeGroundColor = ImVec4(0.317f, 0.639f, 0.639f, 1.0f);
	ImVec4 BackGroundColor = ImVec4(0.047f, 0.047f, 0.047, 1.0f);
	ImVec4 LabelColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
	ImVec4 SuccessColor = ImVec4(0.380f, 1.0f, 0.203f, 1.0f);

	i32 NumberOfTicks = 0;
	bool SingleStepMode = false;

	i32 ClockSpeed = 960;
	i32 PreviousClockSpeed = ClockSpeed;
	Chip8 *Interpreter;

	NanoTimePoint LastTimer;

	GLuint DisplayTexture;
	GLubyte *DisplayPixels;


};

#endif //GUI_H
