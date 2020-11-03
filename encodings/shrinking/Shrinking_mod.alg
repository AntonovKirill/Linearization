define len 64;

__in bit regA[22];
__in bit regB[23];

__out bit result[len];


void shift_regA() {
	int j;
	bit y = regA[21] ^ regA[20];
	for (j = 21; j > 0; j = j - 1) {
		regA[j] = regA[j - 1];
	}
	regA[0] = y;
}


void shift_regB() {
	int j;
	bit y = regB[22] ^ regB[21] ^ regB[20] ^ regB[7];
	for(j = 22; j > 0; j = j - 1) {
		regB[j] = regB[j - 1];
	}
	regB[0] = y;
}


void main() {
	int i;
	int j;
	int sum = 0;

	for (i = 0; i < 3 * len; i = i + 1) {
		shift_regA();
		shift_regB();
		sum = sum + regA[21];

		if (regA[21]) {
			result[sum - 1] = regB[22];
		}
	}
}
