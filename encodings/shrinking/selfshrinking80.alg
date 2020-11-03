define len 80;

__in bit reg[80];

__out bit result[len];

bit summ[8] = 0;

bit stream[len];

void shift_reg()
{
	bit y = reg[79] ^ reg[37] ^ reg[36] ^ reg[0];
	for(int j = 79; j > 0; j = j - 1)
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
		summ = summ + reg[79];

		for(int j = 0; j < len; j = j + 1)
		{
			if(reg[79] & (summ == j + 1))
				stream[j] = reg[78];
		}
	}

	for(i = 0; i < len; i = i + 1)
	{
		result[i] = stream[i];
	}
}
