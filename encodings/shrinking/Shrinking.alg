define len 64;

__in bit regA[22];
__in bit regB[23];

__out bit result[len];

bit summ[7] = 0;

bit stream[len];

void shift_regA()
{
	bit y = regA[21] ^ regA[20];
	for(int j = 21; j > 0; j = j - 1)
	{
		regA[j] = regA[j - 1];
	}
	regA[0] = y;
}

void shift_regB()
{
	bit y = regB[22] ^ regB[21] ^ regB[20] ^ regB[7];
	for(int j = 22; j > 0; j = j - 1)
	{
		regB[j] = regB[j - 1];
	}
	regB[0] = y;
}

void main()
{
	int i;

	for(i = 0; i < 3*len; i = i + 1)
	{
		shift_regA();
		shift_regB();
		summ = summ + regA[21];

		for(int j = 0; j < len; j = j + 1)
		{
			if(regA[21] & (summ == j + 1))
				stream[j] = regB[22];
		}
	}

	for(i = 0; i < len; i = i + 1)
	{
		result[i] = stream[i];
	}
}
