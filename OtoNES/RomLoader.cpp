#include "RomLoader.h"

#include "Mapper_0.h"
#include "Mapper_1.h"
#include "Mapper_2.h"
#include "Mapper_4.h"


#include <iostream>
#include <fstream>
#include "bitmanip.h"

namespace nes
{
	std::shared_ptr<mappers::IMapper> RomLoader::loadMapper1(uint8_t* temph, uint8_t* header)
	{
		std::cout << "Loading mapper 1!" << std::endl;
		std::shared_ptr<mappers::IMapper> retMapper(new mappers::Mapper_1);
		retMapper->setMirroring(readbit(header[6], 0));
		std::cout << "Mirroring: " << (retMapper->getMirroring() ? "Vertical" : "Horizontal") << std::endl;
		
		std::vector<std::shared_ptr<uint8_t[]> > progBanks;
		for (size_t i = 0; i < header[4]; i++)
		{
			std::shared_ptr<uint8_t[]> bank(new uint8_t[0x4000]);
			for (size_t j = 0; j < 0x4000; j++)
			{
				bank[j] = temph[i * 0x4000 + j];
			}
			progBanks.push_back(bank);

		}
		std::cout << "Loaded: " << progBanks.size() << " 16KB banks." << std::endl;
		//Load CHR rom....
		std::vector<std::shared_ptr<uint8_t[]> > chrBanks;
		for (size_t i = 0; i < header[5]; i++)
		{
			std::shared_ptr<uint8_t[]> bank(new uint8_t[0x2000]);
			for (size_t j = 0; j < 0x2000; j++)
			{
				bank[j] = temph[j + ( 0x2000 * i) + 0x4000 * header[4]];
			
			}
			chrBanks.push_back(bank);
		}


		
		std::static_pointer_cast<mappers::Mapper_1>(retMapper)->m_progBanks = progBanks;
		std::static_pointer_cast<mappers::Mapper_1>(retMapper)->m_charBanks = chrBanks;
		return retMapper;
	}
std::shared_ptr<mappers::IMapper> RomLoader::loadMapper2(uint8_t* temph, uint8_t* header)
{
		std::cout << "Loading mapper 2!" << std::endl;
		std::shared_ptr<mappers::IMapper> retMapper(new mappers::Mapper_2);
		retMapper->setMirroring(readbit(header[6], 0));
		std::cout << "Mirroring: " << (retMapper->getMirroring() ? "Vertical" : "Horizontal") << std::endl;

		std::vector<std::shared_ptr<uint8_t[]> > progBanks;
		for (size_t i = 0; i < header[4]; i++)
		{
			std::shared_ptr<uint8_t[]> bank(new uint8_t[0x4000]);
			for (size_t j = 0; j < 0x4000; j++)
			{
				bank[j] = temph[i * 0x4000 + j];
			}
			progBanks.push_back(bank);

		}
		std::cout << "Loaded: " << progBanks.size() << " 16KB banks." << std::endl;

		std::static_pointer_cast<mappers::Mapper_2>(retMapper)->m_progBanks = progBanks;
		return retMapper;
	}
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
               case 1:
                       retMap = loadMapper1(temph, header);
                       break;
               case 2:
                       retMap = loadMapper2(temph, header);
                       break;
               case 4:
                        retMap = loadMapper4(temph, header);
                        break;
               default:
			//UNSUPPORTED MAPPER
			std::cerr << "UNSUPPORTED MAPPER: " << mappernum << std::endl;
			
		}
		delete header;
		delete temph;
		return retMap;
	}


	std::shared_ptr<mappers::IMapper> RomLoader::loadMapper4(uint8_t* temph, uint8_t* header)
	{
		std::cout << "Loading mapper 4!" << std::endl;
		std::shared_ptr<mappers::IMapper> retMapper(new mappers::Mapper_4);
		retMapper->setMirroring(readbit(header[6], 0));
		std::cout << "Mirroring: " << (retMapper->getMirroring() ? "Vertical" : "Horizontal") << std::endl;

		std::vector<std::shared_ptr<uint8_t[]>> progBanks;
		int num8k = header[4] * 2;
		for (int i = 0; i < num8k; ++i)
		{
			std::shared_ptr<uint8_t[]> bank(new uint8_t[0x2000]);
			for (int j = 0; j < 0x2000; ++j)
			{
				bank[j] = temph[i * 0x2000 + j];
			}
			progBanks.push_back(bank);
		}
		std::cout << "Loaded: " << progBanks.size() << " 8KB banks." << std::endl;

		std::vector<std::shared_ptr<uint8_t[]>> chrBanks;
		if (header[5] == 0)
		{
			for (int i = 0; i < 8; ++i)
			{
				std::shared_ptr<uint8_t[]> bank(new uint8_t[0x400]);
				for (int j = 0; j < 0x400; ++j) bank[j] = 0;
				chrBanks.push_back(bank);
			}
			std::static_pointer_cast<mappers::Mapper_4>(retMapper)->m_chrRam = true;
		}
		else
		{
			for (int i = 0; i < header[5] * 8; ++i)
			{
				std::shared_ptr<uint8_t[]> bank(new uint8_t[0x400]);
				for (int j = 0; j < 0x400; ++j)
				{
					bank[j] = temph[header[4] * 0x4000 + i * 0x400 + j];
				}
				chrBanks.push_back(bank);
			}
			std::static_pointer_cast<mappers::Mapper_4>(retMapper)->m_chrRam = false;
		}

		std::static_pointer_cast<mappers::Mapper_4>(retMapper)->m_progBanks = progBanks;
		std::static_pointer_cast<mappers::Mapper_4>(retMapper)->m_chrBanks = chrBanks;
		return retMapper;
	}
}
