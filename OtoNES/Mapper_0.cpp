#include "Mapper_0.h"

#include <iostream>
namespace nes
{
	namespace mappers
	{
		uint8_t Mapper_0::readMapper(uint16_t addr)
		{
			//std::cout << "READ: " <<std::hex <<addr <<std::dec << std::endl;
			uint16_t maskedAddr = m_16kMirror ? (addr & 0x3FFF) : (addr & 0x7FFF);
			//std::cout << " [" << std::hex << (int)m_progBanks[maskedAddr] << "] " << std::dec << std::endl;
			return m_progBanks[maskedAddr];
		}
		void Mapper_0::writeMapper(uint16_t addr, uint8_t byte)
		{
			std::cout << "WRITE: " << std::hex << addr  << " : " << (int)byte << std::dec <<std::endl;
			uint16_t maskedAddr = m_16kMirror ? (addr & 0xC000) : (addr & 0x8000);
			m_progBanks[maskedAddr] = byte;
		}
		uint8_t Mapper_0::ppuRead(uint16_t addr)
		{
			//std::cout << std::hex << addr << std::dec << std::endl;
			//Determine	which CHR bank we're in...
			//For mapper 0, it doesn't really matter since we only have 2 pattern tables.
			return m_charBanks[addr];
		}
	}
}
