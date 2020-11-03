// 3 shift registers with total length of 288 bits
define len  288;
define lenA 93;
define lenB 84;
define lenC 111;
define lenK 80;
define stream_len 300;
define init_steps 1152;

__in bit key[lenK];
__in bit iv [lenK];

bit reg[len];

__out bit stream[stream_len];

// function to shift registers
void shift_regs()
{
	bit t1 = reg[65] ^ reg[90]&reg[91] ^ reg[92] ^ reg[170];
	bit t2 = reg[161] ^ reg[174]&reg[175] ^ reg[176] ^ reg[263];
	bit t3 = reg[242] ^ reg[285]&reg[286] ^ reg[287] ^ reg[68];

	int i;
	for (i = lenA - 1; i > 0; i = i - 1)
		reg[i] = reg[i - 1];
	reg[0] = t3;
	for (i = lenA + lenB - 1; i > lenA; i = i - 1)
		reg[i] = reg[i - 1];
	reg[lenA] = t1;
	for(i = lenA + lenB + lenC - 1; i > lenA + lenB; i = i - 1)
		reg[i] = reg[i - 1];
	reg[lenA + lenB] = t2;
}

// initialization
void init()
{
	int i;
	for(i = 0; i < lenK; i = i + 1) {
		reg[i]        = key[i];
		reg[lenA + i] = iv [i];
	}
	reg[287] = 1;
	reg[286] = 1;
	reg[285] = 1;

	for(i = 0; i < init_steps; i = i + 1)
		shift_regs();
}

void main()
{

	init();
	for(int i = 0; i < stream_len; i = i + 1) {
		__mem bit t1 = reg[65] ^ reg[92] ^ reg[161];
		__mem bit t2 = reg[176] ^ reg[242] ^ reg[287];
		stream[i] = t1 ^ t2;
		shift_regs();
	}

}
