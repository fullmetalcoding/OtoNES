#pragma once

#include "IMapper.h"
#include <memory>
#include <vector>

namespace nes {
        namespace mappers {
                class Mapper_4 : public IMapper
                {
                private:
                        std::vector<std::shared_ptr<uint8_t[]>> m_progBanks; //8KB banks
                        std::vector<std::shared_ptr<uint8_t[]>> m_chrBanks;  //1KB banks
                        uint8_t m_bankSelect;
                        uint8_t m_bankRegs[8];
                        bool m_prgMode;
                        bool m_chrMode;
                        bool m_chrRam;
                        friend class RomLoader;
                public:
                        Mapper_4();
                        uint8_t readMapper(uint16_t addr) override;
                        void writeMapper(uint16_t addr, uint8_t byte) override;
                        uint8_t ppuRead(uint16_t addr) override;
                        void ppuWrite(uint16_t addr, uint8_t byte) override;
                };
        }
}
