#include "PPU.h"
#include <iostream>
#include "nesPalette.h"
#include "bitmanip.h"
#include <fstream>
#include <bitset>



inline bool isAttrSpace(uint16_t addr)
{
	//This is to check if our tile address is in attribute table space
	//if so, we want to add 0x40 to push us into the next name table.
	//Need a faster, cleaner, cleverer way to do this.
	uint16_t uglyHack = addr - 0x2000;
	bool inAttrSpace = (uglyHack >= 0x03C0 && uglyHack < 0x0400) ||
		(uglyHack >= 0x07C0 && uglyHack < 0x0800) ||
		(uglyHack >= 0x0BC0 && uglyHack < 0x0C00);//+ ||
												 //	(uglyHack > 0x0FC0 && uglyHack < 0x1000);//Shouldn't need since we'd be off the end of the nametable...
	return inAttrSpace;
}
namespace nes
{
	inline uint16_t getBaseNametable(int n)
	{
		uint16_t retVal = 0x2000;
		switch (n) {
		case 0:
			retVal = 0x2000;
			break;
		case 1:
			retVal = 0x2400;
			break;
		case 2:
			retVal = 0x2800;
			break;
		case 3:
			retVal = 0x2C00;
			break;
		}
		return retVal;

	}
	void PPU::writeVRAM(uint8_t byte)
	{
		//Prevent writes to CHRROM since it's ROM...
		//Though this will need to be flexible in the future, some mappers allow writing
		//to chr "ROM"
		if ((m_cpuAccessPtr >= 0x3000) && (m_cpuAccessPtr < 0x3F00))
		{
			//Handle mirroring name tables. 
			clearbit(m_cpuAccessPtr, 13);
		}
		if ((m_cpuAccessPtr >= 0x2000) && (m_cpuAccessPtr < 0x3000))
		{

			//Look to cpuAccessPtr, as this is where we want to write.
			//Figure out where the pointer is in our internal nametables (handle mirroring)
			int nameTable = getNameTable(m_cpuAccessPtr);
			//std::cout << nameTable << ".";
			//Now we know which nametable we're in, we need to make sure the address is less than 0x400
			//To do this, chop off the top 6 bits. 
			uint16_t nametableAddr = (m_cpuAccessPtr & 0x03FF);
			//Now add to the nametableAddr to handle mirroring.

			nametableAddr += nameTable *  0x400;
			//Update memory
			m_nameTable[nametableAddr] = byte;
			//Increment cpuAccessPTR
		}
		else if((m_cpuAccessPtr >= 0x3F00) && (m_cpuAccessPtr < 0x4000))
		{
			//Palette RAM write. With mirroring we only care about the lower 5 bits.
			//Only the first part of each 4 addresses is mirrored, not the rest...
			//Who came up with this hardware design?!
			uint16_t ptr = m_cpuAccessPtr;
			//Compiler optimize this for me plz kthx
			bool weirdMirror = 
				(ptr == 0x3F10) ||
				(ptr == 0x3F14) ||
				(ptr == 0x3F18) ||
				(ptr == 0x3F1C);
			if (weirdMirror) ptr &= 0xFFEF; //Quit FFEFing about!
			uint16_t paletteIdx = ptr & 0x001F;
			m_paletteRAM[paletteIdx] = byte;
		}
		else {
			m_write(m_cpuAccessPtr, byte);
		}
		m_cpuAccessPtr +=  (readbit(m_registers[0], 2) ? 32 : 1);
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
		uint8_t retVal = m_registers[wrappedAddr];
		if (wrappedAddr == 0x02)
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
		m_registers[wrappedAddr] = byte;
		switch (addr)
		{
		case 0x2005:
			if (m_cpuAccessLatch)
			{
				vScroll = byte;
				m_cpuAccessLatch = false;
			}
			else
			{
				hScroll = byte;
				m_cpuAccessLatch = true;
			}
			break;
		case 0x2006:
			if (m_cpuAccessLatch)
			{
				//Write lower
				m_cpuAccessPtr &= 0xFF00;
				m_cpuAccessPtr |= byte;
				m_cpuAccessLatch = false;
			}
			else
			{
				//Write higher
				m_cpuAccessPtr &= 0x00FF;
				m_cpuAccessPtr |= (byte << 8);
				m_cpuAccessLatch = true;
			}
			break;
		case 0x2007:
			//Write the VRAM.
			//After access, the video memory address will increment by an amount determined by bit 2 of $2000 (reg 0)
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
		for (size_t i = 0; i < 255; i++)
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
		hScroll = 0;
		vScroll = 0;

	}
	void PPU::renderNextScanline()
	{
		//writebit(m_registers[2], 0, 6);
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
		if (m_scanlineCounter == 260)
		{
			writebit(m_registers[2], 0, 7);
			writebit(m_registers[2], 0, 6);
		}
		if (m_scanlineCounter > 260)
		{
			m_scanlineCounter = -1;
			writebit(m_registers[2], 0, 5);

		}
		loadSpriteScan();

	}
	void PPU::loadSpriteScan()
	{
		//Find the first 8 sprites on the current scanline
		for (size_t i = 0; i < 8; i++)
		{
			m_spriteScanIdx[i] = 0;
			m_spriteShiftHi[i] = 0;
			m_spriteShiftLo[i] = 0;
			m_spriteX[i] = 0;
			m_spriteActive[i] = 0;
		}
		m_sprCount = 0;
		for (size_t i = 0; i < 64; i++)
		{
			//Each sprite is 4 bytes... The info we're looking for is in byte 0
			int spriteScanline = m_oam[i * 4];
			if ((m_scanlineCounter >= spriteScanline) && (m_scanlineCounter < (spriteScanline + 8)))
			{
				//Found one.
				m_spriteScanIdx[m_sprCount] = (int)i;

				uint8_t lineOffset = m_scanlineCounter - spriteScanline;
				if (readbit(m_oam[i * 4 + 2], 7))
				{
					lineOffset = 7 - lineOffset;
				}
				//Get the sprite's pattern from memory...
				uint8_t tileRef = m_oam[i * 4 + 1];
				uint16_t patternAddr = (readbit(m_registers[0], 3) ? 0x1000 : 0x0000) | (tileRef << 4) | (lineOffset & 0x0007);
				uint16_t patternAddr2 = patternAddr + 0x0008;
				m_spriteShiftHi[m_sprCount] = reverseBits(m_read(patternAddr));
				m_spriteShiftLo[m_sprCount] = reverseBits(m_read(patternAddr2));
				if (readbit(m_oam[i * 4 + 2], 6))
				{
					m_spriteShiftHi[m_sprCount] = reverseBits(m_spriteShiftHi[m_sprCount]);
					m_spriteShiftLo[m_sprCount] = reverseBits(m_spriteShiftLo[m_sprCount]);

				}
				m_spriteX[m_sprCount] = m_oam[i * 4 + 3];
				m_spriteActive[m_sprCount] = 0;
				m_sprCount++;
			}
			if (m_sprCount > 7) break;
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
		m_paletteRAM.reset(new uint8_t[0x20]);

	}
	void PPU::__debugFakeSprite0Hit()
	{
		writebit(m_registers[2], 1, 6);

	}
	void PPU::debugDumpPatternTable(std::string filename)
	{
		std::ofstream debugDumpBin(filename, std::ios::binary);
		for (size_t line = 0; line < 128; line++)
		{
			for (size_t xPix = 0; xPix < 256; xPix++)
			{
				
				    uint16_t side = (uint16_t)xPix / 128;
					uint16_t col = (xPix / 8) % 8 ;
					uint16_t row = line / 8 ;
					uint16_t fineY = line % 8;
					uint16_t pixel = 7-(xPix % 8);
					uint16_t baseAddr = (side ? 0x1000 : 0x0000);

				    uint16_t patternAddr = baseAddr | (row << 8) | (col << 4) | (fineY & 0x0007);
					uint16_t patternAddr2 = patternAddr | 0x0008;
					uint8_t tileByte = m_read(patternAddr);
					uint8_t tileByte2 = m_read(patternAddr2);
					uint8_t colorz = (readbit(tileByte2, pixel) << 1) | readbit(tileByte, pixel);
					debugDumpBin << (uint8_t)(colorz  * 85);
				
			}
		}
		std::cout << "dumped pattern table." << std::endl;
		debugDumpBin.close();
	}
	void PPU::debugDumpRawScreen(std::string filename)
	{
		std::ofstream debugDumpBin(filename, std::ios::binary);
		for (size_t i = 0; i < (256 * 256); i++)
		{
			debugDumpBin << m_screen[i * 3]
				<< m_screen[i * 3 + 1]
				<< m_screen[i * 3 + 2];
					
		}
		debugDumpBin.close();
	}
	void PPU::debugDumpNameTable(std::string filename)
	{
		uint16_t base = getBaseNametable(m_registers[0] & 0x3);
		uint16_t nextTableBase = getBaseNametable(((m_registers[0] & 0x3) + 1) % 4);
		std::ofstream debugDumpBin(filename + ".raw", std::ios::binary);
		//uint8_t base = getNameTable(getBaseNametable(m_registers[0] & 0x3));
		//std::cout << "BASE: " << std::hex << (int)getBaseNametable(m_registers[0] & 0x3) << " : " << (int) base << std::dec << std::endl;
		for (size_t scanline = 0; scanline < 240; scanline++)
		{
			size_t tileY = scanline / 8;
			int line = scanline % 8;
			size_t adjustedTileX, adjustedTileY;
			for (size_t tileX = 0; tileX < 64; tileX++)
			{
				
				int pitch;
				if (m_mirroring)
				{
					tileY = tileY % 30;
					pitch = 32;
					
				}
				else
				{
					adjustedTileX = tileX % 32;
					pitch = 32;
				
				}
				
				int scrolledX = 0 + adjustedTileX;
				//@TODO: Find and kill this hack.
				uint16_t yAdjustHack = tileY - (scrolledX > 31 ? 1 : 0);
				uint16_t tileAddr = (yAdjustHack * pitch) + scrolledX;
				uint16_t adjust = ((scrolledX > 31) ? nextTableBase : base) + tileAddr;

				int nt = getNameTable(adjust);
				uint8_t tileRef = m_nameTable[tileAddr + (nt * 0x400)];
				uint8_t row = (tileRef & 0xF0) >> 4;
				uint8_t col = (tileRef & 0x0F);
				uint16_t patternAddr = (readbit(m_registers[0], 4) ? 0x1000 : 0x0000) | (row << 8) | (col << 4) | (line & 0x0007);
				uint16_t patternAddr2 = patternAddr | 0x0008;
				uint8_t tileByte = m_read(patternAddr);
				uint8_t tileByte2 = m_read(patternAddr2);
					for (size_t i = 0; i < 8; i++)
					{
						uint8_t col;
						col = ((readbit(tileByte2, 7-i) << 1 )| readbit(tileByte, 7-i)) * 85;
						debugDumpBin << col;
					}
			}
			//debugDump << std::endl;
		}
		std::cout << "Dumped nametable." << std::endl;
		debugDumpBin.close();
	}
	void PPU::advanceCycles(int n)
	{
		uint16_t base = getBaseNametable(m_registers[0] & 0x3);
		uint16_t nextTableBase = getBaseNametable(((m_registers[0] & 0x3) + 1) % 4);
		for (size_t i = 0; i < n; i++)
		{


				//Each PPU cycle renders one pixel to the buffer
				//If we reach the end of the scanline, update the scanline.
			if (m_dotCounter > 340)
			{
				renderNextScanline();
				m_dotCounter = 0;
			}
			//If we're between 256 and 340 we're in "HBlank"
			//No pixels are rendered, but memory accesses still occur like
			//they are...


			if ((m_scanlineCounter >= 0) && (m_scanlineCounter < 241))
			{
				//std::cout << "." << m_dotCounter << "|" << std::endl;
				if ((m_dotCounter % 8) == 0)
				{
					//Dot count = X
					int tileX = ((m_dotCounter + (hScroll +8)) / 8);

					

					//Line count = Y
					int tileY = ((m_scanlineCounter + vScroll) / 8);
					int lineOffset = (m_scanlineCounter + vScroll) % 8;
					//Adjust for mirroring. in Horizontal X gets % 32
					//in Vertical, Y gets  % 30
					int pitch;
					if (m_mirroring)
					{
						tileY = tileY % 30;

						pitch = 32;
					
					}
					else
					{
						tileX = (tileX % 32);
						pitch = 32;
					}
					

					uint16_t yAdjustHack =  tileY - (tileX > 31 ? 1 : 0);
					uint16_t tileAddr = (yAdjustHack * pitch) + tileX;
					uint16_t adjust = ((tileX > 31) ? nextTableBase : base) + tileAddr;

					int nt = getNameTable(adjust);
					uint8_t tileRef = m_nameTable[(uint16_t) tileAddr + ((uint16_t)nt * 0x400)];

					uint16_t patternAddr = (readbit(m_registers[0], 4) ? 0x1000 : 0x0000) | (tileRef << 4) | (lineOffset & 0x0007);
					

					//Reversed because the most significant bit is the *right* side of the tile (OLC video)
					uint8_t tileByte = reverseBits(m_read(patternAddr)); //But why
					uint8_t tileByte2 = reverseBits(m_read(patternAddr + 8)); //But why tho?
					m_bgShiftRegs[0] = (m_bgShiftRegs[0] & 0x00FF) | (tileByte << 8);
					m_bgShiftRegs[1] = (m_bgShiftRegs[1] & 0x00FF) | (tileByte2 << 8);

					//Load the attribute regs.
					//This code is AFU. Scrolling immediately breaks everything. 
					
					uint8_t attrX = (m_dotCounter ) / 32;
					uint8_t attrY = (m_scanlineCounter + vScroll) / 32;
//					if (attrX > 7) std::cout << "WOOPS X: " << (uint16_t) attrX << std::endl;
//					if (attrY > 7) std::cout << "WOOPS Y: " << (uint16_t) attrY << std::endl;
					//This is such an ugly kludge. 
					//if attrX is greater than 7 it means we're in the next
					//attribute table over... 
					int ntOffset = 0; 
					
					
					int coarseX = (m_dotCounter) / 4;
					int coarseY = (m_scanlineCounter ) / 4;
					//uint16_t attrAddr = (nt << 10) | 0x3c0 | (coarseY << 3) | coarseX;
					uint16_t attrAddr = ((nt + ntOffset) * 0x0400) + 0x03C0 + (attrY * 8) + attrX;
					
					uint8_t attrByte = m_nameTable[attrAddr];
					
					/*
					int coarseX = (m_dotCounter + hScroll) / 4;
					int coarseY = (m_scanlineCounter + vScroll) / 4;
					uint16_t attrAddr = (nt << 12) | 0x3c0 | (coarseY << 3) | coarseX;
					uint8_t quadX = ((m_dotCounter + hScroll) / 16) % 2;
					uint8_t quadY = ((m_scanlineCounter + vScroll) / 16) % 2;
					uint8_t quadrant = ((quadY << 1) | quadX) * 2;
					uint8_t attrByte = m_nameTable[attrAddr];
					*/
					int offset = 0; 
					if (((m_scanlineCounter + vScroll) % 32) > 15)
					{
						offset += 4; 
					}
					m_attrShiftRegs[0] = (readbit(attrByte, 0+offset) * 0x0F00) | (m_attrShiftRegs[0] & 0xF0FF);
					m_attrShiftRegs[0] = (readbit(attrByte, 2+offset) * 0xF000) | (m_attrShiftRegs[0] & 0x0FFF);
					m_attrShiftRegs[1] = (readbit(attrByte, 1+offset) * 0x0F00) | (m_attrShiftRegs[1] & 0xF0FF);
					m_attrShiftRegs[1] = (readbit(attrByte, 3+offset) * 0xF000) | (m_attrShiftRegs[1] & 0x0FFF);
					

				}
				//we're live... Render next pixel...
				if (m_dotCounter < 256)
					renderNextPixel();
			}
			//Shift down.
			m_bgShiftRegs[0] = m_bgShiftRegs[0] >> 1;
			m_bgShiftRegs[1] = m_bgShiftRegs[1] >> 1;
			m_attrShiftRegs[0] =  m_attrShiftRegs[0] >> 1;
			m_attrShiftRegs[1] =  m_attrShiftRegs[1] >> 1;
			//if (m_sprCount > 7) m_sprCount = 7;
			for (size_t s = 0; s < m_sprCount; s++)
			{
				//Check our sprite X counters...
				if (m_spriteX[s] > 0)
				{
					m_spriteX[s]--;

				}
				else
				{
					if (m_spriteX[s] > -8)
					{
						m_spriteActive[s] = true;
						if (m_spriteX[s] != 0) //Cheezy hack. Don't want to shift before the first pixel is read.
						{
							m_spriteShiftHi[s] = m_spriteShiftHi[s] >> 1;
							m_spriteShiftLo[s] = m_spriteShiftLo[s] >> 1;
						}
						m_spriteX[s]--;
					}
					else
					{
						m_spriteActive[s] = false;
					}
				}
			}

			m_dotCounter++;
		}
	}

	void PPU::renderNextPixel()
	{
		
		//Read from the pattern table to get the pixel color.
		//Adjust with attribute table.
		//Check for any sprites on this pixel...
		uint8_t fineScroll = (hScroll & 0x07);
		uint8_t fineYScroll = (vScroll & 0x07);

		
		//Place the pixel in our current backbuffer...
		int bgColorIndex = 
						  (readbit(m_attrShiftRegs[0], fineScroll) << 3)
						| (readbit(m_attrShiftRegs[1], fineScroll) << 2)
						| (readbit(m_bgShiftRegs[1], (fineScroll)) << 1) 
						| readbit(m_bgShiftRegs[0], (fineScroll));	

		int sprColorIndex = 0x00;
		bool sprPri = true;
		for (int s = 7; s >= 0; --s)
		{
			if (!m_spriteActive[s]) continue;
			uint8_t sprAttr = (m_oam[(m_spriteScanIdx[s] * 4) + 2]) & 0x3;
			int thisSprCol =  (sprAttr << 2) | (readbit(m_spriteShiftLo[s], 0) << 1) | (readbit(m_spriteShiftHi[s], 0));
			if (thisSprCol) sprColorIndex = thisSprCol;
			if ((m_spriteScanIdx[s] == 0) && ((sprColorIndex != 0)))// && ((bgColorIndex & 0x3) != 0)))
			{
				writebit(m_registers[2], 1, 6);
			}
			if(sprColorIndex != 0)
			sprPri = readbit(m_oam[m_spriteScanIdx[s]*4 + 2], 5);
			break;
			/*
			if ((!sprPri && (sprColorIndex != 0)) || (bgColorIndex == 0))
			{
				//Sprite has priority. it's in the foreground
				sprColorIndex = (sprColorIndex & 0x0F) | 0x10;

			}
			else
			{
				//Sprite has no priority. it's in the background
				sprColorIndex = bgColorIndex;
			}
			*/
			
		}


		int finalColorIndex = bgColorIndex; 
		
		if ((!sprPri && (sprColorIndex != 0)) || (bgColorIndex == 0) && ((m_dotCounter > 64) && (m_dotCounter < 241)))
		{
			//Sprite has priority. it's in the foreground
			finalColorIndex = (sprColorIndex & 0x0F) | 0x10;
	
		}
	
	
		
		RGB pixelColor;
		pixelColor = nes_palette[m_paletteRAM[finalColorIndex]];

		m_backBuffer[3 * ((m_scanlineCounter * 256) + m_dotCounter)] = pixelColor.r * 4;
		m_backBuffer[3 * ((m_scanlineCounter * 256) + m_dotCounter) + 1] = pixelColor.g * 4;
		m_backBuffer[3 * ((m_scanlineCounter * 256) + m_dotCounter) + 2] = pixelColor.b * 4;
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
