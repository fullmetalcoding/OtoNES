#pragma once
#include "IMapper.h"
#include <memory>
#include <functional>
#include <fstream>
namespace nes
{
	class RomLoader
	{
	private:
		std::shared_ptr<mappers::IMapper> loadMapper1(uint8_t* temph, uint8_t* header);
		std::shared_ptr<mappers::IMapper> loadMapper0(uint8_t* temph, uint8_t* header);
               std::shared_ptr<mappers::IMapper> loadMapper2(uint8_t* temph, uint8_t* header);
               std::shared_ptr<mappers::IMapper> loadMapper4(uint8_t* temph, uint8_t* header);

	public:
		std::shared_ptr<mappers::IMapper> loadRom(std::string filename);
	};
}