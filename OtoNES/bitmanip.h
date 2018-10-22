#pragma once
#define writebit(var, bit, pos) {clearbit(var, pos); var |= (bit << pos);}
#define clearbit(var, pos) var &= ~(1 << pos)
#define readbit(x,y) (((x)>>(y))&0x01)
