define len 160;

__in bit regA[41];
__in bit regB[43];
__in bit regC[44];
__out bit result[len];

void shift_regA()
{
	bit x = regA[40];
	bit y = regA[40] ^ regA[37];
	for (int j = 40; j > 0; j = j - 1)
		regA[j] = regA[j - 1];
	regA[0] = y;	
}

void shift_regB()
{
	bit x = regB[42];
	bit y = regB[42] ^ regB[41] ^ regB[37] ^ regB[36];
	for (int j = 42; j > 0; j = j - 1)
		regB[j] = regB[j-1];
	regB[0] = y;
}

void shift_regC()
{
	bit x = regC[43];
	bit y = regC[43] ^ regC[42] ^ regC[37] ^ regC[16];
	for (int j = 43; j > 0; j = j - 1)
		regC[j] = regC[j - 1];
	regC[0] = y;
}

void main()
{

	for (int i = 0; i < len; i = i + 1) {
		// result[i] = regB[42] ^ regC[43];
		shift_regA();
		if(regA[40] & 1)
			shift_regB();
		if(!(regA[40] & 1))
			shift_regC();
		result[i] = regB[42] ^ regC[43];
	}

}
