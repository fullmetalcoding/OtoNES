#pragma once

#include "mos6502.h"
#include "IMapper.h"
#include "PPU.h"

#include <memory>
#include <map>
#include <functional>

#define NES_KEYUP 0
#define NES_KEYDOWN 1
#define NES_KEYLEFT 2
#define NES_KEYRIGHT 3
#define NES_KEY_A 4
#define NES_KEY_B 5
#define NES_KEY_ST 6
#define NES_KEY_SL 7

namespace nes
{
	class NesMachine
	{
	private:
		//PPU
		std::shared_ptr<PPU> m_ppu;
		//APU
		//Memory banks
		std::shared_ptr<uint8_t[]> m_internalRAM;
		std::shared_ptr<mappers::IMapper> m_bankMapper;
		//6502 core
		std::shared_ptr<mos6502> m_cpu;
		std::map<uint8_t, std::function<void(uint16_t, uint8_t)> > m_sectionWriteMap;
		std::map<uint8_t, std::function<uint8_t(uint16_t)> > m_sectionReadMap;

		//Memory mapper...
				//Memory access functions...
		void writeInternalRAM(uint16_t address, uint8_t byte);
		uint8_t readInternalRAM(uint16_t address);

		unsigned char m_keymap[3];
		int m_jrladdr;
		bool m_jwlatch;




	public:
		void keyDown(int key);
		void keyUp(int key);
		void writeMem(uint16_t address, uint8_t byte);
		uint8_t readMem(uint16_t address);

		NesMachine(std::shared_ptr<mappers::IMapper> cartMap);
		void reset()
		{
			m_cpu->Reset();
			m_ppu->reset();
			std::cout << "Reset vector: " << std::hex << (int)m_bankMapper->readMapper(0xFFFC) << (int)m_bankMapper->readMapper(0xFFFD) << std::dec << std::endl;
		}
		void advanceCycles(int cycles);
	
		void __debugTriggerNmi()
		{
			m_cpu->NMI();
		}
		void __debugFakeSprite0Hit()
		{
			m_ppu->__debugFakeSprite0Hit();
		}

		std::shared_ptr<PPU> getPPU()
		{
			return m_ppu;
		}
	};
}