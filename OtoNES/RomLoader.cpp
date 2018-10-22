#include "RomLoader.h"

#include "Mapper_0.h"

#include <iostream>
#include <fstream>
#include "bitmanip.h"
namespace nes
{
	std::shared_ptr<mappers::IMapper> RomLoader::loadMapper0(uint8_t* temph, uint8_t * header)
	{
		std::shared_ptr<mappers::IMapper> retMapper(new mappers::Mapper_0);
		
		std::shared_ptr<uint8_t[]> progBanks(new uint8_t[header[4] * 0x4000]);
		std::shared_ptr<uint8_t[]> charBanks(new uint8_t[header[5] * 0x2000]);
		std::static_pointer_cast<mappers::Mapper_0>(retMapper)->setProgBanks(progBanks);
		std::static_pointer_cast<mappers::Mapper_0>(retMapper)->setCharBanks(charBanks);
		retMapper->setMirroring(readbit(header[6], 0));
		std::cout << "Mirroring: " << (retMapper->getMirroring() ? "Vertical" : "Horizontal") << std::endl;
		for (size_t i = 0; i < header[4] * 0x4000; i++)
		{
			progBanks[i] = temph[i];
		}
		if (header[4] == 1)
		{
			std::static_pointer_cast<mappers::Mapper_0>(retMapper)->set16kMirror(true);
		}
		else
		{
			std::static_pointer_cast<mappers::Mapper_0>(retMapper)->set16kMirror(false);
		}

		for (size_t i = 0; i < header[5] * 0x2000; i++)
		{
			charBanks[i] = temph[i + (0x4000 * header[4])];
		}

		return retMapper;
		
	}
	std::shared_ptr<mappers::IMapper> RomLoader::loadRom(std::string filename)
	{
		std::shared_ptr<mappers::IMapper> retMapper;
		std::ifstream in;

		std::cout << "Attempt to open: " << filename.c_str() << std::endl;

		uint8_t *header = new uint8_t[16];
		in.open(filename.c_str(), std::ios::binary);
		if (!in.good())
		{
			std::cerr << "BAD LOAD." << std::endl;
		}
		int length;
		in.seekg(0, std::ios::end);
		length = in.tellg();
		in.seekg(0, std::ios::beg);
		std::cout << "len: " << std::dec << length << std::endl;


		if (!in.is_open()) {
			delete header;

			return nullptr; // File not found or otherwise unopenable
		}
		in.seekg(0, std::ios::beg);
		in.read((char*)header, 16);

		uint8_t *temph = new uint8_t[length - 16];
		in.read((char*)temph, (static_cast<unsigned int>(length - 16)));
		if (in.eof())
		{
			std::cerr << "hit eof prematurely!\n";
			delete header;
			delete temph;
			return nullptr;
		}
		if (in.fail())
		{
			std::cout << "failed somehow\n";
			delete header;
			delete temph;
			return nullptr;
		}

		std::cout << "Read: " << in.gcount() << " bytes" << std::endl;
		std::cout << "Number of program roms: " << std::dec << static_cast<unsigned int>(header[4]) << std::endl;
		std::cout << "Number of vrom: " << std::dec << static_cast<unsigned int>(header[5]) << std::endl;
		int mappernum = ((header[6] & 0xF0) >> 4);
		std::cout << "Mapper: " << mappernum <<std::endl;

		std::shared_ptr<mappers::IMapper> retMap;
		switch (mappernum)
		{
		case 0:
			retMap = loadMapper0(temph, header);
			break;
		default:
			//UNSUPPORTED MAPPER
			std::cerr << "UNSUPPORTED MAPPER: " << mappernum << std::endl;
			
		}
		delete header;
		delete temph;
		return retMap;
	}
}
