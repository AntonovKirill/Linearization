define len 76;

__in bit regA[23];
__in bit regB[24];
__in bit regC[25];
__out bit result[len];

void shift_regA()
{
	bit y = regA[22] ^ regA[17];
	for (int j = 22; j > 0; j = j - 1)
		regA[j] = regA[j - 1];
	regA[0] = y;
}

void shift_regB()
{
	bit y = regB[23] ^ regB[22] ^ regB[21] ^ regB[16];
	for (int j = 23; j > 0; j = j - 1)
		regB[j] = regB[j - 1];
	regB[0] = y;
}

void shift_regC()
{
	bit y = regC[24] ^ regC[21];
	for (int j = 24; j > 0; j = j - 1)
		regC[j] = regC[j - 1];
	regC[0] = y;
}

void main()
{

	for (int i = 0; i < len; i = i + 1) {
		// result[i] = regB[23] ^ regC[24];
		shift_regA();
		if(regA[22] & 1)
			shift_regB();
		if(!(regA[22] & 1))
			shift_regC();
		result[i] = regB[23] ^ regC[24];
	}

}
