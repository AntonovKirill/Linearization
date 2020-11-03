#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

using namespace std;

const int N = 4;

struct binset {
	int value;
	vector <int> a;

	binset(): value(-1) {}
	binset(vector <int> &a): value(-1), a(a) {}
	binset(vector <int> &&a): value(-1), a(a) {}
	
	int operator[] (int n) {
		return a[n];
	}
	
	int val() {
		if (value == -1) {
			value = 0;
			for (int x: a)
				value = 2 * value + x;
		}
		return value;
	}
};

struct lin {
	vector <int> a;

	lin(vector <int> a): a(a) {}
	
	int operator[] (int n) {
		return a[n];
	}

	int val(binset &b) {
		return (a[0] & b[0]) ^ (a[1] & b[1]) ^ (a[2] & b[2]) ^ a[3];
	}
};

vector<lin> equations;
vector<vector<binset>> sat_sets;

bool is_subset(vector <binset> &a, vector <binset> &b) {
	set <int> sa, sb;
	for (auto &bs: a)
		sa.insert(bs.val());
	for (auto &bs: b)
		sb.insert(bs.val());
	return includes(all(sa), all(sb));
}

int main() {

	ios::sync_with_stdio(0);
	cin.tie();
	cout.tie();

	sat_sets.resize(1 << N);
	for (int eq = 0; eq < (1 << N); ++eq) {
		vector <int> a(N);
		for (int i = 0; i < N; ++i)
			a[i] = (eq >> (N - i - 1)) & 1;
		equations.push_back((lin)(a));
		for (int n = 0; n < (1 << (N - 1)); ++n) {
			binset d({(n >> 2) & 1, (n >> 1) & 1, n & 1});
			if (equations[eq].val(d) == 1)
				sat_sets[eq].push_back(d);
		}
	}

	ifstream fin("learnts_by_key");
	ofstream fout("linear_equations");

	int n;
	while (fin >> n) {
		int x, y, z;
		vector <binset> bs(n);
		for (int i = 0; i < n; ++i) {
			fin >> x >> y >> z;
			binset b({x & 1, y & 1, z & 1});
			bs[i] = b;
		}
		for (int eq = 2; eq < (1 << N); ++eq) {
			x &= -2;
			y &= -2;
			z &= -2;

			if (equations[eq][0])
				x ^= equations[eq][3];
			else if (equations[eq][1])
				y ^= equations[eq][3];
			else // if (equations[eq][2])
				z ^= equations[eq][3];

			if (is_subset(bs, sat_sets[eq]))
				fout << (equations[eq][0] ? to_string(x) + " ": "") <<
						(equations[eq][1] ? to_string(y) + " " : "") <<
						(equations[eq][2] ? to_string(z) + " " : "") << "\n";
		}
	}

	fin.close();
	fout.close();

}
