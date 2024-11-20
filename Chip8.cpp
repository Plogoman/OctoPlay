#include <fstream>
#include <iostream>
#include <random>

#include "Type.h"
#include "Chip8.h"

static std::random_device rd;
static std::mt19937 gen(rd());
std::uniform_int_distribution<u8> dist(0, 255);

void Chip8::Reset() {
	ProgramCounter = 0x200;
	IndexRegister = 0;
	StackPointer = 0;
	OperationCode = 0;
	DelayTimer = 0;

	// Clear Display
	std::fill(Display.begin(), Display.end(), false);

	// Clear Registers
	std::fill(Register.begin(), Register.end(), 0);

	// Clear Memory
	std::fill(Memory.begin(), Memory.end(), 0);

	// Load Font
	for (i32 i = 0; i < 80; ++i) {
		Memory[0x50 + i] = Font[i];
	}

	//Clear Stack
	std::fill(Stack.begin(), Stack.end(), 0);

	//Clear Key State
	std::fill(KeyState.begin(), KeyState.end(), false);
}
bool Chip8::LoadProgram(const String &File) {
	InputFileStream InputFile(File, std::ios::binary | std::ios::ate);
	if (!InputFile.is_open()) {
		std::cerr << "Failed to Open File: " << File << std::endl;
		return false;
	}

	std::streamsize size = InputFile.tellg();
	InputFile.seekg(0, std::ios::beg);

	if (size > (4096 - 512)) {
		std::cerr << "ROM Too Large! Size: " << size << " bytes." << std::endl;
		InputFile.close();
		return false;
	}

	if (!InputFile.read(reinterpret_cast<char*>(&Memory[0x200]), size)) {
		std::cerr << "Failed to Read ROM Data" << std::endl;
		InputFile.close();
		return false;
	}

	InputFile.close();

	// Debug: Print confirmation and size
	std::cout << "ROM loaded successfully. Size: " << size << " bytes." << std::endl;

	// Optional: Print the first few opcodes
	std::cout << "First few opcodes:" << std::endl;
	for (std::streamsize i = 0; i < size && i < 10; i += 2) {
		u16 opcode = (Memory[0x200 + i] << 8) | Memory[0x200 + i + 1];
		std::cout << "0x" << std::hex << opcode << " ";
	}
	std::cout << std::endl;

	return true;
}
void Chip8::StackPush(u16 Data) {
	if (StackPointer >= Stack.size()) {
		std::cerr << "Stack Overflow!" << std::endl;
		// Handle error
		return;
	}
	Stack[StackPointer] = Data;
	StackPointer++;
}
u16 Chip8::StackPop() {
	if (StackPointer == 0) {
		std::cerr << "Stack Underflow!" << std::endl;
		// Handle error
		return 0;
	}
	StackPointer--;
	u16 Data = Stack[StackPointer];
	return Data;
}
void Chip8::Tick() {
	if (ProgramCounter >= Memory.size()) {
		std::cerr << "Program Counter Out of Bounds: " << ProgramCounter << std::endl;
	}

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
					for (i32 i = 0; i < 2048; ++i) {
						Display[i] = false;
					}
					ProgramCounter += 2;
					Redraw = true;
					break;
				}
				//0x00EE Return from Subroutine
				case 0x00EE: {
					ProgramCounter = StackPop();
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
			StackPush(ProgramCounter + 2);
			ProgramCounter = OperationCode & 0x0FFF;
			break;
		}
		//3XNN Skip Next if NN == vX
		case 0x3000: {
			if (Register[(OperationCode & 0x0F00) >> 8] == (OperationCode & 0x00FF)) {
				ProgramCounter += 4;
			} else {
				ProgramCounter += 2;
			}
			break;
		}
		//4XNN Skip Next if NN != vX
		case 0x4000: {
			if (Register[(OperationCode & 0x0F00) >> 8] != (OperationCode & 0x00FF)) {
				ProgramCounter += 4;
			} else {
				ProgramCounter += 2;
			}
			break;
		}
		//5XY0 Skip Next if vX == vY
		case 0x5000: {
			if (Register[(OperationCode & 0x0F00) >> 8] == Register[(OperationCode & 0x00F0) >> 4]) {
				ProgramCounter += 4;
			} else {
				ProgramCounter += 2;
			}
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
				ProgramCounter += 4;
			} else {
				ProgramCounter += 2;
			}
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
			Register[(OperationCode & 0x0F00) >> 8] = (rand() % 256) & (OperationCode & 0x00FF);
			ProgramCounter += 2;
			break;
		}
		//DXYN Display X, Y, N
		case 0xD000: {
			u8 X = Register[(OperationCode & 0x0F00) >> 8];
			u8 Y = Register[(OperationCode & 0x00F0) >> 4];
			u8 height = OperationCode & 0x000F;
			Register[0xF] = 0;

			for (int yline = 0; yline < height; yline++) {
				if (IndexRegister + yline >= Memory.size()) {
					std::cerr << "Index Register Out of Bounds during Draw Opcode." << std::endl;
					break;
				}
				u8 pixel = Memory[IndexRegister + yline];
				for (int xline = 0; xline < 8; xline++) {
					if ((pixel & (0x80 >> xline)) != 0) {
						int xpos = (X + xline) % 64;
						int ypos = (Y + yline) % 32;
						int index = xpos + (ypos * 64);

						if (Display[index]) {
							Register[0xF] = 1;
						}
						Display[index] ^= true;
					}
				}
			}
			Redraw = true;
			ProgramCounter += 2;
			break;
		}
		case 0xE000: {
			switch (OperationCode & 0x00FF) {
				//EX9E Skip an Instruction if Key Stored in vX is True
				case 0x009E: {
					if (KeyState[Register[(OperationCode & 0x0F00) >> 8]]) {
						ProgramCounter += 4;
					} else {
						ProgramCounter += 2;
					}
					break;
				}
				//EXA1 Skip an Instuction if Key Stored in vX is False
				case 0x00A1: {
					if (!KeyState[Register[(OperationCode & 0x0F00) >> 8]]) {
						ProgramCounter += 4;
					} else {
						ProgramCounter += 2;
					}
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

					for (i32 i = 0; i < 16; ++i) {
						if (KeyState[i]) {
							Register[(OperationCode & 0x0F00) >> 8] = i;
							IsKeyPressed = true;
							break;
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
					IndexRegister = 0x50 + (Register[(OperationCode & 0x0F00) >> 8] * 5);
					ProgramCounter += 2;
					break;
				}
				//FX33 Set Index, Index + 1, Index + 2 = BCD(vX)
				case 0x0033: {
					Memory[IndexRegister] = Register[(OperationCode & 0x0F00) >> 8] / 100;
					Memory[IndexRegister + 1] = (Register[(OperationCode & 0x0F00) >> 8] / 10) % 10;
					Memory[IndexRegister + 2] = (Register[(OperationCode & 0x0F00) >> 8] % 10) % 10;

					ProgramCounter += 2;
					break;
				}
				//FX55 Store Registers in Memory
				case 0x0055: {
					for (i32 i = 0; i <= ((OperationCode & 0x0F00) >> 8); ++i) {
						Memory[IndexRegister + i] = Register[i];
					}

					IndexRegister += ((OperationCode & 0x0F00) >> 8) + 1;
					ProgramCounter += 2;
					break;
				}
				//FX65 Load Registers in Memory
				case 0x0065: {
					for (i32 i = 0; i < ((OperationCode & 0x0F00) >> 8); ++i) {
						Register[i] = Memory[IndexRegister + i];
					}

					IndexRegister += ((OperationCode & 0x0F00) >> 8) + 1;
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

		default: {
			Invalid = true;
			ProgramCounter += 2;
			break;
		}
	}
	if (Invalid) {
		std::cerr << "Invalid Opcode: 0x" << std::hex << OperationCode << std::endl;
	}
}
void Chip8::TickTimer() {
	if (DelayTimer > 0) {
		DelayTimer--;
	}
}
