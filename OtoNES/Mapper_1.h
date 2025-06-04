#pragma once

#include "IMapper.h"
#include <memory>
#include <vector>

namespace nes {

	class RomLoader;
	namespace mappers {

		class Mapper_1 : public IMapper
		{
		private:
			std::vector<std::shared_ptr<uint8_t[]> > m_progBanks;
			std::vector<std::shared_ptr<uint8_t[]> > m_charBanks;//char pointer to char banks.... char char banks?
			uint8_t m_prgBankSel;
		

			uint8_t m_chrLoBankSel;
			uint8_t m_chrHiBankSel;
			uint8_t m_ctrlReg;

			uint8_t m_shiftReg;
			uint8_t m_shiftCount; 

			friend class RomLoader;

		public:
			Mapper_1() :
				m_prgBankSel(0),
				m_chrLoBankSel(0),
				m_chrHiBankSel(0),
				m_ctrlReg(0x06)
			{
				
				
			}
			uint8_t readMapper(uint16_t addr) override;
			uint8_t getPrgBankMode();
			void writeMapper(uint16_t addr, uint8_t byte) override;
			uint8_t ppuRead(uint16_t addr) override;
			
		};
	}
}