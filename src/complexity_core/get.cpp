#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

typedef long long ll;

using namespace std;

int N = 0;
int points_cnt = 0;

random_device rd;
mt19937 generator(rd());

mutex mtx, log_mtx, gen_mtx;


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


class Bit
{
public:
	int n;
	int bit;
	
	Bit() {}
	Bit(int n, int b): n(n), bit(b) {}
};

bool operator== (const Bit &a, const Bit &b)
{
	return a.n == b.n && a.bit == b.bit;
}

bool operator< (const Bit &a, const Bit &b)
{
	return a.n < b.n || (a.n == b.n && a.bit < b.bit);
}


/// МНОЖЕСТВО ВСЕХ УРАВНЕНИЙ
vector<AndEquation> equations;
set <vector <int>> pattern_linear_constraints;
map <vector <int>, set <vector <char>>> pattern_learnts;
map <vector <Bit>, vector <vector <int>>> lin_table;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector <int> key_vars;
vector <int> iv_vars;
vector <int> input_vars;
vector <int> output_vars;
vector <int> guessed_vars;
vector <int> core_vars;
set <int> all_vars_set;
vector <int> all_vars;


/// КОЛИЧЕСТВО УРАВНЕНИЙ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int and_equations_cnt = 0;


/// КОЛИЧЕСТВО ВСЕХ ПЕРЕМЕННЫХ И
/// ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int vars_cnt = 0;
int input_vars_cnt = 0;
int key_vars_cnt = 0;
int iv_vars_cnt = 0;
int output_vars_cnt = 0;
int guessed_vars_cnt = 0;
int latches_cnt = 0;
int core_vars_cnt = 0;


map <vector <char>, double> complexity_set;
map <vector <char>, double> probability_set;

/// вектор значений переменных для всех выборок
vector<vector<char>> interp;


/// ЧТЕНИЕ ПАРАМЕТРОВ ИЗ ТЕРМИНАЛА
void init(int argc, char *argv[],
		string &in_filename, string &out_filename,
		string &sub_filename, string &core_filename,
		string &lin_filename, string &learnts_filename,
		string &start_point_filename, string &lin_table_filename)
{
	in_filename          = "";
	out_filename         = "";
	sub_filename         = "";
	core_filename        = "";
	lin_filename         = "";
	learnts_filename     = "";
	start_point_filename = "";
	lin_table_filename   = "";

	for (int i = 1; i < (int) argc; ++i) {
		string param;
		int j = 0;

		for (; argv[i][j] != '=' && argv[i][j] != 0; ++j)
			param.insert(param.end(), argv[i][j]);

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help                                             Вывод этой справки и выход.\n";
			cout << "  -i, --input <file>                                     Файл с описанием И-Не графа.\n";
			cout << "  -o, --output <file>                                    Файл для вывода упрощённого И-Не графа.\n";
			cout << "  -N, --sample-size <size>                               Размер выборки.\n";
			cout << "  -s, --substitution <file>    default=substitution      Файл с описанием подстановки.\n";
			cout << "  -c, --core <file>            default=core              Файл с описанием множества переменных ядра.\n";
			cout << "  -L, --linear <file>          default=linear            Файл дополнительных линейных ограничений.\n";
			cout << "  -l, --learnts <file>         default=learnts           Файл дополнительных дизъюнктов.\n";
			cout << "  -k, --key-size <size>        default=input_vars_cnt    Число бит ключа.\n";
			cout << "  -p, --start-point <file>     default=point             Файл с описанием начальной точки поиска.\n";
			cout << "  -lt, --lin-table <file>      default=lin-table         Файл с селекторными ограничениями.\n";
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
		else if (param == "--sample-size" || param == "-N") { /// СДЕЛАТЬ ПРОВЕРКУ НА НЕОТРИЦАТЕЛЬНОСТЬ
			if (argv[i][j] == 0)
				N = atoi(argv[++i]);
			else
				N = atoi(argv[i] + j + 1);
		}
		else if (param == "--key-size" || param == "-k") {
			if (argv[i][j] == 0)
				key_vars_cnt = (int) atoi(argv[++i]);
			else
				key_vars_cnt = (int) atoi(argv[i] + j + 1);
		}
		else if (param == "--substitution" || param == "-s") {
			if (argv[i][j] == 0)
				sub_filename = (string) (argv[++i]);
			else
				sub_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--core" || param == "-c") {
			if (argv[i][j] == 0)
				core_filename = (string) (argv[++i]);
			else
				core_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--linear" || param == "-L") {
			if (argv[i][j] == 0)
				lin_filename = (string) (argv[++i]);
			else
				lin_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--learnts" || param == "-l") {
			if (argv[i][j] == 0)
				learnts_filename = (string) (argv[++i]);
			else
				learnts_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--start-point" || param == "-p") {
			if (argv[i][j] == 0)
				start_point_filename = (string) argv[++i];
			else
				start_point_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--lin-table" || param == "-lt") {
			if (argv[i][j] == 0)
				lin_table_filename = (string) argv[++i];
			else
				lin_table_filename = (string) (argv[i] + j + 1);
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
	if (N <= 0) {
		cerr << "error: void init(): " <<
			"value of parameter -N (--sample-size) must be a positive number\n";
		exit(0);
	}
	if (key_vars_cnt < 0) {
		cerr << "error: void init(): " <<
			"value of parameter -k (--key-size) must be a positive number\n";
		exit(0);
	}
	if (sub_filename.empty()) {
		sub_filename = "substitution";
		clog << "warning: void init(): " <<
			"substitution filename has been set to default value \'substitution\'\n"; // TODO: оформить как warning (или тоже throw smth)
	}
	if (core_filename.empty()) {
		core_filename = "core";
		clog << "warning: void init(): " <<
			"core variables filename has been set to default value \'core\'\n";
	}
	if (lin_filename.empty()) {
		lin_filename = "linear";
		clog << "warning: void init(): " <<
			"additional linear constraints filename has been set to default value \'linear\'\n";
	}
	if (learnts_filename.empty()) {
		learnts_filename = "learnts";
		clog << "warning: void init(): " <<
			"learnts filename has been set to default value \'learnts\'\n";
	}
	if (lin_table_filename.empty()) {
		lin_table_filename = "lin-table";
		clog << "warning: void init(): " <<
			"lin_table_filename has been set to default value \'lin-table\'\n";
	}
}


/***********************
 * general subprograms *
 **********************/
template <typename T>
void order(vector <T> &v) {
	sort(all(v));
	v.resize(unique(all(v)) - v.begin());
	v.shrink_to_fit();
}

/*
void log_linear_constraints(const set <vector <int>> &linear_constraints)
{
	clog << "linear constraints:\n";
	for (auto &v: linear_constraints) {
		for (auto x: v)
			clog << x << " ";
		clog << "\n";
	}
}
*/
/*
void log_learnts(const map <vector <int>, set <vector <char>>> &learnts) {
	for (auto &p: learnts) {
		for (auto &v: p.second) {
			for (int i = 0; i < (int)p.first.size(); ++i)
				clog << (v[i] ? "-" : "") << p.first[i] / 2 << " ";
			clog << "0\n";
		}
	}
}
*/
/*
void print_linear_constraints(const set <vector <int>> &linear_constraints,
		const string &filename)
{
	ofstream fout(filename.data());
	for (auto &v: linear_constraints) {
		for (auto x: v)
			fout << x << " ";
		fout << "\n";
	}
	fout.close();
}
*/
/*
/// PRINTS learnts IN DIMACS FORMAT
void print_learnts(const map <vector <int>, set <vector <char>>> &learnts,
		const string &filename)
{
	ofstream fout(filename.data());
	for (auto &p: learnts) {
		int n = p.first.size();
		for (auto &v: p.second) {
			for (int i = 0; i < n; ++i) {
				int x = p.first[i] ^ v[i];
				fout << (x & 1 ? - x / 2 : x / 2) << " ";
			}
			fout << "0\n";
		}
	}
	fout.close();
}
*/

char add_learnts(map <vector <int>, set <vector <char>>> &learnts,
		vector <int> &key_value)
{
	sort(all(key_value));
	vector <int> key(key_value.size());
	vector <char> negations(key_value.size());
	for (int i = 0; i < (int)key_value.size(); ++i) {
		key[i] = key_value[i] & -2;
		negations[i] = key_value[i] & 1;
	}
	auto it = learnts.find(key);
	if (it == learnts.end()) {
		learnts[key].insert(negations);
		return 1;
	}
	auto learnts_cnt = (it -> second).size();
	(it -> second).insert(negations);
	return ((it -> second).size() > learnts_cnt);
}


void equations_xor(vector <int> e1, vector <int> e2,
		vector <int> &res)
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

/*
void log_vars_values(vector <char> &vars_values, vector <char> &is_def,
		vector <int> &dsu)
{
	clog << "variables values\n" <<
		"var is_def value dsu\n";
	for (int x: all_vars) {
		int y = dsu[x];
		clog << x << " " << (int)is_def[y] << " " <<
			(int)vars_values[y] << " " << y << "\n";
	}
}
*/

void reduce_constraints(set <vector <int>> &linear_constraints)
{
	vector <vector <int>> new_lc;
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


void reduce_learnts(map <vector <int>, set <vector <char>>> &learnts,
		set <vector <int>> &linear_constraints)
{
	for (auto it = learnts.begin(); it != learnts.end(); ) {
		auto key = it -> first;
		auto negations = it -> second;
		vector <vector <int>> disjuncts;
		for (auto &v: negations) {
			vector <int> disjunct;
			for (int i = 0; i < (int)key.size(); ++i)
				disjunct.push_back(key[i] ^ v[i]);
			disjuncts.push_back(disjunct);
		}

		int n = key.size();
		if (n == 1) {
			for (auto &d: disjuncts) {
				int x = d[0];
				linear_constraints.insert({x ^ 1});
			}
			it = learnts.erase(it);
			continue;
		}
		else if (n == 2) {
			if (disjuncts.size() == 2) {
				auto d0 = disjuncts[0], d1 = disjuncts[1];
				int x1 = d0[0], y1 = d0[1],
					 x2 = d1[0], y2 = d1[1];
				if (x1 == x2) {
					linear_constraints.insert({x1 ^ 1});
				}
				else if (y1 == y2) {
					linear_constraints.insert({y1 ^ 1});
				}
				else {
					linear_constraints.insert({x1 ^ 1, y1});
				}
				it = learnts.erase(it);
				continue;
			}
			else if (disjuncts.size() == 3) {
				int x = 0, y = 0;
				for (auto &d: disjuncts) {
					x ^= d[0];
					y ^= d[1];
				}
				linear_constraints.insert({x});
				linear_constraints.insert({y});
				it = learnts.erase(it);
				continue;
			}
			else if (disjuncts.size() == 4) {
				cerr << "error: void reduce_learnts(): " <<
					"4 different disjuncts in 2 variables can't be solved\n"; // TODO: throw runtime_error("...")
				exit(0);
			}
		}
		++it;
	}

	for (auto it = learnts.begin(); it != learnts.end(); ) {
		auto key = it -> first;
		int n = key.size();
		for (auto it1 = (it -> second).begin(); it1 != (it -> second).end(); ) {
			vector <int> vars = key;
			for (int i = 0; i < n; ++i)
				vars[i] ^= (*it1)[i];
			bool erase = false;
			for (auto x: vars) {
				if (linear_constraints.find({x ^ 1}) != linear_constraints.end()) {
					erase = true;
					break;
				}
			}
			if (erase) {
				it1 = (it -> second).erase(it1);
				continue;
			}
			
			for (int i = 0; i < n; ++i) {
				for (int j = i + 1; j < n; ++j) {
					int x = vars[i], y = vars[j];
					if (linear_constraints.find({x, y ^ 1}) != linear_constraints.end() ||
						linear_constraints.find({x ^ 1, y}) != linear_constraints.end())
					{
						erase = true;
						break;
					}
				}
				if (erase)
					break;
			}
			if (erase) {
				it1 = (it -> second).erase(it1);
				continue;
			}
			++it1;
		}
		
		if ((it -> second).empty())
			it = learnts.erase(it);
		else
			++it;
	}
}


void resize_all(vector <vector <char> > &vec, int cnt)
{
	// #pragma omp parallel for
	for (int i = 0; i < (int) vec.size(); ++i) {
		vec[i].resize(cnt);
	}
}


int bin_set_size(const vector <char> &a)
{
	int res = 0;
	for (char i: a)
		res += i;

	return res;
}


bool operator== (const AndEquation &a, const AndEquation &b)
{
	return a.y == b.y && a.z == b.z;
}

bool operator< (const AndEquation &a, const AndEquation &b)
{
	return (((a.y & 1) << 1) ^ (a.z & 1)) < (((b.y & 1) << 1) ^ (b.z & 1));
}

/*
int random_int()
{
	gen_mtx.lock();
	int res = generator();
	gen_mtx.unlock();
	return res;
}
*/

bool random_bool()
{
	gen_mtx.lock();
	bool res = generator() & 1;
	gen_mtx.unlock();
	return res;
}


/********************
 * AIG-file reading *
 ********************/
/// HEADER INCLUDES NUMBER OF INPUT VARIABLES,
/// OUTPUT VARIABLES AND TOTAL NUMBER OF VARIABLES
void read_header(ifstream &fin)
{
	clog << " reading header ... ";

	string header;
	fin >> header;

	if (header == "aag") {
		fin >> vars_cnt >> input_vars_cnt >> latches_cnt >> output_vars_cnt >> and_equations_cnt;
	}
	else {
		cerr << "error: void read_header(): " <<
			"wrong input format: \'aag\' expected but \'" << header << "\' found" << endl;
		exit(0);
	}

	clog << "ok" << endl;
}

void read_input(ifstream &fin)
{
	clog << " reading input ... ";

	if (key_vars_cnt == 0)
		key_vars_cnt = input_vars_cnt;
	iv_vars_cnt = input_vars_cnt - key_vars_cnt;

	input_vars.clear();
	input_vars.resize(input_vars_cnt);

	key_vars.clear();
	key_vars.resize(key_vars_cnt);

	iv_vars.clear();
	iv_vars.resize(iv_vars_cnt);

	for (int i = 0; i < (int)key_vars_cnt; ++i) {
		fin >> key_vars[i];
		input_vars[i] = key_vars[i];
	}
	for (int i = 0; i < (int)iv_vars_cnt; ++i) {
		fin >> iv_vars[i];
		input_vars[key_vars_cnt + i] = iv_vars[i];
	}
	
	for (int x: input_vars)
		all_vars_set.insert(x);

	clog << "ok" << endl;
}

void read_output(ifstream &fin)
{
	clog << " reading output ... ";

	output_vars.resize(output_vars_cnt);

	for (int i = 0; i < output_vars_cnt; ++i)
		fin >> output_vars[i];

	clog << "ok" << endl;
}

void read_equations(ifstream &fin)
{
	clog << " reading equations ... ";

	equations.resize(and_equations_cnt);
	for (int i = 0; i < and_equations_cnt; ++i) {
		int x, y, z;
		fin >> x >> y >> z;
		equations[i] = {x, min(y, z), max(y, z)};
		all_vars_set.insert(x & -2);
	}

	clog << "ok" << endl;
}

void read_aig(const string &in_filename)
{
	clog << "reading aig from \'" << in_filename << "\'..." << endl;
	ifstream fin(in_filename.data());

	read_header(fin);
	read_input(fin);
	read_output(fin);
	read_equations(fin);

	fin.close();
	clog << "ok" << endl;
}


void read_linear_constraints(set <vector <int>> &linear_constraints,
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
		vector <int> equation;
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


/// READ LEARNTS FROM THE FILE.
/// DATA ARE PRESENTED IN DIMACS FORMAT (WITHOUT HEADER).
void read_learnts(map <vector <int>, set <vector <char>>> &learnts,
		const string &filename)
{
	clog << "reading additional learnts from \'" << filename << "\' ... ";
	ifstream fin(filename.data());
	int x;
	vector <int> key_value;
	while (fin >> x) {
		if (x == 0) {
			vector <int> key(key_value.size());
			vector <char> negations(key_value.size());
			sort(all(key_value));
			for (int i = 0; i < (int)key_value.size(); ++i) {
				key[i] = key_value[i] & -2;
				negations[i] = key_value[i] & 1;
			}
			learnts[key].insert(negations);
			key_value.clear();
		}
		else {
			key_value.push_back((x > 0 ? 2 * x : 1 - 2 * x));
		}
	}
	if (!key_value.empty()) {
		clog << "warning: void read_learnts(): " <<
			"each line of the \'" << filename << "\' file must end with 0\n";
	}

	fin.close();
	clog << "ok" << endl;
}


void read_core_vars(const string &filename)
{
	clog << "reading core variables from \'" << filename << "\' ... ";
	ifstream fin(filename.data());

	int x;
	while (fin >> x)
		core_vars.push_back(x);
	core_vars_cnt = core_vars.size();

	fin.close();
	clog << "ok" << endl;

	// core_interp.resize(N, vector <char> (core_vars_cnt));
	// guessed_interp.resize(N);

	guessed_vars = core_vars;
	guessed_vars_cnt = core_vars_cnt;
}


/// ЧТЕНИЕ ФАЙЛА С ОПИСАНИЕМ НАЧАЛЬНОЙ ТОЧКИ ПОИСКА
void read_start_point_file(string &filename, vector <char> &point)
{
	if (!filename.empty()) {
		clog << "read point (" << filename << ") ... ";
		clog.flush();

		point.resize(guessed_vars_cnt, 0);
		ifstream fin(filename.data());
		int u;
		while (fin >> u) {
			int i;
			for (i = 0; i < guessed_vars_cnt; ++i) {
				if ((guessed_vars[i] & -2) == (u & -2))
					break;
			}
			if (i < guessed_vars_cnt) {
				point[i] = 1;
			}
			else {
				guessed_vars.push_back(u);
				++guessed_vars_cnt;
				point.push_back(1);
			}
		}

		guessed_vars_cnt = guessed_vars.size();
		
		fin.close();
		clog << "ok" << endl;
	}
	else {
		point = vector <char> (guessed_vars_cnt, 1);
	}
}


void read_lin_table(const string &lt_filename,
		map <vector <Bit>, vector <vector <int>>> &lt)
{
	clog << "reading selector constraints from \'" << lt_filename << "\' . . . ";

	ifstream fin(lt_filename.data());
	
	vector <Bit> key;
	string line;
	
	while (getline(fin, line)) {
		if (line.empty())
			continue;
		
		stringstream ss;
		ss << line;
		
		if (line[0] == '#') {
			key.clear();

			char sharp;
			ss >> sharp;
			
			vector <int> nums;
			int x;
			
			while (ss >> x)
				nums.push_back(x);
			
			int bits = nums.back();
			nums.pop_back();
			int n = nums.size();
			
			for (int i = 0; i < (int)nums.size(); ++i)
				key.push_back({nums[i], (bits >> (n - 1 - i)) & 1});

			continue;
		}
		
		int x, r = 0;
		vector <int> equation;

		while (ss >> x) {
			equation.push_back(x & -2);
			r ^= x & 1;
		}
		
		if (equation.empty())
			continue;

		sort(all(equation));
		equation[0] ^= r;
		lt[key].push_back(equation);
	}

	clog << "ok" << endl;
}


/**************************
 * simplification methods *
 **************************/
void define_variable_value(int var, char val,
		vector <char> &vars_values, vector <char> &is_def)
{
	if (is_def[var]) {
		if (vars_values[var] == val)
			return;
		throw runtime_error("error: void define_variable_value(): value equals to " +
			to_string((int)val) + " but variable is already assigned to " +
			to_string((int)vars_values[var]) + "\n");
		exit(0);
	}
	is_def[var] = 1;
	is_def[var ^ 1] = 1;
	vars_values[var] = val;
	vars_values[var ^ 1] = (val ^ 1);
}


void define_variable_value(int var, char val,
		vector <char> &vars_values, vector <char> &is_def,
		const vector <int> &dsu)
{
	define_variable_value(dsu[var], val, vars_values, is_def);
}


void join_sets(int x, int y,
		vector <char> &vars_values, vector <char> &is_def,
		vector <int> &dsu, vector <vector <int>> &classes)
{
	x = dsu[x];
	y = dsu[y];

	if (x == y)
		return;
	if (x == (y ^ 1)) {
		throw runtime_error("error: void join_sets(): equation 0 = 1 has been deduced\n");
	}
	if (is_def[x] && is_def[y]) {
		if (vars_values[x] != vars_values[y]) {
			throw runtime_error("error: void join_sets(): equation 0 = 1 has been deduced\n");
		}
		return;
	}
	if (is_def[x]) {
		define_variable_value(y, vars_values[x], vars_values, is_def, dsu);
		return;
	}
	if (is_def[y]) {
		define_variable_value(x, vars_values[y], vars_values, is_def, dsu);
		return;
	}

	if (classes[x].size() < classes[y].size())
		swap(x, y);

	for (int z: classes[y]) {
		dsu[z] = x;
		dsu[z ^ 1] = x ^ 1;
		classes[x].push_back(z);
		classes[x ^ 1].push_back(z ^ 1);
	}
	classes[y].clear();
	classes[y ^ 1].clear();
}

/*
void vars_random_assignment(const vector <int> &vars,
		vector <char> &vars_values, vector <char> &is_def, vector <int> &dsu)
{
	for (int var: vars)
		define_variable_value(var, random_bool(), vars_values, is_def, dsu);
}
*/

void vars_random_assignment(const vector <int> &vars,
		vector <char> &vars_values, vector <char> &is_def)
{
	for (int var: vars)
		define_variable_value(var, random_bool(), vars_values, is_def);
}


/*
void read_substitution(const string &filename, vector <char> &vars_values,
		vector <char> &is_def, vector <int> &dsu)
{
	clog << "reading substitution from \'" << filename << "\' ... ";
	ifstream fin(filename.data());

	int x;
	while (fin >> x)
		define_variable_value(x, 0, vars_values, is_def, dsu);

	fin.close();
	clog << "ok" << endl;
}
*/

void find_linear_constraints(set <vector <int>> &linear_constraints)
{
	clog << "finding additional linear constraints ... ";
	map <pair <int, int>, vector <AndEquation>> similar_gates;
	for (auto &e: equations)
		similar_gates[{e.y & -2, e.z & -2}].push_back(e);

	for (auto &p: similar_gates) {
		auto s = p.second;
		order(s);
		if (s.size() == 1)
			continue;

		auto key = p.first;
		int a = key.first + (s[0].y & 1), b = key.second + (s[0].z & 1);
		int id0 = ((s[0].y & 1) << 1) ^ (s[0].z & 1);
		vector <int> vars;

		for (int i = 1; i < (int)s.size(); ++i) {
			int idi = ((s[i].y & 1) << 1) ^ (s[i].z & 1) ^ id0;
			if (idi == 1) // x0 + xi = ab + a(b + 1) = ab + ab + a = a
				vars = {s[0].x, s[i].x, a};
			else if (idi == 2) // x0 + xi = ab + (a + 1)b = ab + b + ab = b
				vars = {s[0].x, s[i].x, b};
			else // idi == 3 // x0 + xi = ab + (a + 1)(b + 1) = ab + ab + a + b + 1 = a + (b + 1)
				vars = {s[0].x, s[i].x, a, b ^ 1};
			linear_constraints.insert(vars);
		}
	}
	clog << "ok" << endl;
}


void find_linear_constraints_(set <vector <int>> &linear_constraints)
{
	clog << "finding additional linear constraints _ ... ";
	
	std::vector <char> used(and_equations_cnt, 0), used_lin(and_equations_cnt, 0);
	std::vector <int> index(vars_cnt + 1, -1);
	
	for (int i = 0; i < and_equations_cnt; ++i)
		index[equations[i].x / 2] = i;

	// std::vector <std::vector <int>> graph;
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

	std::vector <AndEquation> equations_new;
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
	
	std::set <int> all_vars_new;
	
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
	
	
	// std::vector <int> index_new(vars_cnt + 1, -1);
	
	// for (int i = 0; i < and_equations_cnt; ++i)
		// index[equations[i].x / 2] = i;
}


/// ВЫВОД ИЗ КВАДРАТИЧНОГО УРАВНЕНИЯ.
/// ВОЗВРАЩАЕТ 1, ЕСЛИ ХОТЬ ЧТО-ТО ВЫВЕЛОСЬ.
/// ИНАЧЕ 0. ИСПОЛЬЗУЕТ КЛАССЫ ЭКВИВАЛЕНТНОСТИ.
char propagation(const AndEquation &e,
		vector <char> &vars_values, vector <char> &is_def,
		vector <int> &dsu, vector <vector <int>> &classes,
		map <vector <int>, set <vector <char>>> &learnts,
		char &useless)
{
	useless = 1;

	int x = dsu[e.x];
	int y = dsu[e.y];
	int z = dsu[e.z];

	char val_x = vars_values[x];
	char val_y = vars_values[y];
	char val_z = vars_values[z];

	if (is_def[x] && is_def[y] && is_def[z])
		return 0;

	if (z == y) {
		if (is_def[y] && !is_def[x]) {
			define_variable_value(x, val_y, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (!is_def[y] && is_def[x]) {
			define_variable_value(y, val_x, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (x == y) {
			// useless = 1;
			return 0;
		}
		join_sets(x, y, vars_values, is_def, dsu, classes);
		// useless = 1;
		return 1;
	}

	if (z == (y ^ 1)) {
		if (!is_def[x]) {
			define_variable_value(x, 0, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		// useless = 1;
		return 0;
	}

	if (y == x) {
		if (is_def[x] && val_x == 1) {
			define_variable_value(z, 1, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (is_def[z] && val_z == 0) {
			define_variable_value(x, 0, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (is_def[x]) {
			// useless = 1;
			return 0;
		}
		if (is_def[z]) {
			// useless = 1;
			return 0;
		}
		vector <int> key_value = {x ^ 1, z};
		// useless = 1;
		return add_learnts(learnts, key_value);
	}

	if (y == (x ^ 1)) {
		if (!is_def[x] || !is_def[z]) {
			define_variable_value(x, 0, vars_values, is_def, dsu);
			define_variable_value(z, 0, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		// useless = 1;
		return 0;
	}

	if (z == x) {
		if (is_def[x] && val_x == 1) {
			define_variable_value(y, 1, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (is_def[y] && val_y == 0) {
			define_variable_value(x, 0, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (is_def[x]) {
			// useless = 1;
			return 0;
		}
		if (is_def[y]) {
			// useless = 1;
			return 0;
		}
		vector <int> key_value = {x ^ 1, y};
		// useless = 1;
		return add_learnts(learnts, key_value);
	}

	if (z == (x ^ 1)) {
		if (!is_def[x] || !is_def[y]) {
			define_variable_value(x, 0, vars_values, is_def, dsu);
			define_variable_value(y, 0, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		// useless = 1;
		return 0;
	}

	if (!is_def[x]) {
		if ((is_def[y] && val_y == 0) || (is_def[z] && val_z == 0)) {
			define_variable_value(x, 0, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (is_def[y] && is_def[z]) { // && val_y == 1 && val_z == 1
			define_variable_value(x, 1, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (is_def[y]) { // && val_y == 1 && !is_def[z]
			join_sets(x, z, vars_values, is_def, dsu, classes);
			// useless = 1;
			return 1;
		}
		if (is_def[z]) { // && val_z == 1 && !is_def[y]
			join_sets(x, y, vars_values, is_def, dsu, classes);
			// useless = 1;
			return 1;
		}
		useless = 0;
		return 0;
	}
	// is_def[x]
	if (val_x == 0) {
		if (is_def[y] && val_y == 1) { // && !is_def[z]
			define_variable_value(z, 0, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		if (is_def[z] && val_z == 1) { // && !is_def[y]
			define_variable_value(y, 0, vars_values, is_def, dsu);
			// useless = 1;
			return 1;
		}
		vector <int> key_value = {y ^ 1, z ^ 1};
		// useless = 1;
		return add_learnts(learnts, key_value);
	}
	// val_x == 1
	if (!is_def[y])
		define_variable_value(y, 1, vars_values, is_def, dsu);
	if (!is_def[z])
		define_variable_value(z, 1, vars_values, is_def, dsu);
	// useless = 1;
	return 1;
}


/// ВЫВОД ИЗ КВАДРАТИЧНОГО УРАВНЕНИЯ.
/// ВОЗВРАЩАЕТ 1, ЕСЛИ ХОТЬ ЧТО-ТО ВЫВЕЛОСЬ.
/// ИНАЧЕ 0. ДОБАВЛЕНА ПРОВЕРКА ЭКВИВАЛЕНТНОСТИ ВХОДОВ.
char propagation(const AndEquation &e,
		vector <char> &vars_values, vector <char> &is_def,
		vector <int> &dsu, vector <vector <int>> &classes,
		map <pair <int, int>, int> &gate_pairs,
		map <vector <int>, set <vector <char>>> &learnts,
		char &useless)
{
	int x = dsu[e.x];
	int y = dsu[e.y];
	int z = dsu[e.z];

	if (is_def[x] && is_def[y] && is_def[z]) {
		useless = 1;
		return 0;
	}

	if (y > z)
		swap(y, z);
	pair <int, int> key = {y, z};

	auto it = gate_pairs.find(key);
	char res = 0;
	if (it != gate_pairs.end()) {
		if (dsu[it -> second] != x) {
			join_sets(it -> second, x, vars_values, is_def, dsu, classes);
			it -> second = dsu[x];
			res = 1;
			useless = 1;
		}
		else {
			useless = 0;
		}
	}
	else {
		gate_pairs[key] = x;
		res = 1;
		useless = 0;
	}
	char u;
	res |= propagation(e, vars_values, is_def, dsu, classes, learnts, u);
	useless |= u;
	return res;
}


char learnts_propagation(vector <int> &key, vector <vector <char>> &negations,
		vector <char> &vars_values, vector <char> &is_def,
		vector <int> &dsu, vector <vector <int>> &classes,
		char &useless)
{
	useless = 1;
	int n = key.size();
	vector <vector <int>> disjuncts;
	vector <int> variables = key;
	for (auto &x: variables)
		x = dsu[x];
	for (auto &v: negations) {
		vector <int> disjunct;
		bool skip = 0; // applying UP rule flag
		for (int i = 0; i < (int)key.size(); ++i) {
			int x = variables[i] ^ v[i];
			if (is_def[x]) {
				if (vars_values[x] == 0)
					continue;
				// vars_values[x] == 1
				skip = 1;
				break;
			}
			disjunct.push_back(x);
		}
		if (skip)
			continue;
		disjuncts.push_back(disjunct);
	}
	if (disjuncts.empty()) {
		// useless = 1;
		return 0;
	}

	for (auto x: variables) {
		if (is_def[x])
			--n;
	}
	if (n == 0) {
		std::cerr << "error: char learnts_propagation(): empty disjunct can't be solved\n"; // TODO: throw runtime_error("...")
		exit(0);
	}
	else if (n == 1) {
		for (auto &d: disjuncts) {
			int x = dsu[d[0]];
			define_variable_value(x, 1, vars_values, is_def, dsu);
		}
		// useless = 1;
		return 1;
	}
	else if (n == 2) {
		if (disjuncts.size() == 1) {
			auto d = disjuncts[0];
			int x = dsu[d[0]], y = dsu[d[1]];
			if (x == y) {
				define_variable_value(x, 1, vars_values, is_def, dsu);
				// useless = 1;
				return 1;
			}
			else if (x == (y ^ 1)) {
				// useless = 1;
				return 0;
			}
			else {
				useless = 0;
				return 0;
			}
		}
		else if (disjuncts.size() == 2) {
			auto d0 = disjuncts[0], d1 = disjuncts[1];
			int x1 = dsu[d0[0]], y1 = dsu[d0[1]],
				 x2 = dsu[d1[0]], y2 = dsu[d1[1]];
			if (x1 == x2) {
				define_variable_value(x1, 1, vars_values, is_def, dsu);
				// useless = 1;
				return 1;
			}
			else if (y1 == y2) {
				define_variable_value(y1, 1, vars_values, is_def, dsu);
				// useless = 1;
				return 1;
			}
			else if (x1 != (y1 ^ 1)) {
				join_sets(x1, y1 ^ 1, vars_values, is_def, dsu, classes);
				// useless = 1;
				return 1;
			}
			else {
				// useless = 1;
				return 0;
			}
		}
		else if (disjuncts.size() == 3) {
			int x = 0, y = 0;
			for (auto &d: disjuncts) {
				x ^= dsu[d[0]];
				y ^= dsu[d[1]];
			}
			define_variable_value(x, 0, vars_values, is_def, dsu);
			define_variable_value(y, 0, vars_values, is_def, dsu);
			// useless = 1
			return 1;
		}
		else {
			cerr << "error: char learnts_propagation(): " <<
				"4 different disjuncts in 2 variables can't be solved\n"; // TODO: throw runtime_error("...")
			exit(0);
		}
	}
	else { // n >= 3
		// clog << "warning: void learnts_propagation(): n >= 3. There are no methods to use it.\n"; // TODO: throw smth
		useless = 0;
		return 0;
	}
	return 0;
}


char analyze_learnts(map <vector <int>, set <vector <char>>> &learnts,
		vector <char> &vars_values, vector <char> &is_def,
		vector <int> &dsu, vector <vector <int>> &classes)
{
	char cnt = 0;
	for (auto it = learnts.begin(); it != learnts.end(); ) {
		auto key = it -> first;
		vector <vector <char>> negations(all(it -> second));
		bool change_key = 0;
		for (int i = 0; i < (int)key.size(); ++i) {
			int x = key[i];
			if (dsu[x] != x) {
				change_key = 1;
				key[i] = dsu[x] & -2;
				if (dsu[x] & 1) {
					for (auto &v: negations)
						v[i] ^= 1;
				}
			}
		}

		if (change_key) {
			learnts[key].insert(all(negations));
			it = learnts.erase(it);
			cnt |= 1;
			continue;
		}

		char useless;
		cnt |= learnts_propagation(key, negations, vars_values, is_def, dsu, classes, useless);
		if (useless)
			it = learnts.erase(it);
		else
			++it;
	}
	return cnt;
}


void simple_linear_propagation(set <vector <int>> &linear_constraints,
		vector <vector <int>> &relations,
		vector <char> &vars_values, vector <char> &is_def,
		vector <int> &dsu)
{
	vector <vector <int>> changes;
	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ) {
		char useless = 0, value = 0, change = 1;
		vector <int> equation_vector;
		map <int, int> equation_map;
		for (auto x: *it) {
			if (x != dsu[x])
				change = 1;
			x = dsu[x];
			if (is_def[x]) {
				value ^= vars_values[x];
				continue;
			}
			value ^= x & 1;
			++equation_map[x & -2];
		}
		for (auto &p: equation_map) {
			if (p.second > 1)
				change = 1;
			if (p.second & 1) {
				equation_vector.push_back(p.first ^ value);
				value = 0;
			}
		}
		if (equation_vector.empty()) {
			if (value) {
				cerr << "error: void simple_linear_propagation(): " <<
					"wrong equation has been deduced: 0 = 1\n";
				exit(0);
			}
			useless = 1;
		}
		else if (equation_vector.size() <= 2) {
			relations.push_back(equation_vector);
			useless = 1;
		}
		// else {
		//  useless = 0;
		// }


		if (useless) {
			it = linear_constraints.erase(it);
			continue;
		}
		else if (change) {
			changes.push_back(equation_vector);
			it = linear_constraints.erase(it);
			continue;
		}
		else {
			++it;
		}
	}
	linear_constraints.insert(all(changes));
}


void linear_propagation(set <vector <int>> &linear_constraints,
		vector <vector <int>> &relations)
{
	vector <vector <int>> equations_by_var(vars_cnt + 1);
	int counter = 0;
	/* Gauss algorithm stage 1 */
	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ++it) {
		for (int i = 1; i < (int)(*it).size(); ++i)
			equations_by_var[(*it)[i] / 2].push_back(counter);
		++counter;
		auto it1 = it;
		++it1;
		while (it1 != linear_constraints.end() && ((*it1)[0] / 2) == ((*it)[0] / 2)) {
			vector <int> res;
			equations_xor(*it, *it1, res);
			if (!res.empty())
				linear_constraints.insert(res);
			it1 = linear_constraints.erase(it1);
		}
	}
	/// СОХРАНИТЬ СИСТЕМУ В vector <vector <int>> ls
	/// ДЛЯ КАЖДОГО x \in ls СОХРАНИТЬ ВСЕ УРАВНЕНИЯ, СОДЕРЖАЩИЕ x[0] или x[0] ^ 1
	/// ИСПОЛЬЗОВАТЬ vector <vector <int>> equations_by_vars РАЗМЕРА vars_cnt + 1

	vector <vector <int>> ls(all(linear_constraints));
	/* Gauss algorithm stage 2 */
	for (int i = ls.size() - 1; i >= 0; --i) {
		/// ЕСЛИ ls[i] СОДЕРЖИТ ЛИНЕЙНОЕ СООТНОШЕНИЕ
		/// НАД 1 ИЛИ 2 ПЕРЕМЕНЫМИ, НАДО ЕГО СОХРАНИТЬ
		if (ls[i].size() <= 2)
			relations.push_back(ls[i]);

		for (int j : equations_by_var[ls[i][0] / 2]) {
			vector <int> res;
			equations_xor(ls[i], ls[j], res);
			ls[j] = res;
			if (ls[j].size() <= 2)
				relations.push_back(ls[j]);
		}
	}
	linear_constraints.clear();
	linear_constraints.insert(all(ls));
}


char analyze_relations(vector <vector <int>> &relations,
		vector <char> &vars_values, vector <char> &is_def,
		vector <int> &dsu, vector <vector <int>> &classes)
{
	if (relations.empty())
		return 0;
	char res = 0;
	for (auto &relation: relations) {
		if (relation.size() == 1) {
			try {
				define_variable_value(relation[0], 0, vars_values, is_def, dsu);
			}
			catch (exception &e) {
				cerr << e.what() << endl;
			}
			res = 1;
		}
		else if (relation.size() == 2) {
			try {
				join_sets(relation[0], relation[1], vars_values, is_def, dsu, classes);
			}
			catch (exception &e) {
				cerr << e.what() << endl;
			}
			res = 1;
		}
		else
			res = 0;
	}
	return res;
}


int get_rank(set <vector <int>> linear_constraints) {
	int counter = 0;
	/* Gauss algorithm stage 1 */
	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ++it) {
		++counter;
		auto it1 = it;
		++it1;
		while (it1 != linear_constraints.end() && ((*it1)[0] & -2) == ((*it)[0] & -2)) {
			vector <int> res;
			equations_xor(*it, *it1, res);
			if (!res.empty())						//
				linear_constraints.insert(res);		// TODO: is linear_constraints able to be broken?
			it1 = linear_constraints.erase(it1);	//
		}
	}
	return linear_constraints.size();
}


void solve(char &a, vector <char> &vars_values, vector <char> &is_def)
{
	map <pair <int, int>, int> gate_pairs;
	vector <int> dsu(2 * (vars_cnt + 1));
	vector <vector <int>> classes(2 * (vars_cnt + 1));
	vector <char> useless_equations(and_equations_cnt, 0);

	set <vector <int>> linear_constraints = pattern_linear_constraints;

	for (const auto &p: lin_table) {
		auto key = p.first;
		
		bool skip = 0;
		for (auto b: key) {
			if (vars_values[output_vars[b.n]] != b.bit) {
				skip = 1;
				break;
			}
		}
		
		if (skip)
			continue;
		
		linear_constraints.insert(all(p.second));
	}

	// int rank = get_rank(linear_constraints);
	// log_mtx.lock();
	// 	std::clog << rank << std::endl;
	// log_mtx.unlock();
	// return;

	auto linear_constraints_copy = linear_constraints;
	auto learnts = pattern_learnts;

	for (int i = 0; i < (int)dsu.size(); ++i) {
		dsu[i] = i;
		classes[i].push_back(i);
	}

	while (true) {
		char cnt;
		while (true) {
			cnt = 0;
			for (int i = 0; i < and_equations_cnt; ++i) {
				if (useless_equations[i])
					continue;

				char useless;
				auto e = equations[i];
				cnt |= propagation(e, vars_values, is_def,
					dsu, classes, gate_pairs, learnts, useless);
				if (useless) {
					useless_equations[i] = 1;
					// TODO: STORE equations COPY IN list<AndEquation> AND ERASE e FROM IT HERE
				}
			}
			if (!cnt)
				break;
		}

		vector <vector <int>> relations;
		
		linear_constraints = linear_constraints_copy;
		simple_linear_propagation(linear_constraints, relations, vars_values, is_def, dsu);
		linear_constraints_copy = linear_constraints;
		
		cnt |= analyze_relations(relations, vars_values, is_def, dsu, classes);

		cnt |= analyze_learnts(learnts, vars_values, is_def, dsu, classes);

		if (!cnt) {
			relations.clear();
			linear_propagation(linear_constraints, relations);
			cnt |= analyze_relations(relations, vars_values, is_def, dsu, classes);
		}

		if (!cnt)
			break;
	}

	a = 1;
	for (auto x: input_vars) {
		if (!is_def[dsu[x]]) {
			a = 0;
			break;
		}
	}
	
	// #pragma omp critical (clog)
	// {
	// 	if (a == 0) {
	// 		vector <int> rem;
			
	// 		for (const auto &e: equations) {
	// 			if (!is_def[dsu[e.y]] && !is_def[dsu[e.z]])
	// 				rem.push_back(e.x);
	// 		}

	// 		clog << rem.size() << ":\n";
	// 		for (int x: rem)
	// 			clog << x << " ";
	// 		clog << "\n";
	// 	}
	// }
}


void find_output(char &a, vector <char> &vars_values, vector <char> &is_def)
{
	map <pair <int, int>, int> gate_pairs;
	vector <int> dsu(2 * (vars_cnt + 1));
	vector <vector <int>> classes(2 * (vars_cnt + 1));
	vector <char> useless_equations(and_equations_cnt, 0);
	auto linear_constraints = pattern_linear_constraints;
	auto learnts = pattern_learnts;

	for (int i = 0; i < (int)dsu.size(); ++i) {
		dsu[i] = i;
		classes[i].push_back(i);
	}

	while (true) {
		char cnt;
		while (true) {
			cnt = 0;
			for (int i = 0; i < and_equations_cnt; ++i) {
				if (useless_equations[i])
					continue;

				char useless;
				auto e = equations[i];
				cnt |= propagation(e, vars_values, is_def,
					dsu, classes, gate_pairs, learnts, useless);
				if (useless) {
					useless_equations[i] = 1;
				}
			}
			if (!cnt)
				break;
		}

		vector <vector <int>> relations;
		simple_linear_propagation(linear_constraints, relations, vars_values, is_def, dsu);
		cnt |= analyze_relations(relations, vars_values, is_def, dsu, classes);

		cnt |= analyze_learnts(learnts, vars_values, is_def, dsu, classes);

		if (!cnt) {
			relations.clear();
			linear_propagation(linear_constraints, relations);
			cnt |= analyze_relations(relations, vars_values, is_def, dsu, classes);
		}

		if (!cnt)
			break;
	}

	// for (int j = 0; j < guessed_vars_cnt; ++j)
	// 	vars_values[guessed_vars[j]] = vars_values[dsu[guessed_vars[j]]];
	// for (int j = 0; j < output_vars_cnt; ++j)
	// 	vars_values[output_vars[j]] = vars_values[dsu[output_vars[j]]];

	for (int x: all_vars) {
		vars_values[x] = vars_values[dsu[x]];
		vars_values[x ^ 1] = vars_values[dsu[x ^ 1]];
	}
}


// void save_input_interp(vector <char> &core_interp, vector <char> &iv_interp,
// 		vector <char> &vars_values)
// {
// 	for (int j = 0; j < core_vars_cnt; ++j)
// 		core_interp[j] = vars_values[core_vars[j]];
// 	for (int j = 0; j < iv_vars_cnt; ++j)
// 		iv_interp[j] = vars_values[iv_vars[j]];
// }


// void save_output_interp(vector <char> &guessed_interp, vector <char> &output_interp,
// 		vector <char> &vars_values)
// {
// 	for (int j = 0; j < guessed_vars_cnt; ++j)
// 		guessed_interp[j] = vars_values[guessed_vars[j]];
// 	for (int j = 0; j < output_vars_cnt; ++j)
// 		output_interp[j] = vars_values[output_vars[j]];
// }


// void gen_random_sample(vector <char> &core_interp, vector <char> &iv_interp,
// 		vector <char> &guessed_interp, vector <char> &output_interp)
// {
// 	vector <char> vars_values(2 * (vars_cnt + 1));
// 	vector <char> is_def(2 * (vars_cnt + 1));

// 	fill(all(is_def), 0);
// 	vars_random_assignment(core_vars, vars_values, is_def);
// 	vars_random_assignment(iv_vars,   vars_values, is_def);
// 	save_input_interp(core_interp, iv_interp, vars_values);
// 	char a;
// 	find_output(a, vars_values, is_def);
// 	save_output_interp(guessed_interp, output_interp, vars_values);
// }


void save_interp(const vector <char> &vars_values, int i)
{
	for (int j = 0; j < 2 * vars_cnt + 2; ++j)
		interp[i][j] = vars_values[j];
}


void gen_random_sample()
{
	interp.resize(N, vector<char>(2 * vars_cnt + 2));

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < N; ++i) {
		vector <char> vars_values(2 * (vars_cnt + 1));
		vector <char> is_def(2 * (vars_cnt + 1));
		
		fill(all(is_def), 0);
		vars_random_assignment(core_vars, vars_values, is_def);
		vars_random_assignment(iv_vars, vars_values, is_def);
		char a;
		find_output(a, vars_values, is_def);
		save_interp(vars_values, i);
	}
}


// void clear_vars_values(vector <char> &iv_interp, vector <char> &guessed_interp,
// 		vector <char> &output_interp, const vector <char> &vars_set,
// 		vector <char> &vars_values, vector <char> &is_def)
// {
// 	fill(all(is_def), 0);

// 	for (int j = 0; j < iv_vars_cnt; ++j)
// 		define_variable_value(iv_vars[j], iv_interp[j], vars_values, is_def);

// 	for (int j = 0; j < guessed_vars_cnt; ++j) {
// 		if (vars_set[j])
// 			define_variable_value(guessed_vars[j], guessed_interp[j], vars_values, is_def);
// 	}

// 	for (int j = 0; j < output_vars_cnt; ++j)
// 		define_variable_value(output_vars[j], output_interp[j], vars_values, is_def);
// }


void clear_vars_values(vector<char> &interp, const vector <char> &vars_set,
		vector <char> &vars_values, vector <char> &is_def)
{
	fill(all(is_def), 0);

	for (int j = 0; j < iv_vars_cnt; ++j)
		define_variable_value(iv_vars[j], interp[iv_vars[j]], vars_values, is_def);

	for (int j = 0; j < guessed_vars_cnt; ++j) {
		if (vars_set[j])
			define_variable_value(guessed_vars[j], interp[guessed_vars[j]], vars_values, is_def);
	}

	for (int j = 0; j < output_vars_cnt; ++j)
		define_variable_value(output_vars[j], interp[output_vars[j]], vars_values, is_def);
}


double complexity(const vector <char> &vars_set)
{
	auto it = complexity_set.find(vars_set);
	if (it != complexity_set.end())
		return it -> second;

	int cnt = 0;
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < N; ++i) {
		char answer;
		vector <char> vars_values(2 * (vars_cnt + 1));
		vector <char> is_def(2 * (vars_cnt + 1));
		// vector <char> core_interp(core_vars_cnt), iv_interp(iv_vars_cnt),
			// guessed_interp(guessed_vars_cnt), output_interp(output_vars_cnt);
		// #pragma omp critical (gen)
		// {
		// 	gen_random_sample(core_interp, iv_interp, guessed_interp, output_interp);
		// }
		// gen_random_sample(core_interp[i], iv_interp[i], guessed_interp[i], output_interp[i]);
		// clear_vars_values(iv_interp, guessed_interp, output_interp,
			// vars_set, vars_values, is_def);
		clear_vars_values(interp[i], vars_set, vars_values, is_def);
		solve(answer, vars_values, is_def);
		if (answer) {
			#pragma omp atomic
				++cnt;
		}
	}

	double prob = (double) cnt / N;
	double res;
	if (prob == 0)
		res = DBL_MAX;
	else
		res = 3.0 * pow(2.0, (double) bin_set_size(vars_set)) / prob;

	probability_set[vars_set] = prob;
	complexity_set[vars_set] = res;
	++points_cnt;

	return res;
}


void print_complexity(const vector<char> &vars_set, const string &out_filename)
{
	ofstream fout(out_filename.data());
	
	fout << bin_set_size(vars_set) << ": ";
	for (int i = 0; i < guessed_vars_cnt; ++i) {
		if (vars_set[i])
			fout << guessed_vars[i] << " ";
	}
	fout << endl;
	fout << "complexity: "  << complexity(vars_set) << endl;
	complexity(vars_set);
	fout << "probability: " << probability_set[vars_set] << endl;

	fout.close();
}


/*****************
 * main function *
 *****************/
int main(int argc, char *argv[])
{

	string in_filename, out_filename;
	string substitution_filename, core_vars_filename;
	string linear_constraints_filename, learnts_filename;
	string start_point_filename;
	string lin_table_filename;

	init(argc, argv, in_filename, out_filename,
		substitution_filename, core_vars_filename,
		linear_constraints_filename, learnts_filename,
		start_point_filename, lin_table_filename);

	read_core_vars(core_vars_filename);
	read_aig(in_filename);

	read_learnts(pattern_learnts, learnts_filename);
	reduce_learnts(pattern_learnts, pattern_linear_constraints);
	
	// find_linear_constraints(pattern_linear_constraints);
	// find_linear_constraints_(pattern_linear_constraints);
	read_linear_constraints(pattern_linear_constraints, linear_constraints_filename);
	// reduce_constraints(pattern_linear_constraints);

	all_vars = vector <int> (all(all_vars_set));
	clog << "all vars size: " << all_vars.size() << endl;

	// read_lin_table(lin_table_filename, lin_table);
	
	vector <char> start_point;
	read_start_point_file(start_point_filename, start_point);

	// map<double, vector<int>> addition;

	// generator.seed(123456789);
	// gen_random_sample();

/*
	print_complexity(start_point, out_filename);
	clog << "complexity: " << complexity_set[start_point] << endl;
	clog << "probability: " << probability_set[start_point] << endl;

	for (int i = 0; i < (int)all_vars.size(); ++i) {
		clog << i << "\n";
		int x = all_vars[i];

		guessed_vars.push_back(x);
		guessed_vars_cnt = guessed_vars.size();
		start_point.resize(guessed_vars_cnt, 1);

		// clog << x << ": ";
		print_complexity(start_point, out_filename);

		addition[probability_set[start_point]].push_back(x);

		complexity_set.erase(start_point);
		probability_set.erase(start_point);

		guessed_vars.pop_back();
		start_point.pop_back();
	}

	for (auto &p: addition) {
		clog << p.first << ":\n";
		for (int x: p.second)
			clog << x << " ";
		clog << "\n";
	}
*/
	
	// print_complexity(start_point, out_filename);
	// clog << "complexity: " << complexity_set[start_point] << endl;
	// clog << "probability: " << probability_set[start_point] << endl;

	map<int, int> all_vars_map_eq;
	
	for (auto &lc: pattern_linear_constraints) {
		for (int x: lc)
			++all_vars_map_eq[x & -2];
	}

	map<int, vector <int>> all_vars_map_eq_inv;

	for (auto &p: all_vars_map_eq)
		all_vars_map_eq_inv[p.second].push_back(p.first);
	
	for (auto &p: all_vars_map_eq_inv) {
		clog << p.first << " " << p.second.size() << ":\n";
		for (int x: p.second)
			clog << x << " ";
		clog << "\n";
	}

}
