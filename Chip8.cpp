#include <cstdlib>
#include <fstream>
#include <iostream>

#include "Type.h"
#include "Chip8.h"

void Chip8::Reset() {
	//Program Counter starts at 0x200
	ProgramCounter = 0x200;

	// Reset Other State
	IndexRegister = 0;
	StackPointer = 0;
	OperationCode = 0;

	//Reset Timers
	DelayTimer = 0;

	// Clear Display
	for (bool &Display : Display) {
		Display = false;
	}

	// Clear Register
	for (u8 &Register : Register) {
		Register = 0;
	}

	// Clear Stack
	for (u16 &Stack : Stack) {
		Stack = 0;
	}

	for (int i = 0; i < 80; ++i) {
		Memory[i] = Font[i];
	}
}
bool Chip8::LoadProgram(const String &File) {
	InputFileStream InputFile;

	// we have to store it as binary because the rom is in binary
	InputFile.open(File, std::ios::binary);
	if (!InputFile) {
		return false;
	}

	char CurrentByte;
	for (int i = 0; InputFile.get(CurrentByte); ++i) {
		// 512 is 0x200 which increments after
		Memory[i + 512] = CurrentByte;
	}

	InputFile.close();

	return true;
}
void Chip8::StackPush(u16 Data) {
	Stack[StackPointer] = Data;
	StackPointer++;
}
u16 Chip8::StackPop() {
	StackPointer--;
	i32 Data = Stack[StackPointer];
	return Data;
}
void Chip8::Tick() {
	OperationCode = Memory[ProgramCounter] << 8 | Memory[ProgramCounter + 1];
	bool Invalid = false;

	/*
	 *The AND Operation done here says what the Instruction is supposed to be
	 *for example 0x1234 & 0xF000 = 0x1000
	 */
	switch (OperationCode & 0xF000) {
		case 0x0000: {
			switch (OperationCode) {
				//0x00E0 Clear the Screen
				case 0x00E0: {
					for (int i = 0; i < 2048; ++i) {
						Display[i] = false;
					}
					ProgramCounter += 2;
					Redraw = true;
					break;
				}
				//0x00EE Return from Subroutine
				case 0x00EE: {
					ProgramCounter = StackPop();
					ProgramCounter += 2;
					break;
				}

				default: {
					Invalid = true;
					ProgramCounter += 2;
					break;
				}
			}
		}
		//1NNN Jump to NNN
		case 0x1000: {
			ProgramCounter = OperationCode & 0x0FFF;
			break;
		}
		//2NNN Call to NNN
		case 0x2000: {
			StackPush(ProgramCounter);
			ProgramCounter = OperationCode & 0x0FFF;
			break;
		}
		//3XNN Skip Next if NN == vX
		case 0x3000: {
			if (Register[(OperationCode & 0x0F00) >> 8] == (OperationCode & 0x00FF)) {
				ProgramCounter += 2;
			}
			ProgramCounter += 2;
			break;
		}
		//4XNN Skip Next if NN != vX
		case 0x4000: {
			if (Register[(OperationCode & 0x0F00) >> 8] != (OperationCode & 0x00FF)) {
				ProgramCounter += 2;
			}
			ProgramCounter += 2;
			break;
		}
		//5XY0 Skip Next if vX == vY
		case 0x5000: {
			if (Register[(OperationCode & 0x0F00) >> 8] == Register[(OperationCode & 0x00F0) >> 4]) {
				ProgramCounter += 2;
			}
			ProgramCounter += 2;
			break;
		}
		//6XNN Set Register X to NN
		case 0x6000: {
			Register[(OperationCode & 0x0F00) >> 8] = OperationCode & 0x00FF;
			ProgramCounter += 2;
			break;
		}
		//7XNN Add NN to Register X
		case 0x7000: {
			Register[(OperationCode & 0x0F00) >> 8] += OperationCode & 0x00FF;
			ProgramCounter += 2;
			break;
		}
		case 0x8000: {
			switch (OperationCode & 0x000F) {
				//8XY0 Assign vX = vY
				case 0x0: {
					Register[(OperationCode & 0x0F00) >> 8] = Register[(OperationCode & 0x00F0) >> 4];
					ProgramCounter += 2;
					break;
				}
				//8XY1 Assign vX = vX | vY
				case 0x1: {
					Register[(OperationCode & 0x0F00) >> 8] |= Register[(OperationCode & 0x00F0) >> 4];
					ProgramCounter += 2;
					break;
				}
				//8XY2 Assign vX = vX & vY
				case 0x2: {
					Register[(OperationCode & 0x0F00) >> 8] &= Register[(OperationCode & 0x00F0) >> 4];
					ProgramCounter += 2;
					break;
				}
				//8XY3 Assign vX = vX ^ vY
				case 0x3: {
					Register[(OperationCode & 0x0F00) >> 8] ^= Register[(OperationCode & 0x00F0) >> 4];
					ProgramCounter += 2;
					break;
				}

			}
		}
		case 0x9000: {

		}
		case 0xA000: {

		}
		case 0xB000: {

		}
		case 0xC000: {

		}
		case 0xD000: {

		}
		case 0xE000: {

		}
		case 0xF000: {

		}
	}
}
void Chip8::TickTimer() {
	if (DelayTimer > 0) {
		DelayTimer--;
	}
}
