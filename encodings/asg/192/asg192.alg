define len 200;

__in bit regA[61];
__in bit regB[64];
__in bit regC[67];
__out bit result[len];

bit shift_regA()
{
	bit x = regA[60];
	bit y = regA[44] ^ regA[45] ^ regA[59] ^ regA[60];
	for (int j = 60; j > 0; j = j - 1)
		regA[j] = regA[j - 1];
	regA[0] = y;
	return x;
}

bit shift_regB()
{
	bit x = regB[63];
	bit y = regB[59] ^ regB[60] ^ regB[62] ^ regB[63];
	for (int j = 63; j > 0; j = j - 1)
		regB[j] = regB[j - 1];
	regB[0] = y;
	return x;
}

bit shift_regC()
{
	bit x = regC[66];
	bit y = regC[56] ^ regC[57] ^ regC[65] ^ regC[66];
	for (int j = 66; j > 0; j = j - 1)
		regC[j] = regC[j - 1];
	regC[0] = y;
	return x;
}

void main()
{

	for (int i = 0; i < len; i = i + 1) {
		// result[i] = regB[63] ^ regC[66];
		shift_regA();
		if (regA[60] & 1)
			shift_regB();
		if (!(regA[60] & 1))
			shift_regC();
		result[i] = regB[63] ^ regC[66];
	}

}
