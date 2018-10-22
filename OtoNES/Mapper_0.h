#pragma once


#include "IMapper.h"
#include <memory>

namespace nes
{
	namespace mappers
	{
		class Mapper_0 : public IMapper
		{
		private:
			std::shared_ptr<uint8_t[]> m_progBanks;
			std::shared_ptr<uint8_t[]> m_charBanks;//char pointer to char banks.... char char banks?
			bool m_16kMirror;

		public:
			void set16kMirror(bool mir)
			{
				m_16kMirror = mir;
			}
			void setProgBanks(std::shared_ptr<uint8_t[]> banks)
			{
				m_progBanks = banks;
			}
			void setCharBanks(std::shared_ptr<uint8_t[]> banks)
			{
				m_charBanks = banks; 
			}
			uint8_t readMapper(uint16_t addr) override;
			void writeMapper(uint16_t addr, uint8_t byte) override;
			uint8_t ppuRead(uint16_t addr) override;
		};
	}
}