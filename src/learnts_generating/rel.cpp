#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

using namespace std;

int num_shift = 0;

/* x + yz = 0 */
class gate {
public:
	int x, y, z;
	gate(): x(0), y(0), z(0) {}
	gate(int x, int y, int z): x(x), y(y), z(z) {}
};

void read_header(istream &in, int &vars_count, int &input_vars_count, int &output_vars_count, int &gates_count) {
	string header;
	in >> header;

	if (header == "aag") {
		in >> vars_count >> input_vars_count >> output_vars_count >> output_vars_count >> gates_count;
	}
	else {
		throw logic_error("wrong input format: `aag` expected but `" + header + "` found");
	}

}

void read_input_vars(istream &in, const int &input_vars_count, vector <int> &input_vars) {
	input_vars.clear();
	input_vars.resize(input_vars_count);

	for (int i = 0; i < input_vars_count; ++i) {
		int x;
		in >> x;
		x /= 2;
		input_vars[i] = x;
	}
}

void read_output_vars(istream &in, const int &output_vars_count, vector <int> &output_vars, vector <bool> &output_vars_inv) {
	output_vars.clear();
	output_vars_inv.clear();
	output_vars.resize(output_vars_count);
	output_vars_inv.resize(output_vars_count);

	for (int i = 0; i < output_vars_count; ++i) {
		int x;
		in >> x;
		bool inv = x & 1;
		x /= 2;
		output_vars[i] = x;
		output_vars_inv[i] = inv;
	}
}

void read_gates(istream &in, const int &gates_count, vector <gate> &gates) {
	gates.clear();
	gates.resize(gates_count);
	for (int i = 0; i < gates_count; ++i) {
		int x, y, z;
		in >> x >> y >> z;
		gates[i] = {x, y, z};
	}
}


void read_aig(const string &in, int &vars_count, int &input_vars_count, int &output_vars_count, int &gates_count, 
	vector <int> &input_vars, vector <int> &output_vars, vector <bool> &output_vars_inv, vector <gate> &gates)
{
	ifstream fin(in.data());

	read_header(fin, vars_count, input_vars_count, output_vars_count, gates_count);
	read_input_vars(fin, input_vars_count, input_vars);
	read_output_vars(fin, output_vars_count, output_vars, output_vars_inv);
	read_gates(fin, gates_count, gates);

	fin.close();
}

int parent(const int &x) {
	return x / 2 - num_shift;
}

int signed_half(const int &x) {
	return (x & 1 ? -(x / 2) : x / 2);
}

void print(ostream &out, set <int> vars_set) {
	vector <int> vars(all(vars_set));
	int n = vars.size();
	for (int a = 0; a < (1 << n); ++a) {
		for (int i = 0; i < n; ++i)
			out << ((a >> i) & 1 ? -vars[i] : vars[i]) << " ";
		out << "0\n";
	}
}

void print_gate(ostream &out, vector <int> vars) {
	int n = 3;
	vector <int> as = {0, 1, 2, 7};
	for (int a: as) {
		for (int i = 0; i < n; ++i)
			out << ((a >> (n - i - 1)) & 1 ? -vars[i] : vars[i]) << " ";
		out << "0\n";
	}
}

int main(int argc, char *argv[]) {

	string in_filename, out_filename;
	in_filename = (string) (argv[1]);
	out_filename = (string) (argv[2]);
	ofstream fout(out_filename.data());

	int vars_count, input_vars_count, output_vars_count, gates_count;
	vector <int> input_vars, output_vars;
	vector <bool> output_vars_inv;
	vector <gate> gates;

	read_aig(in_filename, vars_count, input_vars_count, output_vars_count, gates_count, input_vars, output_vars, output_vars_inv, gates);
	num_shift = input_vars_count + 1;

	for (auto &g: gates) {
		set <int> vars;
		vector <int> vars_v;

		vars_v = {signed_half(g.x), signed_half(g.y), signed_half(g.z)};
		print_gate(fout, vars_v);
		/*
		if (parent(g.y) >= 0 && parent(g.z) >= 0) {
			gate g1 = gates[parent(g.y)], g2 = gates[parent(g.z)];
			vars = {parent(g.x), parent(g1.y), parent(g1.z), parent(g2.y), parent(g2.z)};
			if (vars.size() <= 3)
				print(fout, vars);
			// vars = {parent(g.x), parent(g1.x), parent(g2.y), parent(g2.z)};
		}
		
		if (parent(g.y) >= 0) {
			gate g1 = gates[parent(g.y)];
			set <int> vars = {parent(g.x), parent(g1.y), parent(g1.z), parent(g.z)};
			if (vars.size() <= 3)
				print(fout, vars);
		}
		
		if (parent(g.z) >= 0) {
			gate g2 = gates[parent(g.z)];
			set <int> vars = {parent(g.x), parent(g.y), parent(g2.y), parent(g2.z)};
			if (vars.size() <= 3)
				print(fout, vars);
		}
		*/
	}

}
