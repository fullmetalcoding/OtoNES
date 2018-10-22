#include "PPU.h"
#include <iostream>
#include "nesPalette.h"
#include "bitmanip.h"
namespace nes
{
	void PPU::writeVRAM(uint8_t byte)
	{
		//Prevent writes to CHRROM since it's ROM...
		//Though this will need to be flexible in the future, some mappers allow writing
		//to chr "ROM"
		if ((m_cpuAccessPtr >= 0x2000) && (m_cpuAccessPtr < 0x3000))
		{
			//std::cout << std::hex << m_cpuAccessPtr << std::dec << std::endl;
			//Look to cpuAccessPtr, as this is where we want to write.
			//Figure out where the pointer is in our internal nametables (handle mirroring)
			int nameTable = getNameTable(m_cpuAccessPtr);
			//std::cout << nameTable << ".";
			//Now we know which nametable we're in, we need to make sure the address is less than 0x400
			//To do this, chop off the top 6 bits. 
			uint16_t nametableAddr = (m_cpuAccessPtr & 0x03FF);
			//Now add to the nametableAddr to handle mirroring.
			if (nameTable > 0)
				nametableAddr += 0x400;
			//Update memory
			m_nameTable[nametableAddr] = byte;
			//Increment cpuAccessPTR
		}
		m_cpuAccessPtr += (readbit(m_registers[0], 2) ? 31 : 1);
	}
	uint8_t PPU::readVRAM()
	{
		//Look to cpuAccessPtr, as this is where we want to read
		//Figure out where the pointer is in our internal nametables (handle mirroring)
		int nameTable = getNameTable(m_cpuAccessPtr);
		//Now we know which nametable we're in, we need to make sure the address is less than 0x400
		//To do this, chop off the top 6 bits.
		uint16_t nametableAddr = (m_cpuAccessPtr & 0x03FF);
		//Now add to the nametableAddr to handle mirroring.
		if (nameTable > 0)
			nametableAddr += 0x400;
		
		//Read memory
		uint8_t fetch = m_nameTable[nametableAddr];
		//Increment cpuAccessPTR
		m_cpuAccessPtr++;
		return fetch;
	}
	uint8_t PPU::readPPU(uint16_t addr)
	{
		uint8_t wrappedAddr = (addr - 0x2000) % 8;
		//std::cout << "PPUREAD: " << std::hex << addr << " (" << (int)wrappedAddr << ")" << std::dec << std::endl;
		uint8_t retVal = m_registers[wrappedAddr];
		if (wrappedAddr = 0x02)
		{
			//Clear vblank flag if we read 0x2002
			uint8_t reg = retVal;
			clearbit(reg, 7);
			m_cpuAccessLatch = false;
			m_registers[wrappedAddr] = reg;
		}
		return retVal;
	}
	void PPU::writePPU(uint16_t addr, uint8_t byte)
	{

		uint8_t wrappedAddr = (addr - 0x2000) % 8;
		//std::cout << "PPUWRITE: " << std::hex << addr << " (" << (int)wrappedAddr << ") : " << (int)byte << std::endl;
		m_registers[wrappedAddr] = byte;
		switch (addr)
		{
		case 0x2005:
		//	std::cout << '.';
			break;
		case 0x2006:
			if (m_cpuAccessLatch)
			{
				//Write lower
				m_cpuAccessPtr &= 0xFF00;
				m_cpuAccessPtr |= byte;
				m_cpuAccessLatch = false;
			//	std::cout << "LATCH: " << std::hex << m_cpuAccessPtr << std::dec << std::endl;
			}
			else
			{
				//Write higher
				m_cpuAccessPtr &= 0x00FF;
				m_cpuAccessPtr |= (byte << 8);
				m_cpuAccessLatch = true;
				//std::cout << "H: " << std::hex << m_cpuAccessPtr << std::endl;
			}
			break;
		case 0x2007:
			//Write the VRAM.
			//After access, the video memory address will increment by an amount determined by bit 2 of $2000 (reg 0)
			//std::cout << std::hex << "!" <<  m_cpuAccessPtr << std::dec << std::endl; 
			writeVRAM(byte);
			break;

		default:
			break;
		}

	}
	void PPU::reset()
	{
		for (size_t i = 0; i < 8; i++)
		{
			m_registers[i] = 0;

		}
		for (size_t i = 0; i < 256; i++)
		{
			m_oam[i] = 0;
		}

		writebit(m_registers[2], 1, 7);
		writebit(m_registers[2], 1, 5);
		//writebit(m_registers[0], 1, 7);
		m_scanlineCounter = -1;
		m_dotCounter = 0;
		m_cpuAccessLatch = false;
		m_cpuAccessPtr = 0x2000;

	}
	void PPU::renderNextScanline()
	{

		m_scanlineCounter++;
	
		if (m_scanlineCounter == 241)
		{
			//Set vblank bit.
			writebit(m_registers[2], 1, 7);
			if (readbit(m_registers[0], 7))
				m_nmiCallback();

			//Swap our buffers... 
			if (m_backBuffer == m_screenBuf0)
			{
				m_screen = m_screenBuf0;
				m_backBuffer = m_screenBuf1;

			}
			else
			{
				m_screen = m_screenBuf1;
				m_backBuffer = m_screenBuf0;

			}
		}
		if (m_scanlineCounter == 261)
		{
			writebit(m_registers[2], 0, 7);
			writebit(m_registers[2], 0, 6);
		}
		if (m_scanlineCounter > 262)
		{
			m_scanlineCounter = -1;
			writebit(m_registers[2], 0, 5);

		
		}

	}
	PPU::PPU(std::function<void()> nmiCallback,
		std::function<uint8_t(uint16_t)> ppuRead,
		std::function<void(uint16_t, uint8_t)> ppuWrite)
		: m_nmiCallback(nmiCallback),
		m_read(ppuRead),
		m_write(ppuWrite)
	{
		m_screenBuf0.reset(new uint8_t[256 * 256 * 3]);
		m_screenBuf1.reset(new uint8_t[256 * 256 * 3]);

		//Fill the screen buffers with red and black for debug purposes...
		for (size_t i = 0; i < 256 * 256; i++)
		{
			m_screenBuf0[i * 3] = 0;
			m_screenBuf0[i * 3 + 1] = 0;
			m_screenBuf0[i * 3 + 2] = 0;

			m_screenBuf1[i * 3] = 0;
			m_screenBuf1[i * 3 + 1] = 0;
			m_screenBuf1[i * 3 + 2] = 0;

		}

		m_screen = m_screenBuf0;
		m_backBuffer = m_screenBuf1;
		m_nameTable.reset(new uint8_t[0x800]);
	}
	void PPU::__debugFakeSprite0Hit()
	{
		writebit(m_registers[2], 1, 6);

	}
	void PPU::advanceCycles(int n)
	{
		for (size_t i = 0; i < n; i++)
		{

		//	std::cout << m_dotCounter << std::endl;
			//Each PPU cycle renders one pixel to the buffer
			//If we reach the end of the scanline, update the scanline.
			if (m_dotCounter > 255)
			{
				m_dotCounter = 0;
				m_shifterCount = 0;
				renderNextScanline();
			}
			
			if ((m_scanlineCounter >= 0) && (m_scanlineCounter < 241))
			{
				//std::cout << "." << m_dotCounter << "|" << std::endl;
				if ((m_shifterCount % 8) == 0)
				{
					//Dot count = X
					int tileX = m_dotCounter / 8;



					//Line count = Y
					int tileY = m_scanlineCounter / 8;

					//Adjust for mirroring. in Horizontal X gets % 32
					//in Vertical, Y gets  % 30
					int pitch;
					if (m_mirroring)
					{
						tileY = tileY % 30;
						pitch = 30;
					}
					else
					{
						tileX = tileX % 32;
						pitch = 64;
					}

					//std::cout << "X: " << tileX << "Y: " << tileY << std::endl;
					uint8_t tileRef = m_nameTable[tileY * pitch + tileX];
					//std::cout << (int)tileRef << std::endl;
					uint8_t tileByte = m_read(tileRef* 16 + (readbit(m_registers[0], 4) ? 0x1000 : 0x0000));
					uint8_t tileByte2 = m_read(tileRef* 16 + (readbit(m_registers[0], 4) ? 0x1000 : 0x0000) + 8);
					m_bgShiftRegs[0] = (m_bgShiftRegs[0] & 0x00FF) | (tileByte << 8);
					m_bgShiftRegs[1] = (m_bgShiftRegs[1] & 0x00FF) | (tileByte << 8);

				}
				//we're live... Render next pixel...
				renderNextPixel();
			}
			//Shift down.
			m_bgShiftRegs[0] = m_bgShiftRegs[0] >> 1;
			m_bgShiftRegs[1] = m_bgShiftRegs[1] >> 1;
			m_attributeShiftRegs[0] = m_attributeShiftRegs[0] >> 1;
			m_attributeShiftRegs[1] = m_attributeShiftRegs[1] >> 1;
			m_dotCounter++;
			m_shifterCount++;
		}
	}

	void PPU::renderNextPixel()
	{

		//Read from the pattern table to get the pixel color.
		//Adjust with attribute table.
		//Check for any sprites on this pixel...

		//Place the pixel in our current backbuffer...
		int colorIndex = (readbit(m_attributeShiftRegs[1], 0) << 3)
			            | (readbit(m_attributeShiftRegs[1], 0) << 2)
						| (readbit(m_bgShiftRegs[1], 0) << 1) 
						| readbit(m_bgShiftRegs[0], 0);
		RGB pixelColor = nes_palette[colorIndex];

		m_backBuffer[3 * ((m_scanlineCounter * 256) + m_dotCounter)] = pixelColor.r;
		m_backBuffer[3 * ((m_scanlineCounter * 256) + m_dotCounter) + 1] = pixelColor.g;
		m_backBuffer[3 * ((m_scanlineCounter * 256) + m_dotCounter) + 2] = pixelColor.b;
	}

	int PPU::getNameTable(uint16_t addr)
	{
		uint16_t reducedAddr = (addr - 0x2000) >> 10;
		if (m_mirroring)
		{
			//Vertical mirroring. 0x2000 = 0x2800, 0x2400 = 0x2C00
			//0 - 0x400 is 0 or 2 (EVENS)
			//0x400-0x2800 is 1 or 3 (ODDS)
			return (reducedAddr & 0x1);//Breaks down to odd or even.			
		}
		else
		{
			//Horizontal mirroring. 0x2000 = 0x2400, 0x2800 = 0x2C00
			//0-0x800 is 0 or 1
			// > 0x800 is 2 or 3
			return ((reducedAddr > 1) ? 1 : 0);
		}
		return 0;
	}
}
