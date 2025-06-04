#include "Mapper_1.h"

uint8_t nes::mappers::Mapper_1::readMapper(uint16_t addr)
{
	switch (getPrgBankMode())
	{
	case 0:
	case 1:
		//switch 32 KB at $8000, ignoring low bit of bank number;
		if (addr < 0xC000)
		{
			return m_progBanks[m_prgBankSel][addr - 0x8000];
		}
		else
		{
			return m_progBanks[m_prgBankSel + 1][addr - 0xC000];
		}
		break;
	case 2:
		//fix first bank at $8000 and switch 16 KB bank at $C000;
		if (addr < 0xC000)
		{
			return (m_progBanks.front())[addr - 0x8000];
		}
		else
		{
			return m_progBanks[m_prgBankSel][addr - 0xC000];
		}
		break;
	case 3:
		//fix last bank at $C000 and switch 16 KB bank at $8000)
		if (addr >= 0xC000)
		{
			return m_progBanks.back()[addr - 0xC000];
		}
		else
		{
			return m_progBanks[m_prgBankSel][addr - 0x8000];
		}
	default:
		break;

	}
	return uint8_t();
}

uint8_t nes::mappers::Mapper_1::getPrgBankMode()
{
	uint8_t prgBankMode = (m_ctrlReg & 0x0C) >> 2;
	return prgBankMode;
}
void nes::mappers::Mapper_1::writeMapper(uint16_t addr, uint8_t byte)
{
	//First check if we're resetting the shift register...
	if ((byte & 0x80) != 0) {
		//RESET.
		m_shiftReg = 0; 
		m_shiftCount = 0;
		return;
	}
	uint8_t bit = byte & 0x1;
	m_shiftReg = m_shiftReg << 1; 
	m_shiftReg = m_shiftReg | bit;
	m_shiftCount++; 
	if (m_shiftCount == 5)
	{
		//LATCH!
		//Last address write controls where the shift reg goes...
		if ((addr >= 0x8000) && (addr <= 0x9FFF))
		{
			m_ctrlReg = m_shiftReg;		
		}
		else if ((addr >= 0xA000) && (addr <= 0xBFFF))
		{
			//CHR0 select.
			m_chrLoBankSel = m_shiftReg;
		}
		else if ((addr >= 0xC000) && (addr <= 0xDFFF))
		{
			//CHR1 select.
			m_chrHiBankSel = m_shiftReg;
		}
		else if ((addr >= 0xE000) && (addr <= 0xFFFF))
		{
			uint8_t mask = 0x0F;
			uint8_t prgBankMode = getPrgBankMode();
			if (prgBankMode < 2)
			{
				mask = mask & 0xE;
			}
			m_prgBankSel = m_shiftReg & mask;
		}
		m_shiftCount = 0; 
		m_shiftReg = 0; 

	}

}

uint8_t nes::mappers::Mapper_1::ppuRead(uint16_t addr)
{
	//std::cout << "MMC1 PPU READ: " << std::hex << addr << std::dec << std::endl;
	if ((m_ctrlReg & 0x10) != 0)
	{
		//Switch separate 4KB banks
		if (addr < 0x1000)
		{
			//lobank
			return m_progBanks[m_chrLoBankSel][addr];
		}
		else
		{
			//hibank
			return m_progBanks[m_chrLoBankSel][addr - 0x1000];
		}
	}
	else
	{
		uint8_t maskedBankSel = m_chrLoBankSel & 0x1E;
		//Switch 8KB at a time
		if (addr < 0x1000)
		{
			//lobanksel 
			return m_progBanks[maskedBankSel][addr];
		}
		else
		{
			//lobanksel + 1
			return m_progBanks[maskedBankSel + 1][addr - 0x1000];
		}
	}
	return 0;
}
