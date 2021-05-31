define len 64;

__in bit regA[19];
__in bit regB[22];
__in bit regC[23];
__out bit result[len];

bit shift_lfsrA()
{
	bit x = regA[18];
	bit y = regA[18] ^ regA[17] ^ regA[16] ^ regA[13];
	for (int j = 18; j > 0; j = j - 1)
		regA[j] = regA[j - 1];
	regA[0] = y;
	return x;
}

bit shift_lfsrB()
{
	bit x = regB[21];
	bit y = regB[21] ^ regB[20];
	for (int j = 21; j > 0; j = j - 1)
		regB[j] = regB[j - 1];
	regB[0] = y;
	return x;
}

bit shift_lfsrC()
{
	bit x = regC[22];
	bit y = regC[22] ^ regC[21] ^ regC[20] ^ regC[7];
	for (int j = 22; j > 0; j = j - 1)
		regC[j] = regC[j - 1];
	regC[0] = y;
	return x;
}

bit majority(bit A, bit B, bit C)
{
    return (A ^ B) & (A ^ C);
}

void main()
{
	int midA = 8;
	int midB = 10;
	int midC = 10;
	for (int i = 0; i < len; i = i + 1) {
		bit majA = majority(regA[midA], regB[midB], regC[midC]);
		bit majB = majority(regB[midB], regA[midA], regC[midC]);
		bit majC = majority(regC[midC], regB[midB], regA[midA]);
		if (!majA)
			shift_lfsrA();
		if(!majB)
			shift_lfsrB();
		if(!majC)
			shift_lfsrC();
		result[i] = regA[18] ^ regB[21] ^ regC[22];
	}
}
