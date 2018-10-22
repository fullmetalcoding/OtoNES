#pragma once

#include <iostream>

namespace DEBUGPLACEHOLDER
{
	void PPUWRITE(uint16_t addr, uint8_t byte)
	{
		std::cout << "PPU WRITE [" << std::hex <<addr << "]: " << (int)byte << std::dec << std::endl;
	}
	uint8_t PPUREAD(uint16_t addr)
	{
		std::cout << "PPU READ [" << addr << "]: " << std::hex << addr << std::dec << std::endl;
		return 0x00;
	}
}