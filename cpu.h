//extern unsigned char mem[];

#include <stdint.h>

extern unsigned char* mem;
extern unsigned int cpucycles;
extern uint16_t register_pc;
void initcpu(unsigned short newpc, unsigned char newa, unsigned char newx, unsigned char newy);
int runcpu(void);
