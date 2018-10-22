#pragma once
#include <stdint.h>

namespace nes
{
	namespace mappers
	{
		class IMapper
		{
		protected:
			bool m_mirroring; 

		public:
			void setMirroring(bool mir)
			{
				m_mirroring = mir;
			}
			bool getMirroring()
			{
				return m_mirroring;
			}
			virtual void writeMapper(uint16_t addr, uint8_t byte) abstract;
			virtual uint8_t readMapper(uint16_t addr) abstract;

			virtual uint8_t ppuRead(uint16_t addr) abstract;
		};
	}
}