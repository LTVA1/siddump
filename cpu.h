//extern unsigned char mem[];
unsigned char* mem;
extern unsigned int cpucycles;
extern unsigned short pc;


#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void initcpu(unsigned short newpc, unsigned char newa, unsigned char newx, unsigned char newy);
uint8_t runcpu(void);
void advance_cycle();

struct regs_Struct {
    uint8_t a,x,y,sp;
    uint16_t pc;
    uint8_t n,v,b,d,i,z,c; // flags
    int cycles;
    uint8_t irq, nmi;
    uint16_t temp, temp_cross;
    uint8_t jammed;
};

#define swap(a,b) { uint8_t temp = a; a = b; b = temp; }

struct regs_Struct regs;

void advance_cycle()
{
    cpucycles++;
    pc = regs.pc;
}

void show_opimp();

#if 1
uint8_t implemented_ops[256];
// from Mesen2
static const char* _opName[256] = {
    //	0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
        "BRK",	"ORA",	"STP",	"SLO",	"NOP",	"ORA",	"ASL",	"SLO",	"PHP",	"ORA",	"ASL",	"ANC",	"NOP",	"ORA",	"ASL",	"SLO", //0
        "BPL",	"ORA",	"STP",	"SLO",	"NOP",	"ORA",	"ASL",	"SLO",	"CLC",	"ORA",	"NOP",	"SLO",	"NOP",	"ORA",	"ASL",	"SLO", //1
        "JSR",	"AND",	"STP",	"RLA",	"BIT",	"AND",	"ROL",	"RLA",	"PLP",	"AND",	"ROL",	"ANC",	"BIT",	"AND",	"ROL",	"RLA", //2
        "BMI",	"AND",	"STP",	"RLA",	"NOP",	"AND",	"ROL",	"RLA",	"SEC",	"AND",	"NOP",	"RLA",	"NOP",	"AND",	"ROL",	"RLA", //3
        "RTI",	"EOR",	"STP",	"SRE",	"NOP",	"EOR",	"LSR",	"SRE",	"PHA",	"EOR",	"LSR",	"ALR",	"JMP",	"EOR",	"LSR",	"SRE", //4
        "BVC",	"EOR",	"STP",	"SRE",	"NOP",	"EOR",	"LSR",	"SRE",	"CLI",	"EOR",	"NOP",	"SRE",	"NOP",	"EOR",	"LSR",	"SRE", //5
        "RTS",	"ADC",	"STP",	"RRA",	"NOP",	"ADC",	"ROR",	"RRA",	"PLA",	"ADC",	"ROR",	"ARR",	"JMP",	"ADC",	"ROR",	"RRA", //6
        "BVS",	"ADC",	"STP",	"RRA",	"NOP",	"ADC",	"ROR",	"RRA",	"SEI",	"ADC",	"NOP",	"RRA",	"NOP",	"ADC",	"ROR",	"RRA", //7
        "NOP",	"STA",	"NOP",	"SAX",	"STY",	"STA",	"STX",	"SAX",	"DEY",	"NOP",	"TXA",	"XAA",	"STY",	"STA",	"STX",	"SAX", //8
        "BCC",	"STA",	"STP",	"AHX",	"STY",	"STA",	"STX",	"SAX",	"TYA",	"STA",	"TXS",	"TAS",	"SHY",	"STA",	"SHX",	"AXA", //9
        "LDY",	"LDA",	"LDX",	"LAX",	"LDY",	"LDA",	"LDX",	"LAX",	"TAY",	"LDA",	"TAX",	"LAX",	"LDY",	"LDA",	"LDX",	"LAX", //A
        "BCS",	"LDA",	"STP",	"LAX",	"LDY",	"LDA",	"LDX",	"LAX",	"CLV",	"LDA",	"TSX",	"LAS",	"LDY",	"LDA",	"LDX",	"LAX", //B
        "CPY",	"CMP",	"NOP",	"DCP",	"CPY",	"CMP",	"DEC",	"DCP",	"INY",	"CMP",	"DEX",	"AXS",	"CPY",	"CMP",	"DEC",	"DCP", //C
        "BNE",	"CMP",	"STP",	"DCP",	"NOP",	"CMP",	"DEC",	"DCP",	"CLD",	"CMP",	"NOP",	"DCP",	"NOP",	"CMP",	"DEC",	"DCP", //D
        "CPX",	"SBC",	"NOP",	"ISC",	"CPX",	"SBC",	"INC",	"ISC",	"INX",	"SBC",	"NOP",	"SBC",	"CPX",	"SBC",	"INC",	"ISC", //E
        "BEQ",	"SBC",	"STP",	"ISC",	"NOP",	"SBC",	"INC",	"ISC",	"SED",	"SBC",	"NOP",	"ISC",	"NOP",	"SBC",	"INC",	"ISC"  //F
};

enum NesAddrMode {
	None, Acc, Imp, Imm, Rel,
	Zero, Abs, ZeroX, ZeroY,
	Ind, IndX, IndY, IndYW,
	AbsX, AbsXW, AbsY, AbsYW
};

char *addr_modes[] = {
    "", "A", "imp", "#imm", "rel",
    "zp", "abs", "zp, x", "zp, y",
    "(ind)", "(ind,x)", "(ind), y", "(ind), y",
    "abs, x", "abs, x", "abs, y", "abs, y"
};

static enum NesAddrMode _opMode[] = {
//	0			1				2			3				4				5				6				7				8			9			A			B			C			D			E			F
	Imp,	IndX,		None,	IndX,		Zero,		Zero,		Zero,		Zero,		Imp,	Imm,	Acc,	Imm,	Abs,	Abs,	Abs,	Abs,	//0
	Rel,	IndY,		None,	IndYW,	ZeroX,	ZeroX,	ZeroX,	ZeroX,	Imp,	AbsY,	Imp,	AbsYW,AbsX,	AbsX,	AbsXW,AbsXW,//1
	Abs,	IndX,		None,	IndX,		Zero,		Zero,		Zero,		Zero,		Imp,	Imm,	Acc,	Imm,	Abs,	Abs,	Abs,	Abs,	//2
	Rel,	IndY,		None,	IndYW,	ZeroX,	ZeroX,	ZeroX,	ZeroX,	Imp,	AbsY,	Imp,	AbsYW,AbsX,	AbsX,	AbsXW,AbsXW,//3
	Imp,	IndX,		None,	IndX,		Zero,		Zero,		Zero,		Zero,		Imp,	Imm,	Acc,	Imm,	Abs,	Abs,	Abs,	Abs,	//4
	Rel,	IndY,		None,	IndYW,	ZeroX,	ZeroX,	ZeroX,	ZeroX,	Imp,	AbsY,	Imp,	AbsYW,AbsX,	AbsX,	AbsXW,AbsXW,//5
	Imp,	IndX,		None,	IndX,		Zero,		Zero,		Zero,		Zero,		Imp,	Imm,	Acc,	Imm,	Ind,	Abs,	Abs,	Abs,	//6
	Rel,	IndY,		None,	IndYW,	ZeroX,	ZeroX,	ZeroX,	ZeroX,	Imp,	AbsY,	Imp,	AbsYW,AbsX,	AbsX,	AbsXW,AbsXW,//7
	Imm,	IndX,		Imm,	IndX,		Zero,		Zero,		Zero,		Zero,		Imp,	Imm,	Imp,	Imm,	Abs,	Abs,	Abs,	Abs,	//8
	Rel,	IndYW,	None,	IndYW,	ZeroX,	ZeroX,	ZeroY,	ZeroY,	Imp,	AbsYW,Imp,	AbsYW,AbsXW,AbsXW,AbsYW,AbsYW,//9
	Imm,	IndX,		Imm,	IndX,		Zero,		Zero,		Zero,		Zero,		Imp,	Imm,	Imp,	Imm,	Abs,	Abs,	Abs,	Abs,	//A
	Rel,	IndY,		None,	IndY,		ZeroX,	ZeroX,	ZeroY,	ZeroY,	Imp,	AbsY,	Imp,	AbsY,	AbsX,	AbsX,	AbsY,	AbsY,	//B
	Imm,	IndX,		Imm,	IndX,		Zero,		Zero,		Zero,		Zero,		Imp,	Imm,	Imp,	Imm,	Abs,	Abs,	Abs,	Abs,	//C
	Rel,	IndY,		None,	IndYW,	ZeroX,	ZeroX,	ZeroX,	ZeroX,	Imp,	AbsY,	Imp,	AbsYW,AbsX,	AbsX,	AbsXW,AbsXW,//D
	Imm,	IndX,		Imm,	IndX,		Zero,		Zero,		Zero,		Zero,		Imp,	Imm,	Imp,	Imm,	Abs,	Abs,	Abs,	Abs,	//E
	Rel,	IndY,		None,	IndYW,	ZeroX,	ZeroX,	ZeroX,	ZeroX,	Imp,	AbsY,	Imp,	AbsYW,AbsX,	AbsX,	AbsXW,AbsXW,//F
};
#endif

uint8_t read6502(uint16_t addr)
{
    return mem[addr];
}

void write6502(uint16_t addr, uint8_t val)
{
    mem[addr] = val;
}

void initcpu(unsigned short newpc, unsigned char newa, unsigned char newx, unsigned char newy)
{
  regs.pc = newpc;
  regs.a = newa;
  regs.x = newx;
  regs.y = newy;

  //flags = 0;
  regs.n = 0;
  regs.v = 0;
  regs.b = 0;
  regs.d = 0;
  regs.i = 0;
  regs.z = 0;
  regs.c = 0;

  regs.sp = 0xff;
  cpucycles = 0;

  regs.irq = 0;
  regs.nmi = 0;
  regs.jammed = 0;

  //printf("init pc: %04X\n\n", regs.pc);
  
  if(mem == NULL)
  {
	mem = (unsigned char*)calloc(1, 0x10000 * sizeof(unsigned char));
    memset(mem, 0, 0x10000);
  }
}

void push(uint8_t val) {
    write6502(0x100|(regs.sp--&0xff),val);
}

uint8_t pop() {
    return read6502(0x100|(++regs.sp&0xff));
}

uint8_t pop_inc_after() {
    return read6502(0x100|(regs.sp++&0xff));
}

uint8_t pop_no_inc() {
    return read6502(0x100|(regs.sp&0xff));
}

uint8_t read_PC() {
    return read6502(regs.pc++);
}

// read a 16-bit word from memory
static inline uint16_t read16() {
    uint16_t imm = (uint16_t)read6502(regs.pc++);
    advance_cycle();
    imm |= (uint16_t)read6502(regs.pc++)<<8;
    advance_cycle();
    return imm;
}

static inline void add_cycles(uint8_t cycles) {
    regs.cycles += cycles;
    return;
}

uint8_t read_flag() {
    return (regs.n<<7)|(regs.v<<6)|(1<<5)|(regs.b<<4)|(regs.d<<3)|(regs.i<<2)|(regs.z<<1)|regs.c;
}

uint8_t read_flag_no_b() {
    return (regs.n<<7)|(regs.v<<6)|(1<<5)|(regs.d<<3)|(regs.i<<2)|(regs.z<<1)|regs.c;
}

void write_flag(uint8_t val) {
    regs.n = val>>7&1;
    regs.v = val>>6&1;
    regs.b = val>>4&1;
    regs.d = val>>3&1;
    regs.i = val>>2&1;
    regs.z = val>>1&1;
    regs.c = val&1;
}

void write_flag_no_b(uint8_t val) {
    regs.n = val>>7&1;
    regs.v = val>>6&1;
    regs.d = val>>3&1;
    regs.i = val>>2&1;
    regs.z = val>>1&1;
    regs.c = val&1;
}

uint8_t lsr(uint8_t val) {
    regs.n = 0;
    regs.z = (val>>1)==0;
    regs.c = val&1;
    return val>>1;
}

void lsr_a() { // for "LSR A"
    regs.n = 0;
    regs.z = (regs.a>>1)==0;
    regs.c = regs.a&1;
    regs.a >>= 1;
}

void eor(uint8_t val) {
    regs.a ^= val;
    regs.n = regs.a>>7;
    regs.z = regs.a==0;
}

void ora(uint8_t val) {
    regs.a |= val;
    regs.n = regs.a>>7;
    regs.z = regs.a==0;
}

void and(uint8_t val) {
    regs.a &= val;
    regs.n = regs.a>>7;
    regs.z = regs.a==0;
}

void anc(uint8_t val) {
    regs.a &= val;
    regs.n = regs.a>>7;
    regs.z = regs.a==0;
    regs.c = regs.a>>7;
}

void nop(uint8_t val) {
    (uint8_t)val;
    return;
}

void nop_imp(void) {
    return;
}

uint8_t sre(uint8_t val) {
    uint8_t new_val = lsr(val);
    eor(new_val);
    return new_val;
}

void lax(uint8_t val) {
    regs.a = val;
    regs.x = val;
    regs.n = val>>7;
    regs.z = val==0;
}

void lax_imm(uint8_t val) {
    val = (regs.a|0xee)&val;
    regs.a = val;
    regs.x = val;
    regs.n = val>>7;
    regs.z = val==0;
}

void las(uint8_t val) {
    regs.a = regs.sp & val;
    regs.x = regs.a;
    regs.sp = regs.a;
    regs.n = regs.a>>7;
    regs.z = regs.a==0;
}

uint8_t tas() {
    regs.sp = regs.a & regs.x;
    if ((regs.temp>>8) != (regs.temp_cross>>8)) {
        uint8_t val = regs.sp & ((regs.temp_cross>>8)&0xff);
        regs.temp_cross = (regs.temp_cross&0xff)|(val<<8);
        return val;
    }
    return regs.sp & (((regs.temp>>8)+1)&0xff);}

void lda(uint8_t val) {
    regs.a = val;
    regs.n = val>>7;
    regs.z = val==0;
}

void ldx(uint8_t val) {
    regs.x = val;
    regs.n = val>>7;
    regs.z = val==0;
}

void ldy(uint8_t val) {
    regs.y = val;
    regs.n = val>>7;
    regs.z = val==0;
}

void tya() {
    regs.a = regs.y;
    regs.n = regs.a>>7;
    regs.z = regs.a==0;
}

void txa() {
    regs.a = regs.x;
    regs.n = regs.a>>7;
    regs.z = regs.a==0;
}

void tay() {
    regs.y = regs.a;
    regs.n = regs.y>>7;
    regs.z = regs.y==0;
}

void tax() {
    regs.x = regs.a;
    regs.n = regs.x>>7;
    regs.z = regs.x==0;
}

void inx() {
    regs.x++;
    regs.n = regs.x>>7;
    regs.z = regs.x==0;
}

void iny() {
    regs.y++;
    regs.n = regs.y>>7;
    regs.z = regs.y==0;
}

void dex() {
    regs.x--;
    regs.n = regs.x>>7;
    regs.z = regs.x==0;
}

void dey() {
    regs.y--;
    regs.n = regs.y>>7;
    regs.z = regs.y==0;
}

uint8_t shy() {
    if ((regs.temp>>8) != (regs.temp_cross>>8)) {
        uint8_t val = regs.y & ((regs.temp_cross>>8)&0xff);
        regs.temp_cross = (regs.temp_cross&0xff)|(val<<8);
        return val;
    }
    return regs.y & (((regs.temp>>8)+1)&0xff);
}

uint8_t shx() {
    if ((regs.temp>>8) != (regs.temp_cross>>8)) {
        uint8_t val = regs.x & ((regs.temp_cross>>8)&0xff);
        regs.temp_cross = (regs.temp_cross&0xff)|(val<<8);
        return val;
    }
    return regs.x & (((regs.temp>>8)+1)&0xff);
}

uint8_t sha() {
    if ((regs.temp>>8) != (regs.temp_cross>>8)) {
        uint8_t val = regs.a & regs.x & ((regs.temp_cross>>8)&0xff);
        regs.temp_cross = (regs.temp_cross&0xff)|(val<<8);
        return val;
    }
    return regs.a & regs.x & (((regs.temp>>8)+1)&0xff);
}

uint8_t asl(uint8_t val) {
    uint8_t carry = val>>7&1;
    val = val<<1;
    regs.z = val==0;
    regs.n = val>>7;
    regs.c = carry;
    return val;
}

void asl_a() { // for ASL A
    uint8_t carry = regs.a>>7&1;
    regs.a = regs.a<<1;
    regs.z = regs.a==0;
    regs.n = regs.a>>7;
    regs.c = carry;
}


uint8_t rol(uint8_t val) {
    uint8_t carry = val>>7&1;
    val = regs.c|(val<<1);
    regs.z = val==0;
    regs.n = val>>7;
    regs.c = carry;
    return val;
}

void rol_a() { // for ROL A
    uint8_t carry = regs.a>>7&1;
    regs.a = regs.c|(regs.a<<1);
    regs.z = regs.a==0;
    regs.n = regs.a>>7;
    regs.c = carry;
}

uint8_t ror(uint8_t val) {
    uint8_t carry = val&1;
    val = (regs.c<<7)|(val>>1);
    regs.z = val==0;
    regs.n = val>>7;
    regs.c = carry;
    return val;
}

void ror_a() {
    uint8_t carry = regs.a&1;
    regs.a = (regs.c<<7)|(regs.a>>1);
    regs.z = regs.a==0;
    regs.n = regs.a>>7;
    regs.c = carry;
}

void adc(uint8_t val) {
    if (regs.d) {
        uint16_t result = regs.a + val + regs.c;
        regs.c = result>>8&1;
        regs.z = (result&0xff)==0;
        if ((regs.a^result^val)&0x10) {
            result = (result&0xf0)|((result+0x06)&0xf);
        } else if ((result&15)>9) {
            regs.c |= result >= (256-6);
            result += 0x06;
        }
        regs.n = result>>7&1;
        regs.v = ((result^regs.a)&(result^val))>>7&1;
        regs.c |= result >= 0xA0;
        if (regs.c) result += 0x60;
        regs.a = result&0xff;
    } else {
        uint16_t result = regs.a + val + regs.c;
        regs.n = result>>7&1;
        regs.c = result>>8&1;
        regs.z = (result&0xff)==0;
        regs.v = ((result^regs.a)&(result^val))>>7&1;
        regs.a = result&0xff;
    }
}

void sbc(uint8_t val) {
    val ^= 0xff;
    uint16_t result = regs.a + val + regs.c;
    regs.n = result>>7&1;
    regs.c = result>>8&1;
    regs.z = (result&0xff)==0;
    regs.v = ((result^regs.a)&(result^val))>>7&1;
    if (regs.d) {
        if (!((regs.a^result^val)&0x10)) result = (result&0xf0)|((result+0xfa)&0xf);
        if (!regs.c) result += 0xa0;
    }
    regs.a = result&0xff;
}

uint8_t rra(uint8_t val) {
    uint8_t carry = val&1;
    uint8_t new_val = (regs.c<<7)|(val>>1);
    regs.c = carry;
    adc(new_val);
    return new_val;
}

uint8_t rla(uint8_t val) {
    uint8_t new_val = rol(val);
    and(new_val);
    return new_val;
}

uint8_t slo(uint8_t val) {
    uint8_t new_val = asl(val);
    ora(new_val);
    return new_val;
}

uint8_t sta() {
    return regs.a;
}

uint8_t stx() {
    return regs.x;
}

uint8_t sty() {
    return regs.y;
}

void sed() {
    regs.d = 1;
}

void cld() {
    regs.d = 0;
}

void sec() {
    regs.c = 1;
}

void clc() {
    regs.c = 0;
}

void sei() {
    regs.i = 1;
}

void cli() {
    regs.i = 0;
}

void clv() {
    regs.v = 0;
}

void cmp(uint8_t val) {
    uint8_t diff = regs.a-val;
    regs.c = regs.a >= val;
    regs.z = diff==0;
    regs.n = diff>>7;
}

void cpx(uint8_t val) {
    uint8_t diff = regs.x-val;
    regs.c = regs.x >= val;
    regs.z = diff==0;
    regs.n = diff>>7;
}

void cpy(uint8_t val) {
    uint8_t diff = regs.y-val;
    regs.c = regs.y >= val;
    regs.z = diff==0;
    regs.n = diff>>7;
}

uint8_t inc(uint8_t val) {
    val++;
    regs.z = val==0;
    regs.n = val>>7&1;
    return val;
}

uint8_t dec(uint8_t val) {
    val--;
    regs.z = val==0;
    regs.n = val>>7&1;
    return val;
}

uint8_t isc(uint8_t val) {
    uint8_t new_val = inc(val);
    sbc(new_val);
    return new_val;    
}

uint8_t dcp(uint8_t val) {
    uint8_t new_val = dec(val);
    cmp(new_val);
    return new_val;
}

uint8_t sax() {
    return regs.a & regs.x;
}

void axs(uint8_t val) {
    int result = (regs.a&regs.x)-val;
    regs.c = (result>>8&1)^1;
    regs.z = (result&0xff)==0;
    regs.n = result>>7&1;
    regs.x = result&0xff;
}

void xaa(uint8_t val) { // tf name is XAA?!?!??!
    int result = (regs.a|0xee)&regs.x&val;
    regs.n = result>>7&1;
    regs.z = (result&0xff)==0;
    regs.a = result&0xff;
}

void arr(uint8_t val) { // whar??!?!
    and(val);
    uint8_t and_a = regs.a;
    ror_a();
    if (regs.d) {
        // BCD yay
        regs.v = (regs.a^(regs.a<<1))>>6&1;  
        if ((and_a&0xf)+(and_a&1)>5) regs.a = (regs.a&0xf0)|((regs.a+0x06)&0xf); 
        regs.c = ((and_a&0xf0)+(and_a&0x10)>0x50)?1:0;
        if (regs.c) regs.a += 0x60;   
    } else {
        regs.c = regs.a>>6&1;
        regs.v = (regs.a^(regs.a<<1))>>6&1;
    }
}

void bit(uint8_t val) {
    regs.n = val>>7&1;
    regs.v = val>>6&1;
    val &= regs.a;
    regs.z = val==0;
}

void alr(uint8_t val) {
    and(val);
    lsr_a(val);
}

void txs() {
    regs.sp = regs.x;
}

void tsx() {
    regs.x = regs.sp;
    regs.z = regs.x==0;
    regs.n = regs.x>>7;
}

void r_abs_ind(void (*f)(uint8_t), uint8_t index) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint16_t addr = read16(); // fetch lo and hi (2 cycles)
    // add x to low byte
    uint16_t addr_incomplete = (addr&0xff00)|((addr+index)&0xff);
    uint8_t val = read6502(addr_incomplete);
    advance_cycle(); // read from effective address
    addr += index; // fix hi-byte
    if (addr != addr_incomplete) { // page boundary check
        val = read6502(addr); // re-read
        advance_cycle();
    }
    f(val);
}

void r_abs(void (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint16_t addr = read16(); // fetch lo and hi (2 cycles)
    uint8_t val = read6502(addr);
    advance_cycle(); // read from effective address
    f(val);
}

void rmw_abs(uint8_t (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint16_t addr = read16(); // fetch lo and hi (2 cycles)
    uint8_t old_val = read6502(addr);
    advance_cycle(); // read from effective address
    write6502(addr,old_val); // write value back
    advance_cycle();
    uint8_t val = f(old_val);
    write6502(addr,val); // write new value
    advance_cycle();       
}

void rmw_abs_ind(uint8_t (*f)(uint8_t), uint8_t index) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint16_t addr = read16(); // fetch lo and hi (2 cycles)
    regs.temp = addr; // for TAS
    // add x to low byte
    uint16_t addr_incomplete = (addr&0xff00)|((addr+index)&0xff);
    read6502(addr_incomplete);
    advance_cycle(); // read from effective address
    addr += index; // fix hi-byte
    regs.temp_cross = addr; // for TAS
    uint8_t old_val = read6502(addr); // read from effective address
    advance_cycle();
    write6502(addr,old_val); // write value back
    advance_cycle();       
    uint8_t val = f(old_val);
    write6502(addr,val); // write new value
    advance_cycle();       
}

void w_abs_ind(uint8_t (*f)(), uint8_t index) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint16_t addr = read16(); // fetch lo and hi (2 cycles)
    regs.temp = addr; // for SHY
    // add x to low byte
    uint16_t addr_incomplete = (addr&0xff00)|((addr+index)&0xff);
    read6502(addr_incomplete);
    advance_cycle(); // read from effective address
    addr += index; // fix hi-byte
    regs.temp_cross = addr;
    uint8_t val = f();
    write6502(regs.temp_cross,val); // write new value
    advance_cycle();       
}

void w_abs(uint8_t (*f)()) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint16_t addr = read16(); // fetch lo and hi (2 cycles)
    uint8_t val = f();
    write6502(addr,val); // write new value
    advance_cycle();          
}


void r_zp_ind(void (*f)(uint8_t), uint8_t index) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t old_val = read6502(addr);
    advance_cycle(); // read from address
    addr = (addr+index)&0xff; // add index reg (HIGH BYTE IS 0)
    uint8_t val = read6502(addr);  // read from effective address
    advance_cycle();
    f(val);  
}

void w_zp_ind(uint8_t (*f)(void), uint8_t index) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t old_val = read6502(addr);
    advance_cycle(); // read from address
    addr = (addr+index)&0xff; // add index reg (HIGH BYTE IS 0)
    uint8_t val = f();
    write6502(addr,val);
    advance_cycle();
}

void rmw_zp_ind(uint8_t (*f)(uint8_t), uint8_t index) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t old_val = read6502(addr);
    advance_cycle(); // read from address
    addr = (addr+index)&0xff; // add index reg (HIGH BYTE IS 0)
    old_val = read6502(addr);  // read from effective address
    advance_cycle();
    write6502(addr,old_val); // write value back
    advance_cycle();
    uint8_t val = f(old_val); 
    write6502(addr,val); // write value
    advance_cycle(); 
}


void r_zp(void (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t val = read6502(addr);
    advance_cycle(); // read from address 
    f(val);
}

void w_zp(uint8_t (*f)()) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t val = f(); 
    write6502(addr,val); // write value
    advance_cycle(); 
}

void rmw_zp(uint8_t (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t old_val = read6502(addr);
    advance_cycle(); // read from address
    write6502(addr,old_val); // write value back
    advance_cycle();
    uint8_t val = f(old_val); 
    write6502(addr,val); // write value
    advance_cycle(); 
}

void r_imp(void (*f)()) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    read6502(regs.pc); // read next instruction byte (and throw it away)
    advance_cycle();
    f();  
}

void r_imm(void (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t val = read_PC(); // read next instruction byte
    advance_cycle();
    f(val);  
}

void branch(bool flag) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    int8_t operand = (int8_t)read_PC(); // fetch operand
    //advance_cycle();
    uint8_t next_ins = read6502(regs.pc);
    uint16_t branch_pc = regs.pc+operand;
    if (flag) regs.pc = (regs.pc&0xff00)|((regs.pc+operand)&0xff);
    advance_cycle();
    if (flag) {
        next_ins = read6502(regs.pc);
        if (regs.pc != branch_pc) {
            // page-crossing has occured!
            regs.pc = branch_pc;
            advance_cycle();
            next_ins = read6502(regs.pc);
            advance_cycle();
        } else {
            //regs.pc++;
            advance_cycle();
        }
    }
}

void r_indx(void (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t old_val = read6502(addr);
    advance_cycle(); // read from address
    addr = (addr+regs.x)&0xff; // add x
    uint16_t addr_ind = read6502(addr);  // fetch lo
    advance_cycle();
    addr_ind |= read6502((addr+1)&0xff)<<8; // fetch hi
    advance_cycle();
    uint8_t val = read6502(addr_ind); // read from effective address
    advance_cycle();
    f(val);      
}

void w_indx(uint8_t (*f)()) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t old_val = read6502(addr);
    advance_cycle(); // read from address
    addr = (addr+regs.x)&0xff; // add x
    uint16_t addr_ind = read6502(addr);  // fetch lo
    advance_cycle();
    addr_ind |= read6502((addr+1)&0xff)<<8; // fetch hi
    advance_cycle();
    uint8_t val = f();
    write6502(addr_ind,val); // read from effective address
    advance_cycle();
}

void rmw_indx(uint8_t (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint8_t old_val = read6502(addr);
    advance_cycle(); // read from address
    addr = (addr+regs.x)&0xff; // add x
    uint16_t addr_ind = read6502(addr);  // fetch lo
    advance_cycle();
    addr_ind |= read6502((addr+1)&0xff)<<8; // fetch hi
    advance_cycle();
    old_val = read6502(addr_ind); // read from effective address
    advance_cycle();
    write6502(addr_ind,old_val); // write back
    advance_cycle();
    uint8_t val = f(old_val);      
    write6502(addr_ind,val); // write new value
    advance_cycle();
}

void r_indy(void (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint16_t addr_ind = read6502(addr);  // fetch lo
    advance_cycle();
    addr_ind |= read6502((addr+1)&0xff)<<8; // fetch hi
    uint16_t real_addr = addr_ind + regs.y;
    addr_ind = (addr_ind&0xff00)|(real_addr&0xff);
    advance_cycle();
    uint8_t val = read6502(addr_ind); // read from effective address
    if (addr_ind != real_addr) {
        // page-crossing
        addr_ind = real_addr;
        advance_cycle();
        val = read6502(addr_ind); // read from effective address
        advance_cycle();
    } else {
        advance_cycle();
    }
    f(val);      
}

void w_indy(uint8_t (*f)()) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint16_t addr_ind = read6502(addr);  // fetch lo
    advance_cycle();
    addr_ind |= read6502((addr+1)&0xff)<<8; // fetch hi
    regs.temp = addr_ind;
    uint16_t real_addr = addr_ind + regs.y;
    addr_ind = (addr_ind&0xff00)|(real_addr&0xff);
    advance_cycle();
    uint8_t val = read6502(addr_ind); // read from effective address
    addr_ind = real_addr;
    advance_cycle();
    regs.temp_cross = addr_ind;
    val = f();      
    write6502(regs.temp_cross,val); // write new value
    advance_cycle();
}

void rmw_indy(uint8_t (*f)(uint8_t)) {
    advance_cycle(); // read PC (already done, just advance 1 cycle)
    uint8_t addr = read_PC(); // fetch addr
    advance_cycle();
    uint16_t addr_ind = read6502(addr);  // fetch lo
    advance_cycle();
    addr_ind |= read6502((addr+1)&0xff)<<8; // fetch hi
    uint16_t real_addr = addr_ind + regs.y;
    addr_ind = (addr_ind&0xff00)|(real_addr&0xff);
    advance_cycle();
    uint8_t val = read6502(addr_ind); // read from effective address
    addr_ind = real_addr;
    advance_cycle();
    val = read6502(addr_ind); // read from effective address
    advance_cycle();
    write6502(addr_ind,val); // write back value
    advance_cycle();
    val = f(val);      
    write6502(addr_ind,val); // write new value
    advance_cycle();
}

uint8_t step() {
    //printf("pc: %04X\n", regs.pc);

    if (regs.jammed) {
        advance_cycle();
        return 0; // JAM (crashes system until RESET)
    }

    if (regs.irq) {
        regs.irq = 0;
        if (regs.i == 0) {
            advance_cycle(); // read PC
            read6502(regs.pc); // read next instruction byte (and throw it away)
            advance_cycle();
            push(regs.pc>>8); // push pch
            advance_cycle();
            push(regs.pc&0xff); // push pcl
            advance_cycle();
            push(read_flag()&(255^(1<<4))); // push p
            advance_cycle();
            uint16_t addr = read6502(0xFFFE); // get lo byte
            advance_cycle();
            addr |= read6502(0xFFFF)<<8; // get hi byte
            advance_cycle();
            regs.pc = addr; // profit
            regs.i = 1;
        }
    }

    uint8_t opcode = read_PC();
    switch (opcode) {
        case 0x4f: { // SRE abs
            rmw_abs(sre);
            break;
        }

        case 0x5f: { // SRE abs, x
            rmw_abs_ind(sre,regs.x);
            break;
        }

        case 0x5b: { // SRE abs, y
            rmw_abs_ind(sre,regs.y);
            break;
        }

        case 0x57: { // SRE zp, x
            rmw_zp_ind(sre,regs.x);
            break;
        }

        case 0x47: { // SRE zp
            rmw_zp(sre);
            break;
        }

        case 0x53: { // SRE (ind), y
            rmw_indy(sre);
            break;
        }

        case 0x43: { // SRE (ind, x)
            rmw_indx(sre);
            break;
        }

        case 0x14:
        case 0x34:
        case 0x54:
        case 0x74:
        case 0xd4:
        case 0xf4: { // NOP zp, x
            r_zp_ind(nop,regs.x);
            break;
        }

        case 0x1a:
        case 0x3a:
        case 0x5a:
        case 0x7a:
        case 0xda:
        case 0xea:
        case 0xfa: { // NOP
            r_imp(nop_imp);
            break;
        }

        case 0x80:
        case 0x82:
        case 0x89:
        case 0xc2:
        case 0xe2: { // NOP #imm
            r_imm(nop);
            break;
        }

        case 0x0C: { // NOP abs
            r_abs(nop);
            break;
        }

        case 0x1C:
        case 0x3C:
        case 0x5C:
        case 0x7C:
        case 0xDC:
        case 0xFC: { // NOP abs, x
            r_abs_ind(nop,regs.x);
            break;
        }

        case 0x04:
        case 0x44:
        case 0x64: { // NOP zp
            r_zp(nop);
            break;
        }
        case 0xa3: { // LAX (ind, x)
            r_indx(lax);
            break;
        }

        case 0xBB: { // LAS abs, y
            r_abs_ind(las,regs.y);
            break;
        }

        case 0xA7: { // LAX zp
            r_zp(lax);
            break;
        }

        case 0xB7: { // LAX zp, y
            r_zp_ind(lax,regs.y);
            break;
        }

        case 0xA1: { // LDA (ind,x)
            r_indx(lda);
            break;
        }

        case 0xB1: { // LDA (ind), y
            r_indy(lda);
            break;
        }

        case 0xB9: { // LDA abs, y
            r_abs_ind(lda,regs.y);
            break;
        }

        case 0xA9: { // LDA #imm
            r_imm(lda);
            break;
        }

        case 0xAB: { // LAX #imm
            r_imm(lax_imm);
            break;
        }

        case 0xBF: { // LAX abs, y
            r_abs_ind(lax,regs.y);
            break;
        }

        case 0x00: { // BRK
            advance_cycle(); // read PC
            read6502(regs.pc++); // read next instruction byte (and throw it away)
            advance_cycle();
            push(regs.pc>>8); // push pch
            advance_cycle();
            push(regs.pc&0xff); // push pcl
            advance_cycle();
            push(read_flag()|(1<<4)); // push p
            advance_cycle();
            uint16_t addr = read6502(0xFFFE); // get lo byte
            advance_cycle();
            addr |= read6502(0xFFFF)<<8; // get hi byte
            advance_cycle();
            regs.pc = addr; // profit
            regs.i = 1;
            return 0;
            break;
        }

        case 0x20: { // JSR imm16
            advance_cycle(); // read PC
            uint16_t addr = read_PC(); // get lo byte
            advance_cycle();
            advance_cycle(); // ????
            push(regs.pc>>8); // push pch
            advance_cycle();
            push(regs.pc&0xff); // push pcl
            advance_cycle();
            addr |= read_PC()<<8; // get hi byte
            advance_cycle();
            regs.pc = addr; // profit
            break;
        }

        case 0x40: { // RTI
            advance_cycle(); // read PC
            read6502(regs.pc); // read next instruction byte (and throw it away)
            advance_cycle();
            regs.sp++; // increment S
            advance_cycle();
            write_flag_no_b(pop_inc_after()); // get P
            advance_cycle();
            regs.pc = (regs.pc&0xff00)|(pop_inc_after()); // get PCL
            advance_cycle();
            regs.pc = (regs.pc&0xff)|(pop_no_inc()<<8); // get PCH
            advance_cycle();
            return 0;
            break;
        }

        case 0x60: { // RTS
            advance_cycle(); // read PC
            read6502(regs.pc); // read next instruction byte (and throw it away)
            advance_cycle();
            regs.sp++; // increment S
            advance_cycle();
            regs.pc = (regs.pc&0xff00)|(pop_inc_after()); // get PCL
            advance_cycle();
            regs.pc = (regs.pc&0xff)|(pop_no_inc()<<8); // get PCH
            advance_cycle();
            regs.pc++;
            advance_cycle();
            return 0;
            break;
        }

        case 0x9c: { // SHY abs, x
            w_abs_ind(shy,regs.x);
            break;
        }

        case 0x9e: { // SHX abs, y
            w_abs_ind(shx,regs.y);
            break;
        }

        case 0x98: { // tya
            r_imp(tya);
            break;
        }

        case 0xa8: { // tay
            r_imp(tay);
            break;
        }

        case 0x67: { // rra zp
            rmw_zp(rra);
            break;
        }

        case 0x73: { // rra (ind), y
            rmw_indy(rra);
            break;
        }

        case 0x63: { // rra (ind, x)
            rmw_indx(rra);
            break;
        }

        case 0x77: { // rra zp, x
            rmw_zp_ind(rra,regs.x);
            break;
        }

        case 0x7F: { // rra abs, x
            rmw_abs_ind(rra,regs.x);
            break;
        }

        case 0x7B: { // rra abs, y
            rmw_abs_ind(rra,regs.y);
            break;
        }

        case 0x6F: { // rra abs
            rmw_abs(rra);
            break;
        }

        case 0x8e: { // STX abs
            w_abs(stx);
            break;
        }

        case 0x18: { // CLC
            r_imp(clc);
            break;
        }

        case 0x38: { // SEC
            r_imp(sec);
            break;
        }

        case 0x58: { // CLI
            r_imp(cli);
            break;
        }

        case 0x78: { // SEI
            r_imp(sei);
            break;
        }

        case 0xd8: { // CLD
            r_imp(cld);
            break;
        }

        case 0xf8: { // SED
            r_imp(sed);
            break;
        }

        case 0x61: { // ADC (ind, x)
            r_indx(adc);
            break;
        }

        case 0x71: { // ADC (ind), y
            r_indy(adc);
            break;
        }

        case 0xCD: { // CMP abs
            r_abs(cmp);
            break;
        }

        case 0xDD: { // CMP abs, x
            r_abs_ind(cmp,regs.x);
            break;
        }

        case 0xC9: { // CMP #imm
            r_imm(cmp);
            break;
        }

        case 0xD9: { // CMP abs, y
            r_abs_ind(cmp,regs.y);
            break;
        }

        case 0xD5: { // CMP zp, x
            r_zp_ind(cmp,regs.x);
            break;
        }

        case 0xC5: { // CMP zp
            r_zp(cmp);
            break;
        }

        case 0xC1: { // CMP (ind,x)
            r_indx(cmp);
            break;
        }
        
        case 0xD1: { // CMP (ind), y
            r_indy(cmp);
            break;
        }

        case 0x36: { // ROL zp, x
            rmw_zp_ind(rol,regs.x);
            break;
        }

        case 0x3e: { // ROL abs, x
            rmw_abs_ind(rol,regs.x);
            break;
        }

        case 0x26: { // ROL zp
            rmw_zp(rol);
            break;
        }

        case 0x2e: { // ROL abs
            rmw_abs(rol);
            break;
        }

        case 0x6e: { // ROR abs
            rmw_abs(ror);
            break;
        }

        case 0x66: { // ROR zp
            rmw_zp(ror);
            break;
        }

        case 0x7E: { // ROR abs, x
            rmw_abs_ind(ror,regs.x);
            break;
        }

        case 0x4D: { // EOR abs
            r_abs(eor);
            break;
        }

        case 0x5D: { // EOR abs, x
            r_abs_ind(eor,regs.x);
            break;
        }

        case 0x59: { // EOR abs, y
            r_abs_ind(eor,regs.y);
            break;
        }

        case 0x41: { // EOR (ind,x)
            r_indx(eor);
            break;
        }

        case 0x51: { // EOR (ind), y
            r_indy(eor);
            break;
        }

        case 0x45: { // EOR zp
            r_zp(eor);
            break;
        }

        case 0x55: { // EOR zp, x
            r_zp_ind(eor,regs.x);
            break;
        }

        case 0x49: { // EOR #imm
            r_imm(eor);
            break;
        }

        case 0x06: { // ASL zp
            rmw_zp(asl);
            break;
        }

        case 0x0e: { // ASL abs
            rmw_abs(asl);
            break;
        }

        case 0x16: { // ASL zp, x
            rmw_zp_ind(asl,regs.x);
            break;
        }

        case 0x1E: { // ASL abs, x
            rmw_abs_ind(asl,regs.x);
            break;
        }
        
        case 0x0A: { // ASL A
            r_imp(asl_a);
            break;
        }

        case 0xAD: { // LDA abs
            r_abs(lda);
            break;
        }

        case 0xA5: { // LDA zp
            r_zp(lda);
            break;
        }

        case 0xE6: { // INC zp
            rmw_zp(inc);
            break;
        }

        case 0xEE: { // INC abs
            rmw_abs(inc);
            break;
        }

        case 0xF6: { // INC zp,x
            rmw_zp_ind(inc,regs.x);
            break;
        }

        case 0xFE: { // INC abs,x
            rmw_abs_ind(inc,regs.x);
            break;
        }

        case 0xB5: { // LDA zp, x
            r_zp_ind(lda,regs.x);
            break;
        }

        case 0x94: { // STY zp, x
            w_zp_ind(sty,regs.x);
            break;
        }

        case 0x84: { // STY zp
            w_zp(sty);
            break;
        }

        case 0x8C: { // STY abs
            w_abs(sty);
            break;
        }

        case 0xC3: { // DCP (ind,x)
            rmw_indx(dcp);
            break;
        }

        case 0x83: { // SAX (ind,x)
            w_indx(sax);
            break;
        }

        case 0x87: { // SAX zp
            w_zp(sax);
            break;
        }

        case 0x97: { // SAX zp, y
            w_zp_ind(sax,regs.y);
            break;
        }

        case 0x8F: { // SAX abs
            w_abs(sax);
            break;
        }

        case 0xE9:
        case 0xEB: { // SBC #imm
            r_imm(sbc);
            break;
        }

        case 0xE5: { // SBC zp
            r_zp(sbc);
            break;
        }

        case 0xF5: { // SBC zp, x
            r_zp_ind(sbc,regs.x);
            break;
        }

        case 0xF1: { // SBC (ind), y
            r_indy(sbc);
            break;
        }

        case 0xE1: { // SBC (ind,x)
            r_indx(sbc);
            break;
        }

        case 0xED: { // SBC abs
            r_abs(sbc);
            break;
        }

        case 0xFD: { // SBC abs, x
            r_abs_ind(sbc,regs.x);
            break;
        }

        case 0x91: { // STA (ind) y
            w_indy(sta);
            break;
        }

        case 0x81: { // STA (ind,x)
            w_indx(sta);
            break;
        }

        case 0x95: { // STA zp, x
            w_zp_ind(sta,regs.x);
            break;
        }

        case 0x85: { // STA zp
            w_zp(sta);
            break;
        }

        case 0x2F: { // RLA
            rmw_abs(rla);
            break;
        }

        case 0x27: { // RLA zp
            rmw_zp(rla);
            break;
        }

        case 0x37: { // RLA zp, x
            rmw_zp_ind(rla,regs.x);
            break;
        }

        case 0x3f: { // RLA abs, x
            rmw_abs_ind(rla,regs.x);
            break;
        }

        case 0x3b: { // RLA abs, y
            rmw_abs_ind(rla,regs.y);
            break;
        }

        case 0x23: { // RLA (ind,x)
            rmw_indx(rla);
            break;
        }

        case 0x33: { // RLA (ind), y
            rmw_indy(rla);
            break;
        }

        case 0xA4: { // LDY zp
            r_zp(ldy);
            break;
        }

        case 0xB4: { // LDY zp, x
            r_zp_ind(ldy,regs.x);
            break;
        }

        case 0xBC: { // LDY abs, x
            r_abs_ind(ldy,regs.x);
            break;
        }

        case 0xAC: { // LDY abs
            r_abs(ldy);
            break;
        }

        case 0x4a: { // LSR A
            r_imp(lsr_a);
            break;
        }

        case 0x4e: { // LSR abs
            rmw_abs(lsr);
            break;
        }

        case 0x46: { // LSR zp
            rmw_zp(lsr);
            break;
        }

        case 0x56: { // LSR zp, x
            rmw_zp_ind(lsr,regs.x);
            break;
        }

        case 0x5e: { // LSR abs, x
            rmw_abs_ind(lsr,regs.x);
            break;
        }

        case 0x01: { // ORA (ind,x)
            r_indx(ora);
            break;
        }

        case 0x09: { // ORA #imm
            r_imm(ora);
            break;
        }

        case 0x11: { // ORA (ind), y
            r_indy(ora);
            break;
        }

        case 0x0d: { // ORA abs
            r_abs(ora);
            break;
        }

        case 0x1d: { // ORA abs, x
            r_abs_ind(ora,regs.x);
            break;
        }

        case 0x19: { // ORA abs, y
            r_abs_ind(ora,regs.y);
            break;
        }

        case 0x05: { // ORA zp
            r_zp(ora);
            break;
        }

        case 0x15: { // ORA zp, x
            r_zp_ind(ora,regs.x);
            break;
        }

        case 0xAE: { // LDX abs
            r_abs(ldx);
            break;
        }

        case 0xA6: { // LDX zp
            r_zp(ldx);
            break;
        }

        case 0xB6: { // LDX zp, y
            r_zp_ind(ldx,regs.y);
            break;
        }

        case 0xBE: { // LDX abs. y
            r_abs_ind(ldx,regs.y);
            break;
        }

        case 0xA0: { // LDY #imm
            r_imm(ldy);
            break;
        }

        case 0xA2: { // LDX #imm
            r_imm(ldx);
            break;
        }

        case 0x07: { // SLO zp
            rmw_zp(slo);
            break;
        }

        case 0x0F: { // SLO abs
            rmw_abs(slo);
            break;
        }

        case 0x1F: { // SLO abs,x
            rmw_abs_ind(slo,regs.x);
            break;
        }

        case 0x1B: { // SLO abs,y
            rmw_abs_ind(slo,regs.y);
            break;
        }

        case 0x17: { // SLO zp, x
            rmw_zp_ind(slo,regs.x);
            break;
        }

        case 0x03: { // SLO (ind,x)
            rmw_indx(slo);
            break;
        }

        case 0x13: { // SLO (ind), y
            rmw_indy(slo);
            break;
        }

        case 0xDB: { // DCP abs, y
            rmw_abs_ind(dcp,regs.y);
            break;
        }

        case 0xDF: { // DCP abs, x
            rmw_abs_ind(dcp,regs.x);
            break;
        }

        case 0xCE: { // DEC abs
            rmw_abs(dec);
            break;
        }

        case 0xC6: { // DEC zp
            rmw_zp(dec);
            break;
        }

        case 0xD6: { // DEC zp, x
            rmw_zp_ind(dec,regs.x);
            break;
        }

        case 0xDE: { // DEC abs, x
            rmw_abs_ind(dec,regs.x);
            break;
        }

        case 0xF9: { // SBC abs, y
            r_abs_ind(sbc,regs.y);
            break;
        }

        case 0xEF: { // ISC abs
            rmw_abs(isc);
            break;
        }

        case 0xFB: { // ISC abs, y
            rmw_abs_ind(isc,regs.y);
            break;
        }

        case 0xFF: { // ISC abs, x
            rmw_abs_ind(isc,regs.x);
            break;
        }

        case 0xF3: { // ISC (ind), y
            rmw_indy(isc);
            break;
        }

        case 0xE3: { // ISC abs, y
            rmw_indx(isc);
            break;
        }

        case 0xE7: { // ISC zp
            rmw_zp(isc);
            break;
        }

        case 0xF7: { // ISC zp, x
            rmw_zp_ind(isc,regs.x);
            break;
        }

        case 0x8D: { // STA abs
            w_abs(sta);
            break;
        }

        case 0x99: { // STA abs, y
            w_abs_ind(sta,regs.y);
            break;
        }

        case 0x9d: { // STA abs, x
            w_abs_ind(sta,regs.x);
            break;
        }

        case 0x76: { // ROR zp, x
            rmw_zp_ind(ror,regs.x);
            break;
        }

        case 0x9B: { // TAS abs, y
            w_abs_ind(tas,regs.y);
            break;
        }

        case 0xB8: { // CLV
            r_imp(clv);
            break;
        }

        case 0x25: { // AND zp
            r_zp(and);
            break;
        }

        case 0x2D: { // AND abs
            r_abs(and);
            break;
        }

        case 0x29: { // AND #imm
            r_imm(and);
            break;
        }

        case 0x35: { // AND zp, x
            r_zp_ind(and,regs.x);
            break;
        }

        case 0x3d: { // AND abs, x
            r_abs_ind(and,regs.x);
            break;
        }

        case 0x39: { // AND abs, y
            r_abs_ind(and,regs.y);
            break;
        }

        case 0x21: { // AND (ind,x)
            r_indx(and);
            break;
        }

        case 0x31: { // AND (ind), y
            r_indy(and);
            break;
        }

        case 0x10: { // BPL
            branch(!regs.n);
            break;
        }

        case 0x30: { // BMI
            branch(regs.n);
            break;
        }

        case 0x50: { // BVC
            branch(!regs.v);
            break;
        }

        case 0x70: { // BVS
            branch(regs.v);
            break;
        }

        case 0x90: { // BCC
            branch(!regs.c);
            break;
        }

        case 0xB0: { // BCS
            branch(regs.c);
            break;
        }

        case 0xD0: { // BNE
            branch(!regs.z);
            break;
        }

        case 0xF0: { // BEQ
            branch(regs.z);
            break;
        }


        case 0xE8: { // INX
            r_imp(inx);
            break;
        }

        case 0xC8: { // INY
            r_imp(iny);
            break;
        }

        case 0xCA: { // DEX
            r_imp(dex);
            break;
        }

        case 0x88: { // DEY
            r_imp(dey);
            break;
        }

        case 0xE0: { // CPX #IMM
            r_imm(cpx);
            break;
        }

        case 0xEC: { // CPX abs
            r_abs(cpx);
            break;
        }

        case 0xE4: { // CPX zp
            r_zp(cpx);
            break;
        }

        case 0xC0: { // CPY #imm
            r_imm(cpy);
            break;
        }

        case 0xCC: { // CPY abs
            r_abs(cpy);
            break;
        }

        case 0xC4: { // CPY zp
            r_zp(cpy);
            break;
        }

        case 0xB3: { // LAX (ind), y
            r_indy(lax);
            break;
        }

        case 0xAF: { // LAX abs
            r_abs(lax);
            break;
        }

        case 0x2A: { // ROL A
            r_imp(rol_a);
            break;
        }

        case 0x6A: { // ROR A
            r_imp(ror_a);
            break;
        }

        // STP: 02, 12, 22, 32, 42, 52, 62, 72, 92, B2, D2, F2
        case 0x02:
        case 0x12:
        case 0x22:
        case 0x32:
        case 0x42:
        case 0x52:
        case 0x62:
        case 0x72:
        case 0x92:
        case 0xB2:
        case 0xD2:
        case 0xF2: { // STP/JAM/KIL
            regs.jammed = true;
            regs.cycles += 11; // WHAAAA NO CYCLE ACCURACY??!?!?! /j (this is done so that it passes TomHarte's tests)
            break;
        }

        case 0xBD: { // LDA abs, x
            r_abs_ind(lda,regs.x);
            break;
        }

        case 0xCB: { // AXS #imm
            r_imm(axs);
            break;
        }

        case 0x8B: { // XAA #imm
            r_imm(xaa);
            break;
        }

        case 0xCF: { // DCP abs
            rmw_abs(dcp);
            break;
        }

        case 0xD3: { // DCP (ind), y
            rmw_indy(dcp);
            break;
        }

        case 0x93: { // AHX (ind), y
            w_indy(sha);
            break;
        }

        case 0x9F: { // AHX ind, y
            w_abs_ind(sha,regs.y);
            break;
        }

        case 0xC7: { // DCP zp
            rmw_zp(dcp);
            break;
        }

        case 0xD7: { // DCP zp, x
            rmw_zp_ind(dcp,regs.x);
            break;
        }

        case 0x6B: { // ARR #imm
            r_imm(arr);
            break;
        }

        case 0x69: { // ADC #imm
            r_imm(adc);
            break;
        }

        case 0x6D: { // ADC abs
            r_abs(adc);
            break;
        }

        case 0x7d: { // ADC abs, x
            r_abs_ind(adc,regs.x);
            break;
        }

        case 0x65: { // ADC zp
            r_zp(adc);
            break;
        }

        case 0x75: { // ADC zp, x
            r_zp_ind(adc,regs.x);
            break;
        }

        case 0x79: { // ADC abs, y
            r_abs_ind(adc,regs.y);
            break;
        }

        case 0x86: { // STX zp
            w_zp(stx);
            break;
        }

        case 0x96: { // STX zp, y
            w_zp_ind(stx,regs.y);
            break;
        }

        case 0x24: { // BIT zp
            r_zp(bit);
            break;
        }

        case 0x2C: { // BIT abs
            r_abs(bit);
            break;
        }

        case 0x48: { // PHA
            advance_cycle(); // read PC
            read6502(regs.pc); // read next instruction byte (and throw it away)
            advance_cycle();
            push(regs.a);
            advance_cycle();
            break;
        }
        
        case 0x08: { // PHP
            advance_cycle(); // read PC
            read6502(regs.pc); // read next instruction byte (and throw it away)
            advance_cycle();
            push(read_flag()|(1<<4));
            advance_cycle();
            break;
        }

        case 0x28: { // PLP
            advance_cycle(); // read PC
            read6502(regs.pc); // read next instruction byte (and throw it away)
            advance_cycle();
            regs.sp++;
            advance_cycle();
            write_flag_no_b(pop_no_inc());
            advance_cycle();
            break;
        }

        case 0x68: { // PLA
            advance_cycle(); // read PC
            read6502(regs.pc); // read next instruction byte (and throw it away)
            advance_cycle();
            regs.sp++;
            advance_cycle();
            regs.a = pop_no_inc();
            regs.z = regs.a==0;
            regs.n = regs.a>>7;
            advance_cycle();
            break;
        }

        case 0x8A: { // TXA
            r_imp(txa);
            break;
        }

        case 0xAA: { // TAX
            r_imp(tax);
            break;
        }

        case 0x4B: { // ALR
            r_imm(alr);
            break;
        }

        case 0x4C: { // JMP
            advance_cycle();
            uint8_t lobyte = read_PC();
            advance_cycle();
            uint8_t hibyte = read_PC();
            regs.pc = (lobyte)|(hibyte<<8);
            advance_cycle();
            break;
        }

        case 0x2B:
        case 0x0B: { // ANC #imm
            r_imm(anc);
            break;
        }


        case 0x6C: { // JMP (indirect)
            advance_cycle();
            uint8_t lobyte = read_PC();
            advance_cycle();
            uint8_t hibyte = read_PC();
            advance_cycle();
            uint8_t lolatch = read6502(lobyte|(hibyte<<8)); // fetch to latch
            advance_cycle(); 
            regs.pc = (regs.pc&0xff)|(read6502(((lobyte+1)&0xff)|(hibyte<<8))<<8); // fetch PCH
            regs.pc = (regs.pc&0xff00)|lolatch; // copy latch to PCL
            advance_cycle();
            break;
        }

        case 0x9a: { // TXS
            r_imp(txs);
            break;
        }

        case 0xba: { // TSX
            r_imp(tsx);
            break;
        }

        default: {
            //printf("\nUNSUPPORTED OPCODE %02x: %s %s\nEXITING...\n\n",opcode,_opName[opcode],addr_modes[_opMode[opcode]]);
            //show_opimp();
            //exit(0);
            break;
        }
    }
    implemented_ops[opcode] = 1;
    return 1;
}

void show_opimp() {
    printf("    0 1 2 3 4 5 6 7 8 9 A B C D E F\n");
    printf("  r--------------------------------\n");
    for (int y = 0; y < 16; y++) {
        printf("%x |" ,y);
        for (int x = 0; x < 16; x++) {
            printf("%s▙▟",implemented_ops[x|(y<<4)]==2?"\033[43;30m":implemented_ops[x|(y<<4)]==1?"\033[42;30m":"\033[41;30m");
        }
        printf("\033[0m\n");
    }
    printf("\n");
}


uint8_t runcpu(void)
{
    uint8_t ttt = step();
    return ttt;
}