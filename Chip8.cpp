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
				//8XY4 Assign vX += vY with Carry
				case 0x4: {
					if (Register[(OperationCode & 0x00F0) >> 4] > (0xFF - Register[(OperationCode & 0x0F00) >> 8])) {
						Register[0xF] = 1;
					} else {
						Register[0xF] = 0;
					}

					Register[(OperationCode & 0x0F00) >> 8] += Register[(OperationCode & 0x00F0) >> 4];
					ProgramCounter += 2;
					break;
				}
				//8XY5 Assign vX -= vY With Borrow
				case 0x5: {
					if (Register[(OperationCode & 0x00F0) >> 4] > Register[(OperationCode & 0x0F00) >> 8]) {
						Register[0xF] = 0;
					} else {
						Register[0xF] = 1;
					}

					Register[(OperationCode & 0x0F00) >> 8] -= Register[(OperationCode & 0x00F0) >> 4];
					ProgramCounter += 2;
					break;
				}
				//8XY6 Assign vX >>= 1 and Store the LSB into vF
				case 0x6: {
					Register[0xF] = Register[(OperationCode & 0x0F00) >> 8] & 0x1;
					Register[(OperationCode & 0x0F00) >> 8] >>= 1;
					ProgramCounter += 2;
					break;
				}
				//8XY7 Assign vX = vY = vX With Borrow
				case 0x7: {
					if (Register[(OperationCode & 0x0F00) >> 8] > Register[(OperationCode & 0x00F0) >> 4]) {
						Register[0xF] = 0;
					} else {
						Register[0xF] = 1;
					}

					Register[(OperationCode & 0x0F00) >> 8] = Register[(OperationCode & 0x00F0) >> 4] - Register[(OperationCode & 0x0F00) >> 8];
					ProgramCounter += 2;
					break;
				}
				//8XYE Assign vX <<= 1 and Store MSB into vF
				case 0xE: {
					Register[0xF] = Register[(OperationCode & 0x0F00) >> 8] >> 7;
					Register[(OperationCode & 0x0F00) >> 8] <<= 1;
					ProgramCounter += 2;
					break;
 				}

				default: {
					Invalid = true;
					ProgramCounter += 2;
					break;
				}
			}
			break;
		}
		//9XY0 Skip Next if vX != vY
		case 0x9000: {
			if (Register[(OperationCode & 0x0F00) >> 8] != Register[(OperationCode & 0x00F0) >> 4]) {
				ProgramCounter += 2;
			}
			ProgramCounter += 2;
			break;
		}
		//ANNN Set I to NNN
		case 0xA000: {
			IndexRegister = OperationCode & 0x0FFF;
			ProgramCounter += 2;
			break;
		}
		//BNNN Jump to v0 + NNN
		case 0xB000: {
			ProgramCounter = (OperationCode & 0x0FFF) + Register[0];
			break;
		}
		//CXNN Set vX to rand & NN
		case 0xC000: {
			Register[(OperationCode & 0x0F00) >> 8] = (rand() % 0xFF) & (OperationCode & 0x00FF);
			ProgramCounter += 2;
			break;
		}
		//DXYN Display X, Y, N
		case 0xD000: {
			auto x = Register[(OperationCode & 0x0F00) >> 8];
			auto y = Register[(OperationCode & 0x00F0) >> 4];
			u8 Height = OperationCode & 0x000F;

			Register[0xF] = 0;

			for (int YLine = 0; YLine < Height; ++YLine) {
				auto Pixel = Memory[IndexRegister + YLine];
				for (int XLine = 0; XLine < 8; ++XLine) {
					if ((Pixel & (0x80 >> XLine)) != 0) {
						if (Display[(x + XLine + (y + YLine) * 64)] == 1) {
							Register[0xF] = 1;
						}
						Display[x + XLine + ((y + YLine) * 64)] ^= 1;
					}
				}
			}
			ProgramCounter += 2;
			Redraw = true;
			break;
		}
		case 0xE000: {
			switch (OperationCode & 0x00FF) {
				//EX9E Skip an Instruction if Key Stored in vX is True
				case 0x009E: {
					if (KeyState[Register[(OperationCode & 0x0F00) >> 8]]) {
						ProgramCounter += 2;
					}
					ProgramCounter += 2;
					break;
				}
				//EXA1 Skip an Instuction if Key Stored in vX is False
				case 0x00A1: {
					if (!KeyState[Register[(OperationCode & 0x0F00) >> 8]]) {
						ProgramCounter += 2;
					}
					ProgramCounter += 2;
					break;
				}

				default: {
					Invalid = true;
					ProgramCounter += 2;
					break;
				}
			}
			break;
		}
		case 0xF000: {
			switch (OperationCode & 0x00FF) {
				//FX07 Assign vX = DelayTimer
				case 0x0007: {
					Register[(OperationCode & 0x0F00) >> 8] = DelayTimer;
					ProgramCounter += 2;
					break;
				}
				//FX0A Wait for Keypress and then set the key to vX
				case 0x000A: {
					bool IsKeyPressed = false;

					for (int i = 0; i < 16; ++i) {
						if (KeyState[i]) {
							IsKeyPressed = true;
							Register[(OperationCode & 0x0F00) >> 8] = i;
						}
					}

					if (!IsKeyPressed) {
						return;
					}

					ProgramCounter += 2;
					break;
				}
				//FX15 Assign DelayTimer = vX
				case 0x0015: {
					DelayTimer = Register[(OperationCode & 0x0F00) >> 8];
					ProgramCounter += 2;
					break;
				}
				//FX18 Assign SoundTimer = vX (Not Implemented)
				case 0x0018: {
					ProgramCounter += 2;
					break;
				}
				//FX1E Set IndexRegister += vX with Carry
				case 0x001E: {
					if (IndexRegister + Register[(OperationCode & 0x0F00) >> 8] > 0xFFF) {
						Register[0xF] = 1;
					} else {
						Register[0xF] = 0;
					}

					IndexRegister += Register[(OperationCode & 0x0F00) >> 8];
					ProgramCounter += 2;
					break;
				}
				//FX29 Set IndexRegister = SpriteLocation[vX]
				case 0x0029: {
					//Sprites are Stored from 0x0000 to 0x0200. Each is 5 bytes
					IndexRegister = Register[(OperationCode & 0x0F00) >> 8] * 0x5;
					ProgramCounter += 2;
					break;
				}
			}
		}
	}
}
void Chip8::TickTimer() {
	if (DelayTimer > 0) {
		DelayTimer--;
	}
}
