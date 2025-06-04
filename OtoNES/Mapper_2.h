#pragma once

#include "IMapper.h"
#include <memory>
#include <vector>

namespace nes {

	class RomLoader;
	namespace mappers {
	
		class Mapper_2 : public IMapper
		{
		private:
			std::vector<std::shared_ptr<uint8_t[]> > m_progBanks;
			std::shared_ptr<uint8_t[]>  m_charBanks;//char pointer to char banks.... char char banks?
			uint8_t m_bankNum;
			friend class RomLoader;

		public:
			Mapper_2()
			{
				m_bankNum = 0;
				m_charBanks.reset(new uint8_t[0x2000]); //Create CHR-RAM
			}
			uint8_t readMapper(uint16_t addr) override;
			void writeMapper(uint16_t addr, uint8_t byte) override;
			uint8_t ppuRead(uint16_t addr) override;
			void ppuWrite(uint16_t addr, uint8_t byte) override;
		};
	}
}