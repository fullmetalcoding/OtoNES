#include "Mapper_4.h"
#include <iostream>

using namespace nes::mappers;

Mapper_4::Mapper_4()
{
        m_bankSelect = 0;
        for(int i = 0; i < 8; i++) m_bankRegs[i] = 0;
        m_prgMode = false;
        m_chrMode = false;
        m_chrRam = false;
}

uint8_t Mapper_4::readMapper(uint16_t addr)
{
        if(addr >= 0x8000)
        {
                size_t bank = 0;
                uint16_t offset = addr & 0x1FFF;
                size_t lastBank = m_progBanks.size() - 1;
                switch(addr & 0xE000)
                {
                case 0x8000:
                        bank = m_prgMode ? lastBank - 1 : m_bankRegs[6] % m_progBanks.size();
                        break;
                case 0xA000:
                        bank = m_bankRegs[7] % m_progBanks.size();
                        break;
                case 0xC000:
                        bank = m_prgMode ? m_bankRegs[6] % m_progBanks.size() : lastBank - 1;
                        break;
                case 0xE000:
                        bank = lastBank;
                        break;
                }
                return m_progBanks[bank][offset];
        }
        return 0;
}

void Mapper_4::writeMapper(uint16_t addr, uint8_t byte)
{
        if(addr >= 0x8000 && addr <= 0x9FFF)
        {
                if((addr & 1) == 0)
                {
                        m_bankSelect = byte & 0x07;
                        m_prgMode = (byte & 0x40) != 0;
                        m_chrMode = (byte & 0x80) != 0;
                }
                else
                {
                        m_bankRegs[m_bankSelect & 0x07] = byte;
                }
        }
        else if(addr >= 0xA000 && addr <= 0xBFFF)
        {
                if((addr & 1) == 0)
                {
                        m_mirroring = byte & 0x01;
                }
        }
}

static inline size_t chrBankIndex(bool mode, const uint8_t regs[8], uint16_t addr, uint16_t &offset)
{
        if(!mode)
        {
                if(addr < 0x0400) { offset = addr; return (regs[0] & 0xFE); }
                if(addr < 0x0800) { offset = addr - 0x0400; return (regs[0] & 0xFE) + 1; }
                if(addr < 0x0C00) { offset = addr - 0x0800; return (regs[1] & 0xFE); }
                if(addr < 0x1000) { offset = addr - 0x0C00; return (regs[1] & 0xFE) + 1; }
                if(addr < 0x1400) { offset = addr - 0x1000; return regs[2]; }
                if(addr < 0x1800) { offset = addr - 0x1400; return regs[3]; }
                if(addr < 0x1C00) { offset = addr - 0x1800; return regs[4]; }
                offset = addr - 0x1C00; return regs[5];
        }
        else
        {
                if(addr < 0x0400) { offset = addr; return regs[2]; }
                if(addr < 0x0800) { offset = addr - 0x0400; return regs[3]; }
                if(addr < 0x0C00) { offset = addr - 0x0800; return regs[4]; }
                if(addr < 0x1000) { offset = addr - 0x0C00; return regs[5]; }
                if(addr < 0x1400) { offset = addr - 0x1000; return (regs[0] & 0xFE); }
                if(addr < 0x1800) { offset = addr - 0x1400; return (regs[0] & 0xFE) + 1; }
                if(addr < 0x1C00) { offset = addr - 0x1800; return (regs[1] & 0xFE); }
                offset = addr - 0x1C00; return (regs[1] & 0xFE) + 1;
        }
}

uint8_t Mapper_4::ppuRead(uint16_t addr)
{
        uint16_t offset = 0;
        size_t bank = chrBankIndex(m_chrMode, m_bankRegs, addr, offset) % m_chrBanks.size();
        return m_chrBanks[bank][offset];
}

void Mapper_4::ppuWrite(uint16_t addr, uint8_t byte)
{
        if(!m_chrRam) return; //ignore if ROM
        uint16_t offset = 0;
        size_t bank = chrBankIndex(m_chrMode, m_bankRegs, addr, offset) % m_chrBanks.size();
        m_chrBanks[bank][offset] = byte;
}
