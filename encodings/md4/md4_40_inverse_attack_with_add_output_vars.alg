__in bit M[16][32];

__out bit out_vars[12][32]; // vars in lambda
__out bit Hash[4][32];


bit F(bit X[32], bit Y[32], bit Z[32])
{
	return (X&Y) | (!X&Z);
}

bit G(bit X[32], bit Y[32], bit Z[32])
{
	return X&Y | X&Z | Y&Z;
}

bit H(bit X[32], bit Y[32], bit Z[32])
{
	return X ^ Y ^ Z;
}

// Round 1, (a + F(b, c, d) + M) <<< s
bit FF(bit a[32], bit b[32], bit c[32], bit d[32], bit M[32], int s)
{
	a = sum(sum(a, F(b, c, d), 32), M, 32);
	return (a <<< s);
}

// Round 2, (a + G(b, c, d) + M + 0x5A827999) <<< s
bit GG(bit a[32], bit b[32], bit c[32], bit d[32], bit M[32], int s)
{
	a = sum(sum(sum(a, G(b, c, d), 32), M, 32), 0x5A827999, 32);
	return (a <<< s);
}

// Round 3, (a + H(b, c, d) + M + 0x6ED9EBA1) <<< s
bit HH(bit a[32], bit b[32], bit c[32], bit d[32], bit M[32], int s)
{
	a = sum(sum(sum(a, H(b, c, d), 32), M, 32), 0x6ED9EBA1, 32);
	return (a <<< s);
}


void main()
{


	bit A[32] = 0x67452301;
	bit B[32] = 0xEFCDAB89;
	bit C[32] = 0x98BADCFE;
	bit D[32] = 0x10325476;


	bit a[32] = A;
	bit b[32] = B;
	bit c[32] = C;
	bit d[32] = D;
	
	// Steps from 1--16
	a = FF(a, b, c, d, M[0], 3);
	d = FF(d, a, b, c, M[1], 7);
	c = FF(c, d, a, b, M[2], 11);
	b = FF(b, c, d, a, M[3], 19);

	a = FF(a, b, c, d, M[4], 3);
	d = FF(d, a, b, c, M[5], 7);
	c = FF(c, d, a, b, M[6], 11);
	b = FF(b, c, d, a, M[7], 19);
	

	a = FF(a, b, c, d, M[8], 3);
	d = FF(d, a, b, c, M[9], 7);
	c = FF(c, d, a, b, M[10], 11);
	b = FF(b, c, d, a, M[11], 19);

	
	a = FF(a, b, c, d, M[12], 3); 	
	d = FF(d, a, b, c, M[13], 7); 	out_vars[0] = d;
	c = FF(c, d, a, b, M[14], 11); 	out_vars[1] = c;
	b = FF(b, c, d, a, M[15], 19);

	// Steps from 17--32
	a = GG(a, b, c, d, M[0], 3); 	out_vars[2] = a;
	d = GG(d, a, b, c, M[4], 5); 	out_vars[3] = d;
	c = GG(c, d, a, b, M[8], 9); 	out_vars[4] = c;
	b = GG(b, c, d, a, M[12], 13);

	
	a = GG(a, b, c, d, M[1], 3); 	out_vars[5] = a;
	d = GG(d, a, b, c, M[5], 5); 	out_vars[6] = d;
	c = GG(c, d, a, b, M[9], 9); 	out_vars[7] = c;
	b = GG(b, c, d, a, M[13], 13);

	a = GG(a, b, c, d, M[2], 3); 	out_vars[8] = a;
	d = GG(d, a, b, c, M[6], 5); 	out_vars[9] = d;
	c = GG(c, d, a, b, M[10], 9); 	out_vars[10] = c;
	b = GG(b, c, d, a, M[14], 13);


	a = GG(a, b, c, d, M[3], 3); 	out_vars[11] = a;
	d = GG(d, a, b, c, M[7], 5); 	
	c = GG(c, d, a, b, M[11], 9); 	
	b = GG(b, c, d, a, M[15], 13);

	// Steps from 33--48
	a = HH(a, b, c, d, M[0], 3);
	d = HH(d, a, b, c, M[8], 9);
	c = HH(c, d, a, b, M[4], 11);
	b = HH(b, c, d, a, M[12], 15);

	a = HH(a, b, c, d, M[2], 3);
	d = HH(d, a, b, c, M[10], 9);
	c = HH(c, d, a, b, M[6], 11);
	b = HH(b, c, d, a, M[14], 15);

	// a = HH(a, b, c, d, M[1], 3);
	// d = HH(d, a, b, c, M[9], 9);
	// c = HH(c, d, a, b, M[5], 11);
	// b = HH(b, c, d, a, M[13], 15);

	// a = HH(a, b, c, d, M[3], 3);
	// d = HH(d, a, b, c, M[11], 9);
	// c = HH(c, d, a, b, M[7], 11);
	// b = HH(b, c, d, a, M[15], 15);

	
	Hash[0] = a;
	Hash[1] = b;
	Hash[2] = c;
	Hash[3] = d;
	// assert(!Hash);

}