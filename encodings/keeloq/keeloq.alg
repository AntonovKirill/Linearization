define len 32;

__in bit data[len];
__in bit key[64];
__out bit res[len];

bit F(bit a, bit b, bit c, bit d, bit e)
{
	return d ^ e ^ a&c ^ a&e ^ b&c ^ b&e ^ c&d ^ d&e ^ a&d&e ^ a&c&e ^ a&b&d ^ a&b&c;
}

void main()
{

	for (int i = 0; i < 528; i = i + 1) {
		__mem bit f = F(data[31], data[26], data[20], data[9], data[1]);
		__mem bit g = f ^ data[0] ^ data[16] ^ key[i % 64];
		for (int j = 0; j < 31; j = j + 1)
			data[j] = data[j + 1];
		data[31] = g;
	}
	res = data;

}