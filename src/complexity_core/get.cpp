#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

using namespace std;

int N = 0;

random_device rd;
mt19937 generator(rd());

mutex gen_mtx;


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
set<vector<int>> pattern_linear_constraints;
map<vector<int>, set<vector<char>>> pattern_learnts;
map<vector<Bit>, vector<vector<int>>> lin_table;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector<int> input_vars;
vector<int> output_vars;
vector<int> guessed_vars;
vector<int> core_vars;
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
int guessed_vars_cnt = 0;
int latches_cnt = 0;
int core_vars_cnt = 0;


map<vector<char>, double> complexity_set;
map<vector<char>, double> probability_set;

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
void order(vector<T> &v) {
	sort(all(v));
	v.resize(unique(all(v)) - v.begin());
	v.shrink_to_fit();
}


char add_learnts(map<vector<int>, set<vector<char>>> &learnts,
		vector<int> &key_value)
{
	sort(all(key_value));
	vector<int> key(key_value.size());
	vector<char> negations(key_value.size());
	for (int i = 0; i < (int) key_value.size(); ++i) {
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


void reduce_learnts(map<vector<int>, set<vector<char>>> &learnts,
		set<vector<int>> &linear_constraints)
{
	for (auto it = learnts.begin(); it != learnts.end(); ) {
		auto key = it -> first;
		auto negations = it -> second;
		vector<vector<int>> disjuncts;
		for (auto &v: negations) {
			vector<int> disjunct;
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
			vector<int> vars = key;
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


void resize_all(vector<vector<char> > &vec, int cnt)
{
	// #pragma omp parallel for
	for (int i = 0; i < (int) vec.size(); ++i) {
		vec[i].resize(cnt);
	}
}


int bin_set_size(const vector<char> &a)
{
	int res = 0;
	for (char i: a)
		res += i;

	return res;
}


char random_bool()
{
	gen_mtx.lock();
	char res = generator() & 1;
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

	input_vars.clear();
	input_vars.resize(input_vars_cnt);

	for (int i = 0; i < (int) input_vars_cnt; ++i) {
		fin >> input_vars[i];
		all_vars_set.insert(input_vars[i]);
	}

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


void read_linear_constraints(set<vector<int>> &linear_constraints,
		const string &filename)
{
	clog << "reading additional linear constraints from \'" << filename << "\' ... ";

	ifstream fin(filename.data());

	string line;
	while (getline(fin, line)) {
		stringstream ss;
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


/// READ LEARNTS FROM THE FILE.
/// DATA ARE PRESENTED IN DIMACS FORMAT (WITHOUT HEADER).
void read_learnts(map<vector<int>, set<vector<char>>> &learnts,
		const string &filename)
{
	clog << "reading additional learnts from \'" << filename << "\' ... ";
	ifstream fin(filename.data());
	int x;
	vector<int> key_value;
	while (fin >> x) {
		if (x == 0) {
			vector<int> key(key_value.size());
			vector<char> negations(key_value.size());
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

	guessed_vars = core_vars;
	guessed_vars_cnt = core_vars_cnt;
}


/// ЧТЕНИЕ ФАЙЛА С ОПИСАНИЕМ НАЧАЛЬНОЙ ТОЧКИ ПОИСКА
void read_start_point_file(string &filename, vector<char> &point)
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
		point = vector<char> (guessed_vars_cnt, 1);
	}
}


void read_lin_table(const string &lt_filename,
		map<vector<Bit>, vector<vector<int>>> &lt)
{
	clog << "reading selector constraints from \'" << lt_filename << "\' . . . ";

	ifstream fin(lt_filename.data());
	
	vector<Bit> key;
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
			
			vector<int> nums;
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
		vector<int> equation;

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
		vector<char> &vars_values, vector<char> &is_def)
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
		vector<char> &vars_values, vector<char> &is_def,
		const vector<int> &dsu)
{
	define_variable_value(dsu[var], val, vars_values, is_def);
}


void join_sets(int x, int y,
		vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes)
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


void vars_random_assignment(const vector<int> &vars,
		vector<char> &vars_values, vector<char> &is_def)
{
	for (int var: vars)
		define_variable_value(var, random_bool(), vars_values, is_def);
}


/// ВЫВОД ИЗ КВАДРАТИЧНОГО УРАВНЕНИЯ.
/// ВОЗВРАЩАЕТ 1, ЕСЛИ ХОТЬ ЧТО-ТО ВЫВЕЛОСЬ.
/// ИНАЧЕ 0. ИСПОЛЬЗУЕТ КЛАССЫ ЭКВИВАЛЕНТНОСТИ.
char propagation(const int ind,
		vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes,
		map<vector<int>, set<vector<char>>> &learnts,
		char &useless)
{
	useless = 1;

	auto e = equations[ind];
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
		vector<int> key_value = {x ^ 1, z};
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
		vector<int> key_value = {x ^ 1, y};
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
		vector<int> key_value = {y ^ 1, z ^ 1};
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
/// ИНАЧЕ 0. ПРОВЕРКА ЭКВИВАЛЕНТНОСТИ ВХОДОВ.
char propagation(const int ind,
		vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes,
		map<pair <int, int>, int> &gate_pairs,
		map<vector<int>, set<vector<char>>> &learnts,
		set<vector<int>> &linear_constraints, char &useless)
{
	auto e = equations[ind];
	int x = dsu[e.x];
	int y = dsu[e.y];
	int z = dsu[e.z];

	if (is_def[x] && is_def[y] && is_def[z]) {
		useless = 1;
		return 0;
	}

	useless = 0;

	if (y > z)
		swap(y, z);
	pair<int, int> key = {y & -2, z & -2};

	auto it = gate_pairs.find(key);
	char res = 0;
	if (it == gate_pairs.end()) {
		gate_pairs[key] = ind;
		// res = 0;
		// useless = 0;
	}
	else if (it -> second == ind) {
		// res = 0;
		// useless = 0;
	}
	else { // it -> second != ind
		int j = it -> second;
		auto f = equations[j];

		int _x = dsu[f.x], _y = dsu[f.y], _z = dsu[f.z];
		if (_y > _z)
			swap(_y, _z);

		int id = ((y & 1) << 1) ^ (z & 1);
		int _id = ((_y & 1) << 1) ^ (_z & 1) ^ id;

		if (_id == 0) { // x +_x = yz + yz = 0
			if (_x != x) {
				join_sets(x, _x, vars_values, is_def, dsu, classes);
				useless = 1;
				res = 1;
			}
			// else {
			// 	res = 0;
			// 	useless = 0;
			// }
		}
		else {
			vector<int> vars;

			if (_id == 1) // x + _x = yz + y(z + 1) = yz + yz + y = y
				vars = {x, _x, y};
			else if (_id == 2) // x + _x = yz + (y + 1)z = yz + z + yz = z
				vars = {x, _x, z};
			else // _id == 3, x + _x = yz + (y + 1)(z + 1) = yz + yz + y + z + 1 = y + (z + 1)
				vars = {x, _x, y, z ^ 1};

			int rem = 0;
			for (auto &var: vars) {
				rem ^= var & 1;
				var &= -2;
			}
			sort(all(vars));
			vars[0] ^= rem;

			
			if (linear_constraints.insert(vars).second)
				res = 1;
			useless = 1;
		}
	}

	char u;
	res |= propagation(ind, vars_values, is_def, dsu, classes, learnts, u);
	useless |= u;
	return res;
}


char learnts_propagation(vector<int> &key, vector<vector<char>> &negations,
		vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes,
		char &useless)
{
	useless = 1;
	int n = key.size();
	vector<vector<int>> disjuncts;
	vector<int> variables = key;
	for (auto &x: variables)
		x = dsu[x];
	for (auto &v: negations) {
		vector<int> disjunct;
		bool skip = 0; // applying UP rule flag
		for (int i = 0; i < (int) key.size(); ++i) {
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


char analyze_learnts(map<vector<int>, set<vector<char>>> &learnts,
		vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes)
{
	char cnt = 0;
	for (auto it = learnts.begin(); it != learnts.end(); ) {
		auto key = it -> first;
		vector<vector<char>> negations(all(it -> second));
		bool change_key = 0;
		for (int i = 0; i < (int) key.size(); ++i) {
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


void simple_linear_propagation(set<vector<int>> &linear_constraints,
		vector<vector<int>> &relations,
		vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu)
{
	vector<vector<int>> changes;
	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ) {
		char useless = 0, value = 0, change = 1;
		vector<int> equation_vector;
		map<int, int> equation_map;
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


void linear_propagation(set<vector<int>> &linear_constraints,
		vector<vector<int>> &relations)
{
	vector<vector<int>> equations_by_var(vars_cnt + 1);
	int counter = 0;
	/* Gauss algorithm stage 1 */
	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ++it) {
		for (int i = 1; i < (int)(*it).size(); ++i)
			equations_by_var[(*it)[i] / 2].push_back(counter);
		++counter;
		auto it1 = it;
		++it1;
		while (it1 != linear_constraints.end() && ((*it1)[0] / 2) == ((*it)[0] / 2)) {
			vector<int> res;
			equations_xor(*it, *it1, res);
			if (!res.empty())
				linear_constraints.insert(res);
			it1 = linear_constraints.erase(it1);
		}
	}
	/// СОХРАНИТЬ СИСТЕМУ В vector<vector<int>> ls
	/// ДЛЯ КАЖДОГО x \in ls СОХРАНИТЬ ВСЕ УРАВНЕНИЯ, СОДЕРЖАЩИЕ x[0] или x[0] ^ 1
	/// ИСПОЛЬЗОВАТЬ vector<vector<int>> equations_by_vars РАЗМЕРА vars_cnt + 1

	vector<vector<int>> ls(all(linear_constraints));
	/* Gauss algorithm stage 2 */
	for (int i = ls.size() - 1; i >= 0; --i) {
		/// ЕСЛИ ls[i] СОДЕРЖИТ ЛИНЕЙНОЕ СООТНОШЕНИЕ
		/// НАД 1 ИЛИ 2 ПЕРЕМЕНЫМИ, НАДО ЕГО СОХРАНИТЬ
		if (ls[i].size() <= 2)
			relations.push_back(ls[i]);

		for (int j : equations_by_var[ls[i][0] / 2]) {
			vector<int> res;
			equations_xor(ls[i], ls[j], res);
			ls[j] = res;
			if (ls[j].size() <= 2)
				relations.push_back(ls[j]);
		}
	}
	linear_constraints.clear();
	linear_constraints.insert(all(ls));
}


char analyze_relations(vector<vector<int>> &relations,
		vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes)
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


// int get_rank(set<vector<int>> linear_constraints) {
// 	int counter = 0;
// 	/* Gauss algorithm stage 1 */
// 	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ++it) {
// 		++counter;
// 		auto it1 = it;
// 		++it1;
// 		while (it1 != linear_constraints.end() && ((*it1)[0] & -2) == ((*it)[0] & -2)) {
// 			vector<int> res;
// 			equations_xor(*it, *it1, res);
// 			if (!res.empty())						//
// 				linear_constraints.insert(res);		// TODO: is linear_constraints able to be broken?
// 			it1 = linear_constraints.erase(it1);	//
// 		}
// 	}
// 	return linear_constraints.size();
// }


char analyze_equations(vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes,
		map<pair <int, int>, int> &gate_pairs,
		map<vector<int>, set<vector<char>>> &learnts,
		set<vector<int>> &linear_constraints,
		vector<char> &useless_equations)
{
	char res = 0;

	for (int i = 0; i < and_equations_cnt; ++i) {
		if (useless_equations[i])
			continue;

		char useless;
		res |= propagation(i, vars_values, is_def,
			dsu, classes, gate_pairs, learnts,
			linear_constraints, useless);
		if (useless) {
			useless_equations[i] = 1;
			// TODO: STORE equations COPY IN list<AndEquation> AND ERASE e FROM IT HERE
		}
	}

	return res;
}


void solve(char &a, vector<char> &vars_values, vector<char> &is_def)
{
	map<pair <int, int>, int> gate_pairs;
	vector<int> dsu(2 * (vars_cnt + 1));
	vector<vector<int>> classes(2 * (vars_cnt + 1));
	vector<char> useless_equations(and_equations_cnt, 0);
	auto linear_constraints = pattern_linear_constraints;
	auto learnts = pattern_learnts;

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

	for (int i = 0; i < (int) dsu.size(); ++i) {
		dsu[i] = i;
		classes[i].push_back(i);
	}

	while (true) {
		char cnt;

		while (true) {
			cnt = analyze_equations(vars_values, is_def, dsu, classes,
				gate_pairs, learnts, linear_constraints, useless_equations);

			vector<vector<int>> relations;
			simple_linear_propagation(linear_constraints, relations, vars_values, is_def, dsu);
			cnt |= analyze_relations(relations, vars_values, is_def, dsu, classes);

			if (!cnt)
				break;
		}

		cnt |= analyze_learnts(learnts, vars_values, is_def, dsu, classes);

		if (!cnt) {
			vector<vector<int>> relations;
			auto linear_constraints_copy = linear_constraints;
			linear_propagation(linear_constraints_copy, relations);
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
}


void find_output(char &a, vector<char> &vars_values, vector<char> &is_def)
{
	map<pair <int, int>, int> gate_pairs;
	vector<int> dsu(2 * (vars_cnt + 1));
	vector<vector<int>> classes(2 * (vars_cnt + 1));
	vector<char> useless_equations(and_equations_cnt, 0);
	auto linear_constraints = pattern_linear_constraints;
	auto learnts = pattern_learnts;

	auto linear_constraints_copy = linear_constraints;

	for (int i = 0; i < (int)dsu.size(); ++i) {
		dsu[i] = i;
		classes[i].push_back(i);
	}

	while (true) {
		char cnt;
		
		while (true) {
			cnt = analyze_equations(vars_values, is_def, dsu, classes,
				gate_pairs, learnts, linear_constraints, useless_equations);
			
			vector<vector<int>> relations;

			simple_linear_propagation(linear_constraints, relations, vars_values, is_def, dsu);

			cnt |= analyze_relations(relations, vars_values, is_def, dsu, classes);

			if (!cnt)
				break;
		}

		cnt |= analyze_learnts(learnts, vars_values, is_def, dsu, classes);

		if (!cnt) {
			vector<vector<int>> relations;
			auto linear_constraints_copy = linear_constraints;
			linear_propagation(linear_constraints_copy, relations);
			cnt |= analyze_relations(relations, vars_values, is_def, dsu, classes);
		}

		if (!cnt)
			break;
	}

	for (int x: all_vars) {
		vars_values[x] = vars_values[dsu[x]];
		vars_values[x ^ 1] = vars_values[dsu[x ^ 1]];
	}
}



void save_interp(const vector<char> &vars_values, int i)
{
	for (int j = 0; j < 2 * vars_cnt + 2; ++j)
		interp[i][j] = vars_values[j];
}


void gen_random_sample()
{
	interp.resize(N, vector<char>(2 * vars_cnt + 2));

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < N; ++i) {
		vector<char> vars_values(2 * (vars_cnt + 1));
		vector<char> is_def(2 * (vars_cnt + 1));
		
		fill(all(is_def), 0);
		vars_random_assignment(core_vars, vars_values, is_def);
		char a;
		find_output(a, vars_values, is_def);
		save_interp(vars_values, i);
	}
}


void clear_vars_values(vector<char> &interp, const vector<char> &vars_set,
		vector<char> &vars_values, vector<char> &is_def)
{
	fill(all(is_def), 0);

	for (int j = 0; j < guessed_vars_cnt; ++j) {
		if (vars_set[j])
			define_variable_value(guessed_vars[j], interp[guessed_vars[j]], vars_values, is_def);
	}

	for (int j = 0; j < output_vars_cnt; ++j)
		define_variable_value(output_vars[j], interp[output_vars[j]], vars_values, is_def);
}


double complexity(const vector<char> &vars_set)
{
	auto it = complexity_set.find(vars_set);
	if (it != complexity_set.end())
		return it -> second;

	int cnt = 0;
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < N; ++i) {
		char answer;
		vector<char> vars_values(2 * (vars_cnt + 1));
		vector<char> is_def(2 * (vars_cnt + 1));
		
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

	return res;
}


void print_complexity(const vector<char> &vars_set, const string &filename)
{
	ofstream fout(filename.data());
	
	fout << bin_set_size(vars_set) << ": ";
	for (int i = 0; i < guessed_vars_cnt; ++i) {
		if (vars_set[i])
			fout << guessed_vars[i] << " ";
	}
	fout << "\n";
	fout << "complexity: "  << complexity(vars_set) << "\n";
	fout << "probability: " << probability_set[vars_set] << "\n";

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

	read_linear_constraints(pattern_linear_constraints, linear_constraints_filename);

	all_vars = vector<int> (all(all_vars_set));
	clog << "all vars size: " << all_vars.size() << endl;

	read_lin_table(lin_table_filename, lin_table);
	
	vector<char> start_point;
	read_start_point_file(start_point_filename, start_point);

	gen_random_sample();

	print_complexity(start_point, out_filename);

}
