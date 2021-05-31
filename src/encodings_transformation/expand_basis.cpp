#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

using namespace std;


/// УРАВНЕНИЕ and
/// x ^ y & z = 0 ( x = y & z )
/// ПОД x, y, z ПОНИМАЮТСЯ ПЕРЕМЕННЫЕ
/// С НОМЕРАМИ x, y, z СООТВЕТСТВЕННО
class AndEquation
{
public:
	int x;
	int y;
	int z;
	AndEquation() {}
	AndEquation(int x, int y, int z): x(x), y(y), z(z) {}
};

bool operator== (const AndEquation &a, const AndEquation &b)
{
	return a.y == b.y && a.z == b.z;
}

bool operator< (const AndEquation &a, const AndEquation &b)
{
	return (((a.y & 1) << 1) ^ (a.z & 1)) < (((b.y & 1) << 1) ^ (b.z & 1));
}


/// МНОЖЕСТВО ВСЕХ УРАВНЕНИЙ
vector<AndEquation> equations;
set<vector<int>> pattern_linear_constraints;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector<int> input_vars;
vector<int> output_vars;
set<int> all_vars_set;
vector<int> all_vars;


/// КОЛИЧЕСТВО УРАВНЕНИЙ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int and_equations_cnt = 0;


/// КОЛИЧЕСТВО ВСЕХ ПЕРЕМЕННЫХ И
/// ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int vars_cnt = 0;
int input_vars_cnt = 0;
int output_vars_cnt = 0;
int latches_cnt = 0;


/// ЧТЕНИЕ ПАРАМЕТРОВ ИЗ ТЕРМИНАЛА
void init(int argc, char *argv[],
		string &in_filename, string &out_filename,
		string &lin_filename)
{
	in_filename  = "";
	out_filename = "";
	lin_filename = "";

	for (int i = 1; i < (int) argc; ++i) {
		string param;
		int j = 0;

		for (; argv[i][j] != '=' && argv[i][j] != 0; ++j)
			param.insert(param.end(), argv[i][j]);

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help                               Вывод этой справки и выход.\n";
			cout << "  -i, --input <file>                       Файл с описанием И-Не графа.\n";
			cout << "  -o, --output <file>                      Файл для вывода упрощённого И-Не графа.\n";
			cout << "  -L, --linear <file>    default=linear    Файл дополнительных линейных ограничений.\n";
			exit(0);
		}
		else if (param == "--input" || param == "-i") {
			if (argv[i][j] == 0)
				in_filename = (string) (argv[++i]);
			else
				in_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--output" || param == "-o") {
			if (argv[i][j] == 0)
				out_filename = (string) (argv[++i]);
			else
				out_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--linear" || param == "-L") {
			if (argv[i][j] == 0)
				lin_filename = (string) (argv[++i]);
			else
				lin_filename = (string) (argv[i] + j + 1);
		}
		else {
			clog << "warning: void init(): unknown parameter: " << param << "\n";
			exit(0);
		}
	}

	if (in_filename.empty()) {
		cerr << "error: void init(): " <<
			"parameter -i (--input) is required\n"; // TODO: throw runtime_error
		exit(0);
	}
	if (out_filename.empty()) {
		cerr << "error: void init(): " <<
			"parameter -o (--output) is required\n";
		exit(0);
	}
	if (lin_filename.empty()) {
		lin_filename = "linear";
		clog << "warning: void init(): " <<
			"additional linear constraints filename has been set to default value \'linear\'\n";
	}
}


/********************
 * AIG-file reading *
 ********************/
/// HEADER INCLUDES NUMBER OF INPUT VARIABLES,
/// OUTPUT VARIABLES AND TOTAL NUMBER OF VARIABLES
void read_header(ifstream &fin)
{
	clog << " reading header ... ";
	clog.flush();

	string header;
	fin >> header;

	if (header == "aag") {
		fin >> vars_cnt >> input_vars_cnt >> latches_cnt >> output_vars_cnt >> and_equations_cnt;
	}
	else {
		cerr << "error: void read_header(): " <<
			"wrong input format: \'aag\' expected but \'" << header << "\' found\n";
		exit(0);
	}

	clog << "ok" << endl;
}


void read_input(ifstream &fin)
{
	clog << " reading input ... ";
	clog.flush();

	input_vars.clear();
	input_vars.resize(input_vars_cnt);

	for (int i = 0; i < input_vars_cnt; ++i) {
		int x;
		fin >> x;
		input_vars[i] = x;
		all_vars.push_back(x);
	}

	clog << "ok" << endl;
}


void read_output(ifstream &fin)
{
	clog << " reading output ... ";
	clog.flush();

	output_vars.resize(output_vars_cnt);

	for (int i = 0; i < output_vars_cnt; ++i) {
		int x;
		fin >> x;
		output_vars[i] = x;
		all_vars.push_back(x);
	}

	clog << "ok" << endl;
}


void read_equations(ifstream &fin)
{
	clog << " reading equations ... ";
	clog.flush();

	equations.resize(and_equations_cnt);
	for (int i = 0; i < and_equations_cnt; ++i) {
		int x, y, z;
		fin >> x >> y >> z;
		equations[i] = {x, min(y, z), max(y, z)};
		all_vars.push_back(x);
	}

	clog << "ok" << endl;
}


void read_aig(const string &in_filename)
{
	clog << "reading from \'" << in_filename << "\' ..." << endl;
	ifstream fin(in_filename.data());

	read_header(fin);
	read_input(fin);
	read_output(fin);
	read_equations(fin);

	fin.close();
}


void read_linear_constraints(set<vector<int>> &linear_constraints,
		const string &filename)
{
	clog << "reading additional linear constraints from \'" << filename << "\' ... ";

	ifstream fin(filename.data());

	string line;
	stringstream ss;
	while (getline(fin, line)) {
		ss.clear();
		ss << line;
		int x, rem = 0;
		vector<int> equation;
		while (ss >> x) {
			equation.push_back(x & -2);
			rem ^= x & 1;
			all_vars_set.insert(x & -2);
		}
		if (equation.empty())
			continue;
		sort(all(equation));
		equation[0] ^= rem;
		linear_constraints.insert(equation);
	}

	fin.close();
	
	clog << "ok" << endl;
}


void find_xor_gates(set<vector<int>> &linear_constraints)
{
	clog << "finding xor gates ... ";
	
	std::vector<char> used(and_equations_cnt, 0), used_lin(and_equations_cnt, 0);
	std::vector<int> index(vars_cnt + 1, -1);
	
	for (int i = 0; i < and_equations_cnt; ++i)
		index[equations[i].x / 2] = i;
	
	map<int, int> gates_freq;
	for (auto &e: equations) {
		++gates_freq[e.y / 2];
		++gates_freq[e.z / 2];
	}

	int vars_cnt_new = vars_cnt;

	for (int i = 0; i < and_equations_cnt; ++i) {
		auto e = equations[i];

		int iy = index[e.y / 2], iz = index[e.z / 2];

		if (iy != -1 && iz != -1) {
			auto ey = equations[iy], ez = equations[iz];
			// int y = ey.y / 2, z = ey.z / 2;

			// xor operator condition
			if (ey.y / 2 == ez.z / 2)
				swap(ez.y, ez.z);

			if (ez.y / 2 == ey.y / 2 && ez.z / 2 == ey.z / 2) {
				// ey.y = y, ey.z = z,
				// ez.y = y ^ a, ez.z = z ^ b,
				// e.y = ey.x ^ c = yz ^ c,
				// e.z = ez.x ^ d = (y ^ a)(z ^ b) ^ d,
				// e.x = e.y & e.z =
				// = yz((1 ^ a)(1 ^ b) ^ c ^ d) ^ bcy ^ acz ^ c(ab ^ d)
				char a = ez.y ^ ey.y, b = ez.z ^ ey.z,
					c = e.y ^ ey.x, d = e.z ^ ez.x;
				
				used[i] = 1;
				if (gates_freq[e.z / 2] == 1)
					used[iz] = 1;
				used_lin[i] = 1;

				vector<int> lin;
				lin.push_back(e.x ^ (c & ((a & b) ^ d)));
				
				if (b & c)
					lin.push_back(ey.y);
				if (a & c)
					lin.push_back(ey.z);

				if (((1 ^ a) & (1 ^ b)) ^ c ^ d)
					lin.push_back(ey.x);
				else
					used[iy] = 1;

				linear_constraints.insert(lin);

				if (index[ey.y / 2] != -1 && !used_lin[index[ey.y / 2]] && used[index[ey.y / 2]])
					std::clog << "lost variable: " << ey.y / 2 << std::endl;
				if (index[ey.z / 2] != -1 && !used_lin[index[ey.z / 2]] && used[index[ey.z / 2]])
					std::clog << "lost variable: " << ey.z / 2 << std::endl;

				continue;
			}

			// ternary conditional operator condition
			if ((ez.z ^ ey.y) == 1 || (ez.z ^ ey.z) == 1)
				swap(ez.y, ez.z);
			if ((ey.z ^ ez.y) == 1)
				swap(ey.y, ey.z);

			if ((ey.y ^ ez.y) == 1) {
				// ey.y = x, ey.z = y,
				// ez.y = x ^ 1, ez.z = z,
				// e.y = ey.x ^ a = xy ^ a,
				// e.z = ez.x ^ b = (x ^ 1)z ^ b,
				// u = by ^ az, v = xu,
				// e.x = e.y & e.z = bxy ^ axz ^ az ^ ab =
				// = x(by ^ az) ^ az ^ ab = v ^ az ^ ab
				char a = e.y ^ ey.x, b = e.z ^ ez.x;

				used[i] = 1;

				vector<int> lin;
				lin.push_back(e.x ^ (a & b));

				if (a == 0 && b == 0) { // v = 0
					// do nothing
				}
				else if (a == 0) { // v ^ az = xy
					lin.push_back(ey.x);
				}
				else if (b == 0) { // v ^ az = (x ^ 1)z
					lin.push_back(ez.x);
				}
				else { // u = y ^ z, v ^ az = xu ^ z
					lin.push_back(ez.z);

					int u;
					if (gates_freq[e.y / 2] == 1) {
						u = ey.x;
						// erase ey.x = ey.y & ey.z, add u = ey.z + ez.z
						used[iy] = 1;
						used_lin[iy] = 1;
						linear_constraints.insert({u, ey.z, ez.z});
					}
					else {
						u = 2 * ++vars_cnt_new;
						// add u = ey.z + ez.z
						linear_constraints.insert({u, ey.z, ez.z});
					}

					int v;
					if (gates_freq[e.z / 2] == 1) {
						v = ez.x;
						// erase ez.x = ez.y & ez.z, add v = u & ey.y
						equations[iz] = {v, u, ey.y};
					}
					else {
						v = 2 * ++vars_cnt_new;
						// add v = u & ey.y
						equations.push_back({v, u, ey.y});
					}

					lin.push_back(v);
				}

				used_lin[i] = 1;
				linear_constraints.insert(lin);

				if (index[ey.y / 2] != -1 && !used_lin[index[ey.y / 2]] && used[index[ey.y / 2]])
					std::clog << "lost variable: " << ey.y / 2 << std::endl;
				if (index[ey.z / 2] != -1 && !used_lin[index[ey.z / 2]] && used[index[ey.z / 2]])
					std::clog << "lost variable: " << ey.z / 2 << std::endl;

				continue;
			}
		}
	}

	std::vector<AndEquation> equations_new;
	for (int i = 0; i < and_equations_cnt; ++i) {
		if (!used[i])
			equations_new.push_back(equations[i]);
	}
	
	std::clog << "new aig:\n";
	std::clog << "aag " << vars_cnt << " " << input_vars_cnt << " " << latches_cnt << " "
		<< output_vars_cnt << " " << equations_new.size() << "\n";
	for (auto &e: equations_new)
		std::clog << e.x << " " << e.y << " " << e.z << "\n";
	std::clog << "\n";
	
	std::clog << "new linear constraints:\n";
	for (auto &v: linear_constraints) {
		for (auto x: v)
			clog << x << " ";
		std::clog << "\n";
	}
	std::clog << std::endl;
	
	std::set<int> all_vars_new;
	
	for (auto &e: equations_new) {
		all_vars_new.insert(e.x / 2);
		all_vars_new.insert(e.y / 2);
		all_vars_new.insert(e.z / 2);
	}
	
	for (auto &v: linear_constraints) {
		for (auto x: v)
			all_vars_new.insert(x / 2);
	}
	
	clog << "all vars new size: " << all_vars_new.size() << std::endl;
	
	
	// std::vector<int> index_new(vars_cnt + 1, -1);
	
	// for (int i = 0; i < and_equations_cnt; ++i)
		// index_new[equations[i].x / 2] = i;
}


/*****************
 * main function *
 *****************/
int main(int argc, char *argv[])
{

	string in_filename, out_filename;
	string linear_constraints_filename;

	init(argc, argv, in_filename, out_filename,
		linear_constraints_filename);

	read_aig(in_filename);

	find_xor_gates(pattern_linear_constraints);

}
