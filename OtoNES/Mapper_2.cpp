#include "Mapper_2.h"
#include <iostream>
uint8_t nes::mappers::Mapper_2::readMapper(uint16_t addr)
{
	if (addr >= 0xC000)
	{

		return m_progBanks.back()[(addr - 0xC000)];
	}
	else
	{
		return m_progBanks[m_bankNum][addr - 0x8000];
	}
}

void nes::mappers::Mapper_2::writeMapper(uint16_t addr, uint8_t byte)
{
	
	if (addr >= 0x8000)
	{
		//Bank select!
		m_bankNum = byte & 0x07; 
	//	std::cout << "BANKSEL: " << std::hex <<  (uint16_t) m_bankNum << std::dec << std::endl;
		//std::cout << "MMC2: WRITE: " << std::hex << addr << std::dec << std::endl;
	}
}

uint8_t nes::mappers::Mapper_2::ppuRead(uint16_t addr)
{
	//std::cout << "MMC2: PPU READ: " << std::hex << addr << std::endl;
	return m_charBanks[addr];
}

void nes::mappers::Mapper_2::ppuWrite(uint16_t addr, uint8_t byte)
{
	m_charBanks[addr] = byte;
}
