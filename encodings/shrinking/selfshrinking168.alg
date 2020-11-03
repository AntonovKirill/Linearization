define len 168;

__in bit reg[168];

__out bit result[len];

bit summ[8] = 0;

bit stream[len];

void shift_reg()
{
	bit y = reg[167] ^ reg[16] ^ reg[14] ^ reg[1];
	for(int j = 167; j > 0; j = j - 1)
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
		summ = summ + reg[167];

		for(int j = 0; j < len; j = j + 1)
		{
			if(reg[167] & (summ == j + 1))
				stream[j] = reg[166];
		}
	}

	for(i = 0; i < len; i = i + 1)
	{
		result[i] = stream[i];
	}
}
