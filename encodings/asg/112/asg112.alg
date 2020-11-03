define len 128;

__in bit regA[35];
__in bit regB[38];
__in bit regC[39];
__out bit result[len];


void shift_regA()
{
	bit y = regA[34] ^ regA[32];
	for(int j = 34; j > 0; j = j - 1)
		regA[j] = regA[j - 1];
	regA[0] = y;
}


void shift_regB()
{
	bit y = regB[37] ^ regB[5] ^ regB[4] ^ regB[0];
	for(int j = 37; j > 0; j = j - 1)
		regB[j] = regB[j - 1];
	regB[0] = y;	
}


void shift_regC() 
{
	bit y = regC[38] ^ regC[34];
	for(int j = 38; j > 0; j = j - 1)
		regC[j] = regC[j - 1];
	regC[0] = y;
}

void main()
{

	for (int i = 0; i < len; i = i + 1) {
		// result[i] = regB[37] ^ regC[38];
		shift_regA();
		if(regA[34] & 1)
			shift_regB();
		if(!(regA[34] & 1))
			shift_regC();
		result[i] = regB[37] ^ regC[38];
	}
}
