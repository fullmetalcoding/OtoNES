#include "NesMachine.h"

#include <functional>

//Internal ram writes are 0000-1FFF, mirrored every 0x800 bytes.
//$2000-$2007 are PPU regs
//$2008-$3FFF are mirrored PPU regs every 8 bytes
//$4000-$4017 are APU and I/O functionality
//$4020-$FFFF is cart space. (PRGROM/PRGRAM)

//Space selection:
//If bit 14 or higher is not set, we are in internal ram.
//If bit 14 is set, we are in PPU land.
//If bit 15 is set, we are in APU or progland
//So for memory mapping, grab bit 13 and 14 of the address
//If 00, RAM write. If 01, PPU write, else if 11 or 10, upper address write.
//Further decode from there.
namespace nes
{
	using namespace std::placeholders;
	void NesMachine::writeMem(uint16_t address, uint8_t byte)
	{
		if ((address >= 0x4000) && (address < 0x4020))
		{
			if (address == 0x4014)
			{

				uint16_t dmaAddr = (byte << 8) & 0xFF00;
				//std::cout << "OAM DMA ACCESS: " << std::hex << dmaAddr << std::dec << std::endl;
				for (size_t i = 0; i < 0xFF; i++)
				{
					m_ppu->dmaPage(i, readMem(dmaAddr + i));
				}
				
			}
			return;
		}
		uint8_t sectionWriteMask = (address & 0xE000) >> 13; //Should put us in the range of 0-6. 
	
		m_sectionWriteMap[sectionWriteMask](address, byte);

	}
	
	NesMachine::NesMachine(std::shared_ptr<mappers::IMapper> cartMap) :
			m_bankMapper(cartMap)
	{
		//@TODO: Clean up the handler map. Man I must've been tired writing this. I thought I needed 3 bits instead of two...
		//Also note from self: 101b and 001b are not unique with two bits, so if you're going to try and be clever with
		//2 bits you have to take that into account. 
		/* 
		The 6502 core tries to read from memory the moment it is created through its constructor, 
		so set up the write/read handlers before instantiating the CPU.
		*/
		m_internalRAM.reset(new uint8_t[0x800]);
		m_sectionWriteMap[0x00] = std::bind(&NesMachine::writeInternalRAM, this, _1, _2);
		m_sectionWriteMap[0x02] = std::bind(&mappers::IMapper::writeMapper, m_bankMapper, _1, _2);
		m_sectionWriteMap[0x03] = std::bind(&mappers::IMapper::writeMapper, m_bankMapper, _1, _2);
		m_sectionWriteMap[0x04] = std::bind(&mappers::IMapper::writeMapper, m_bankMapper, _1, _2);
		m_sectionWriteMap[0x05] = std::bind(&mappers::IMapper::writeMapper, m_bankMapper, _1, _2);
		m_sectionWriteMap[0x06] = std::bind(&mappers::IMapper::writeMapper, m_bankMapper, _1, _2);
		m_sectionWriteMap[0x07] = std::bind(&mappers::IMapper::writeMapper, m_bankMapper, _1, _2);

		m_sectionReadMap[0x00] = std::bind(&NesMachine::readInternalRAM, this, _1);
		m_sectionReadMap[0x02] = std::bind(&mappers::IMapper::readMapper, m_bankMapper, _1);
		m_sectionReadMap[0x03] = std::bind(&mappers::IMapper::readMapper, m_bankMapper, _1);
		m_sectionReadMap[0x04] = std::bind(&mappers::IMapper::readMapper, m_bankMapper, _1);
		m_sectionReadMap[0x05] = std::bind(&mappers::IMapper::readMapper, m_bankMapper, _1);
		m_sectionReadMap[0x06] = std::bind(&mappers::IMapper::readMapper, m_bankMapper, _1);
		m_sectionReadMap[0x07] = std::bind(&mappers::IMapper::readMapper, m_bankMapper, _1);

		m_cpu.reset(new mos6502(std::bind(&NesMachine::readMem, this, std::placeholders::_1),
			std::bind(&NesMachine::writeMem, this, std::placeholders::_1, std::placeholders::_2)));

		/*
		However, the PPU handlers can't be instantiated until the PPU is,
		which requires the CPU for the NMI callback.
		Theoretically I could fix this coding quirk by just moving the 
		NMI callback to a setter (and not in the PPU constructor), but
		meh... It's not really any cleaner just to have the map handlers
		all set up in the same place...
		*/
		//@TODO: Replace nullptrs below with mapper access for PPU CHR ROM.
		m_ppu.reset(new PPU(std::bind(&mos6502::NMI, m_cpu),
			std::bind(&mappers::IMapper::ppuRead, m_bankMapper, _1),
			nullptr));
		m_ppu->setMirroring(m_bankMapper->getMirroring());
		m_sectionWriteMap[0x01] = std::bind(&PPU::writePPU, m_ppu, _1, _2);
		m_sectionReadMap[0x01] = std::bind(&PPU::readPPU, m_ppu,  _1);

	}

	void NesMachine::advanceCycles(int cycles)
	{

		for (size_t i = 0; i < cycles; i++)
		{
			m_cpu->Run(1);
			m_ppu->advanceCycles(3);
		}
	}

	uint8_t NesMachine::readMem(uint16_t address)
	{
		//std::cout << "READ: " << std::hex << address << std::dec << std::endl;
		uint8_t sectionReadMask = (address & 0xE000) >> 13; //Should put us in the range of 0-6. 
		return m_sectionReadMap[sectionReadMask](address);
	}

	uint8_t NesMachine::readInternalRAM(uint16_t address)
	{
		uint16_t maskedAddr = (address & 0x1FFF) % 0x800;
		return m_internalRAM[maskedAddr];
	}

	void NesMachine::writeInternalRAM(uint16_t address, uint8_t byte)
	{
		uint16_t maskedAddr = (address & 0x1FFF) % 0x800;
	//	std::cout << std::hex << maskedAddr << " : " << (int)byte << std::dec << std::endl;
		m_internalRAM[maskedAddr] = byte;
	}
}