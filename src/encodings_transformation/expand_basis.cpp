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
// map <vector <int>, set <vector <char>>> pattern_learnts;


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


/***********************
 * general subprograms *
 **********************/
template <typename T>
void order(vector<T> &v) {
	sort(all(v));
	v.resize(unique(all(v)) - v.begin());
	v.shrink_to_fit();
}


void equations_xor(vector<int> e1, vector<int> e2,
		vector<int> &res)
{
	char b = 0;
	if (!e1.empty()) {
		b ^= e1[0];
		e1[0] &= -2;
	}
	if (!e2.empty()) {
		b ^= e2[0];
		e2[0] &= -2;
	}
	b &= 1;
	set_symmetric_difference(all(e1), all(e2), back_inserter(res));
	if (b) {
		if (res.empty())
			throw runtime_error("error: void equations_xor(): 0 == 1");
		res[0] ^= 1;
	}
}


void reduce_constraints(set<vector<int>> &linear_constraints)
{
	vector<vector<int>> new_lc;
	for (auto e: linear_constraints) {
		char b = 0;
		for (auto &x: e) {
			b ^= x & 1;
			x &= -2;
		}
		sort(all(e));
		e[0] ^= b;
		new_lc.push_back(e);
	}
	linear_constraints.clear();
	linear_constraints.insert(all(new_lc));
}


void resize_all(vector<vector<char> > &vec, int cnt)
{
	// #pragma omp parallel for
	for (int i = 0; i < (int) vec.size(); ++i) {
		vec[i].resize(cnt);
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

	// std::vector<std::vector<int>> graph;
	for (int i = 0; i < and_equations_cnt; ++i) {
		auto e = equations[i];

		if (!(e.y & 1) || !(e.z & 1))
			continue;

		int x = e.x / 2, y_ = e.y / 2, z_ = e.z / 2;
		int iy = index[y_], iz = index[z_];
		
		auto ey = equations[iy], ez = equations[iz];
		
		if (iy != -1 && iz != -1) {
			int y = ey.y / 2, z = ey.z / 2;
			if (ez.y / 2 == y && ez.z / 2 == z) {
				used[i] = 1;
				used[iy] = 1;
				used[iz] = 1;
				
				used_lin[i] = 1;

				if (!((ey.y ^ ey.z) & 1))
					linear_constraints.insert({2 * x, 2 * y, 2 * z});
				else
					linear_constraints.insert({2 * x ^ 1, 2 * y, 2 * z});
				
				if (index[y] != -1 && !used_lin[index[y]] && used[index[y]])
					std::clog << "lost variable: " << y << std::endl;
				if (index[z] != -1 && !used_lin[index[z]] && used[index[z]])
					std::clog << "lost variable: " << z << std::endl;
			}
		}
	}

	std::vector<AndEquation> equations_new;
	for (int i = 0; i < and_equations_cnt; ++i) {
		if (!used[i])
			equations_new.push_back(equations[i]);
	}
	
	std::clog << "new aig:\n";
	std::clog << vars_cnt << " " << input_vars_cnt << " " << latches_cnt << " "
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
		// index[equations[i].x / 2] = i;
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
