//
// Created by Plogoman on 11/15/2024.
//

#ifndef CHIP8_H
#define CHIP8_H

#include "Type.h"

class Chip8 {
	private:
	    u8 Font[80] = {
	        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	        0x20, 0x60, 0x20, 0x20, 0x70, // 1
	        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	        0xF0, 0x80, 0xF0, 0x80, 0x80, // F
	    };

		void StackPush(u16 Data);
		u16 StackPop();

	public:
		Array<bool, 64 * 32> Display;
		bool Redraw = false;
		Array<u8, 4096> Memory;
		Array<bool, 16> KeyState;
		Array<u8, 16> Register;
		u16 ProgramCounter;
		u16 IndexRegister;
		u16 OperationCode;
		Array<u16, 16> Stack;
		u8 StackPointer;
		u8 DelayTimer;

		void Reset();
		bool LoadProgram(const String &File);
		void Tick();
		void TickTimer();
};

#endif //CHIP8_H
