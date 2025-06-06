#pragma once

#include <functional>
#include <memory>
#include <string>
namespace nes
{
	class PPU 
	{
	private:
		std::function<void()> m_nmiCallback;
		std::function<uint8_t(uint16_t)> m_read;
		std::function<void(uint16_t, uint8_t)> m_write;
		uint8_t m_registers[8];
		signed int m_scanlineCounter;
		int m_dotCounter; //Current pixel in the current scanline.

		//Internal memory

		std::shared_ptr<uint8_t[]> m_nameTable; //0x2000-0x3EFF (size 0x0F00)
		std::shared_ptr<uint8_t[]> m_paletteRAM; //0x3F00-0x3FFF (size 0x00E0)
		bool m_mirroring;

		//Rendering surfaces...
		std::shared_ptr<uint8_t[]> m_screenBuf0;
		std::shared_ptr<uint8_t[]> m_screenBuf1;

		std::shared_ptr<uint8_t[]> m_backBuffer;
		std::shared_ptr<uint8_t[]> m_screen;

		//Sprites...
		uint8_t m_oam[0x100]; //Sprite ram (OAM)
		uint8_t m_oamAddr;    //Current OAM address
		int m_spriteScanIdx[8]; //Indices of sprites on the current scanline
		int m_spriteX[8]; //Sprite X countdown for current scanline.
		uint8_t m_spriteShiftHi[8]; //Sprite bitmap shift reg high bit
		uint8_t m_spriteShiftLo[8]; //Sprite bitmap shift reg lo bit
		uint8_t m_spriteAttrib[8]; //Sprite attribute number
		bool m_spriPri[8]; //Priority for sprite on current scanline... 
		unsigned int m_sprCount; //Count of sprites on current scanline.
		bool m_spriteActive[8];

		bool m_cpuAccessLatch;
		uint8_t m_vramReadBuffer;
		
		uint16_t m_bgShiftRegs[2];
		uint16_t m_attrShiftRegs[2];
		uint8_t m_bgAttr;
		uint16_t m_cpuAccessPtr;
		uint8_t hScroll; //Horizontal scroll register
		uint8_t vScroll; //Vertical scroll register
		//Internal PPU functions...
		void renderNextScanline();
		void loadSpriteScan();
		void writeVRAM(uint8_t byte);
		uint8_t readVRAM();

		void renderNextPixel();
		int getNameTable(uint16_t addr);
	public:
		uint8_t readPPU(uint16_t);
		void writePPU(uint16_t addr, uint8_t byte);
		void reset();
		
		void setMirroring(bool mir)
		{
			m_mirroring = mir;
		}
		signed int getScanline()
		{
			return m_scanlineCounter;
		}
		void dmaPage(uint8_t addr, uint8_t byte)
		{
			m_oam[addr] = byte;
		}
		PPU(std::function<void()> nmiCallback,
			std::function<uint8_t(uint16_t)> ppuRead,
			std::function<void(uint16_t, uint8_t)> ppuWrite);
			
		void __debugFakeSprite0Hit();

		void debugDumpPatternTable(std::string filename);

		void debugDumpRawScreen(std::string filename);

		void debugDumpNameTable(std::string filename);

		std::shared_ptr<uint8_t[]> getScreen()
		{
			return m_screen;
		}
		void advanceCycles(int n);
		
		

	};
}