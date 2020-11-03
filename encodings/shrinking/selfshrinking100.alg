define len 100;

__in bit reg[100];

__out bit result[len];

bit summ[8] = 0;

bit stream[len];

void shift_reg()
{
	bit y = reg[99] ^ reg[36];
	for(int j = 99; j > 0; j = j - 1)
	{
		reg[j] = reg[j - 1];
	}
	reg[0] = y;
}

void main()
{
	int i;

	for(i = 0; i < 3*len; i = i + 1)
	{
		shift_reg();
		shift_reg();
		summ = summ + reg[99];

		for(int j = 0; j < len; j = j + 1)
		{
			if(reg[99] & (summ == j + 1))
				stream[j] = reg[98];
		}
	}

	for(i = 0; i < len; i = i + 1)
	{
		result[i] = stream[i];
	}
}
