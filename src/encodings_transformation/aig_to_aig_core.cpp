#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

typedef long long ll;
typedef unsigned int uint;

using namespace std;

// mutex log_mtx;

mt19937_64 gen(time(0));

/// УРАВНЕНИЕ and
/// x ^ y & z = 0 ( x = y & z )
/// ПОД x, y, z ПОНИМАЮТСЯ ПЕРЕМЕННЫЕ
/// С НОМЕРАМИ x, y, z СООТВЕТСТВЕННО
class and_equation {
public:
	uint x;
	uint y;
	uint z;
};

bool operator== (const and_equation &a, const and_equation &b)
{
	return a.y == b.y && a.z == b.z;
}

bool operator< (const and_equation &a, const and_equation &b)
{
	return (((a.y & 1) << 1) ^ (a.z & 1)) < (((b.y & 1) << 1) ^ (b.z & 1));
}

char random_bool()
{
	return gen() & 1;
}


/// МНОЖЕСТВО ВСЕХ УРАВНЕНИЙ
vector <and_equation> equations;
set <vector <uint>> pattern_linear_constraints;
map <vector <uint>, set <vector <char>>> pattern_learnts;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector <uint> key_vars;
vector <uint> iv_vars;
vector <uint> input_vars;
vector <uint> output_vars;


/// КОЛИЧЕСТВО УРАВНЕНИЙ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
uint and_equations_cnt = 0;


/// КОЛИЧЕСТВО ВСЕХ ПЕРЕМЕННЫХ И
/// ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
uint vars_cnt = 0;
uint input_vars_cnt = 0;
uint key_vars_cnt = 0;
uint iv_vars_cnt = 0;
uint output_vars_cnt = 0;
uint latches_cnt = 0;


/// ЧТЕНИЕ ПАРАМЕТРОВ ИЗ ТЕРМИНАЛА
void init(int argc, char *argv[],
		string &in_filename, string &out_filename,
		string &sub_filename, string &core_filename,
		string &lin_filename, string &learnts_filename)
{
	in_filename		 = "";
	out_filename	 = "";
	sub_filename	 = "";
	core_filename	 = "";
	lin_filename	 = "";
	learnts_filename = "";

	for (uint i = 1; i < (uint) argc; ++i) {
		string param;
		uint j = 0;

		for (; argv[i][j] != '=' && argv[i][j] != 0; ++j)
			param.insert(param.end(), argv[i][j]);

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help											  Вывод этой справки и выход.\n";
			cout << "  -i, --input <file>									  Файл с описанием И-Не графа.\n";
			cout << "  -o, --output <file>									  Файл для вывода упрощённого И-Не графа.\n";
			cout << "  -s, --substitution <file>	default=substitution	  Файл с описанием подстановки.\n";
			cout << "  -c, --core <file>			default=core			  Файл с описанием множества переменных ядра.\n";
			cout << "  -L, --linear <file>			default=linear			  Файл дополнительных линейных ограничений.\n";
			cout << "  -l, --learnts <file>			default=learnts			  Файл дополнительных дизъюнктов.\n";
			cout << "  -k, --key-size <size>		default=input_vars_cnt	  Число бит ключа.\n";
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
		else if (param == "--key-size" || param == "-k") {
			if (argv[i][j] == 0)
				key_vars_cnt = (uint) atoi(argv[++i]);
			else
				key_vars_cnt = (uint) atoi(argv[i] + j + 1);
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
void log_linear_constraints(const set <vector <uint>> &linear_constraints)
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
void log_learnts(const map <vector <uint>, set <vector <char>>> &learnts) {
	for (auto &p: learnts) {
		for (auto &v: p.second) {
			for (uint i = 0; i < p.first.size(); ++i)
				clog << (v[i] ? "-" : "") << p.first[i] / 2 << " ";
			clog << "0\n";
		}
	}
}
*/

void print_new_linear_constraints(const set <vector <uint>> &linear_constraints,
		const string &filename, const vector <uint> &vars_map)
{
	ofstream fout(filename.data());
	for (auto &v: linear_constraints) {
		for (auto x: v)
			fout << vars_map[x] << " ";
		fout << "\n";
	}
	fout.close();
}

/// PRINTS new_learnts IN DIMACS FORMAT
void print_new_learnts(const map <vector <uint>, set <vector <char>>> &learnts,
		const string &filename, const vector <uint> &vars_map)
{
	ofstream fout(filename.data());
	for (auto &p: learnts) {
		uint n = p.first.size();
		for (auto &v: p.second) {
			for (uint i = 0; i < n; ++i) {
				int x = vars_map[p.first[i] ^ v[i]];
				fout << (x & 1 ? - x / 2 : x / 2) << " ";
			}
			fout << "0\n";
		}
	}
	fout.close();
}


void print_vars_map(const string &filename, const vector <uint> &vars_map)
{
	ofstream fout(filename.data());
	for (auto x: vars_map)
		fout << x << "\n";
}


void print_new_vars(const vector <uint> &v, const vector <uint> &vars_map,
		const string &filename)
{
	ofstream fout(filename.data());
	for (auto x: v)
		fout << vars_map[x] << endl;
	fout.close();
}


char add_learnts(map <vector <uint>, set <vector <char>>> &learnts,
		vector <uint> &key_value)
{
	sort(all(key_value));
	vector <uint> key(key_value.size());
	vector <char> negations(key_value.size());
	for (uint i = 0; i < key_value.size(); ++i) {
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


void equations_xor(vector <uint> e1, vector <uint> e2,
		vector <uint> &res)
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
		vector <uint> &dsu)
{
	clog << "variables values\n" <<
		"var is_def value dsu\n";
	for (uint x = 1; x <= vars_cnt; ++x) {
		uint y = dsu[2 * x];
		clog << 2 * x << " " << (int)is_def[y] << " " <<
			(int)vars_values[y] << " " << y << "\n";
	}
}
*/

void reduce_constraints(set <vector <uint>> &linear_constraints) {
	vector <vector <uint>> new_lc;
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

	for (uint i = 0; i < key_vars_cnt; ++i) {
		fin >> key_vars[i];
		input_vars[i] = key_vars[i];
	}
	for (uint i = 0; i < iv_vars_cnt; ++i) {
		fin >> iv_vars[i];
		input_vars[key_vars_cnt + i] = iv_vars[i];
	}

	clog << "ok" << endl;
}

void read_output(ifstream &fin)
{
	clog << " reading output ... ";

	output_vars.resize(output_vars_cnt);

	for (uint i = 0; i < output_vars_cnt; ++i)
		fin >> output_vars[i];

	clog << "ok" << endl;
}

void read_equations(ifstream &fin)
{
	clog << " reading equations ... ";

	equations.resize(and_equations_cnt);
	for (uint i = 0; i < and_equations_cnt; ++i) {
		uint x, y, z;
		fin >> x >> y >> z;
		equations[i] = {x, min(y, z), max(y, z)};
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


void read_linear_constraints(set <vector <uint>> &linear_constraints,
		const string &filename)
{
	clog << "reading additional linear constraints from \'" << filename << "\' ... ";
	ifstream fin(filename.data());

	string line;
	stringstream ss;
	while (getline(fin, line)) {
		ss.clear();
		ss << line;
		uint x;
		vector <uint> equation;
		while (ss >> x)
			equation.push_back(x);
		linear_constraints.insert(equation);
	}

	fin.close();
	clog << "ok" << endl;
}

/// READ LEARNTS FROM THE FILE.
/// DATA ARE PRESENTED IN DIMACS FORMAT (WITHOUT HEADER).
void read_learnts(map <vector <uint>, set <vector <char>>> &learnts,
		const string &filename)
{
	clog << "reading additional learnts from \'" << filename << "\' ... ";
	ifstream fin(filename.data());
	int x;
	vector <uint> key_value;
	while (fin >> x) {
		if (x == 0) {
			vector <uint> key(key_value.size());
			vector <char> negations(key_value.size());
			sort(all(key_value));
			for (uint i = 0; i < key_value.size(); ++i) {
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


void read_core_vars(const string &filename, vector <uint> &core_vars)
{
	clog << "reading core variables from \'" << filename << "\' ... ";
	ifstream fin(filename.data());

	uint x;
	while (fin >> x)
		core_vars.push_back(x);

	fin.close();
	clog << "ok" << endl;
}


/**************************
 * simplification methods *
 **************************/
void define_variable_value(uint var, char val,
		vector <char> &vars_values, vector <char> &is_def,
		const vector <uint> &dsu)
{
	var = dsu[var];
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


void join_sets(uint x, uint y,
		vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu, vector <vector <uint>> &classes)
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

	for (uint z: classes[y]) {
		dsu[z] = x;
		dsu[z ^ 1] = x ^ 1;
		classes[x].push_back(z);
		classes[x ^ 1].push_back(z ^ 1);
	}
	classes[y].clear();
	classes[y ^ 1].clear();
}


void vars_random_assignment(const vector <uint> &vars,
		vector <char> &vars_values, vector <char> &is_def, vector <uint> &dsu)
{
	for (uint var: vars)
		define_variable_value(var, random_bool(), vars_values, is_def, dsu);
}


void read_substitution(const string &filename, vector <char> &vars_values,
		vector <char> &is_def, vector <uint> &dsu)
{
	clog << "reading substitution from \'" << filename << "\' ... ";
	ifstream fin(filename.data());

	uint x;
	while (fin >> x)
		define_variable_value(x, 0, vars_values, is_def, dsu);

	fin.close();
	clog << "ok" << endl;
}


void find_linear_constraints(set <vector <uint>> &linear_constraints)
{
	clog << "finding additional linear constraints ... ";
	map <pair <uint, uint>, vector <and_equation>> similar_gates;
	for (auto &e: equations)
		similar_gates[{e.y & -2, e.z & -2}].push_back(e);

	for (auto &p: similar_gates) {
		auto s = p.second;
		order(s);
		if (s.size() == 1)
			continue;

		auto key = p.first;
		uint a = key.first + (s[0].y & 1), b = key.second + (s[0].z & 1);
		uint id0 = ((s[0].y & 1) << 1) ^ (s[0].z & 1);
		vector <uint> vars;

		for (uint i = 1; i < s.size(); ++i) {
			uint idi = ((s[i].y & 1) << 1) ^ (s[i].z & 1) ^ id0;
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


/// ВЫВОД ИЗ КВАДРАТИЧНОГО УРАВНЕНИЯ.
/// ВОЗВРАЩАЕТ 1, ЕСЛИ ХОТЬ ЧТО-ТО ВЫВЕЛОСЬ.
/// ИНАЧЕ 0. ИСПОЛЬЗУЕТ КЛАССЫ ЭКВИВАЛЕНТНОСТИ.
char propagation(const and_equation &e,
		vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu, vector <vector <uint>> &classes,
		map <vector <uint>, set <vector <char>>> &learnts,
		char &useless)
{
	useless = 1;

	uint x = dsu[e.x];
	uint y = dsu[e.y];
	uint z = dsu[e.z];

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
		vector <uint> key_value = {x ^ 1, z};
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
		vector <uint> key_value = {x ^ 1, y};
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
		vector <uint> key_value = {y ^ 1, z ^ 1};
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
char propagation(const and_equation &e,
		vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu, vector <vector <uint>> &classes,
		map <pair <uint, uint>, uint> &gate_pairs,
		map <vector <uint>, set <vector <char>>> &learnts,
		char &useless)
{
	uint x = dsu[e.x];
	uint y = dsu[e.y];
	uint z = dsu[e.z];

	if (is_def[x] && is_def[y] && is_def[z]) {
		useless = 1;
		return 0;
	}

	if (y > z)
		swap(y, z);
	pair <uint, uint> key = {y, z};

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


char learnts_propagation(vector <uint> &key, vector <vector <char>> &negations,
		vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu, vector <vector <uint>> &classes,
		char &useless)
{
	useless = 1;
	uint n = key.size();
	vector <vector <uint>> disjuncts;
	vector <uint> variables = key;
	for (auto &x: variables)
		x = dsu[x];
	for (auto &v: negations) {
		vector <uint> disjunct;
		bool skip = 0; // applying UP rule flag
		for (uint i = 0; i < key.size(); ++i) {
			uint x = variables[i] ^ v[i];
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
		cerr << "error: void learnts_propagation(): empty disjunct can't be solved\n"; // TODO: throw runtime_error("...")
		exit(0);
	}
	else if (n == 1) {
		for (auto &d: disjuncts) {
			uint x = dsu[d[0]];
			define_variable_value(x, 1, vars_values, is_def, dsu);
		}
		// useless = 1;
		return 1;
	}
	else if (n == 2) {
		if (disjuncts.size() == 1) {
			auto d = disjuncts[0];
			uint x = dsu[d[0]], y = dsu[d[1]];
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
			uint x1 = dsu[d0[0]], y1 = dsu[d0[1]],
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
			uint x = 0, y = 0;
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
			cerr << "error: void learnts_propagation(): " <<
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


char analyze_learnts(map <vector <uint>, set <vector <char>>> &learnts,
		vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu, vector <vector <uint>> &classes)
{
	map <vector <uint>, set <vector <char>>> changes;
	char cnt = 0;
	for (auto it = learnts.begin(); it != learnts.end(); ) {
		auto key = it -> first;
		vector <vector <char>> negations(all(it -> second));

		bool change_vars = 0;
		for (uint i = 0; i < key.size(); ++i) {
			uint x = key[i];
			if (dsu[x] != x) {
				change_vars = 1;
				key[i] = dsu[x] & -2;
				if (dsu[x] & 1) {
					for (auto &v: negations)
						v[i] ^= 1;
				}
			}
		}
		if (change_vars) {
			changes[key].insert(all(negations));
			it = learnts.erase(it);
			cnt |= 1;
			continue;
		}

		bool remove_vars = 0;
		for (auto x: key) {
			if (is_def[x])
				remove_vars = 1;
		}
		if (remove_vars) {
			vector <uint> new_key;
			for (auto x: key) {
				if (!is_def[x])
					new_key.push_back(x);
			}
			vector <vector <char>> new_negations;
			for (auto &v: negations) {
				vector <char> new_v;
				bool useless = 0;
				for (uint i = 0; i < key.size(); ++i) {
					if (!is_def[key[i]]) {
						new_v.push_back(v[i]);
						continue;
					}
					else if (vars_values[key[i] ^ v[i]] == 1) {
						useless = 1;
						break;
					}
				}
				if (!useless) {
					if (new_v.empty()) {
						throw runtime_error("char analyze_learnts(): error: empty disjunct can't be solved");
					}
					new_negations.push_back(new_v);
				}
			}
			if (!new_negations.empty())
				changes[new_key].insert(all(new_negations));
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
	for (auto &p: changes)
		learnts[p.first].insert(all(p.second));
	return cnt;
}


void simple_linear_propagation(set <vector <uint>> &linear_constraints,
		vector <vector <uint>> &relations,
		vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu)
{
	vector <vector <uint>> changes;
	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ) {
		char useless = 0, value = 0, change = 1;
		vector <uint> equation_vector;
		map <uint, uint> equation_map;
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
		// 	useless = 0;
		// }
		

		if (useless) {
			it = linear_constraints.erase(it);
			continue;
		}
		else if (change) {
			changes.push_back(equation_vector);
			for (uint x: equation_vector) {
				if (x != dsu[x])
					clog << "!!!\n";
			}
			it = linear_constraints.erase(it);
			continue;
		}
		else {
			++it;
		}
	}
	linear_constraints.insert(all(changes));
}


void linear_propagation(set <vector <uint>> &linear_constraints,
		vector <vector <uint>> &relations)
{
	vector <vector <uint>> equations_by_var(vars_cnt + 1);
	int counter = 0;
	/* Gauss algorithm stage 1 */
	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ++it) {
		for (uint i = 1; i < (*it).size(); ++i)
			equations_by_var[(*it)[i] / 2].push_back(counter);
		++counter;
		auto it1 = it;
		++it1;
		while (it1 != linear_constraints.end() && ((*it1)[0] / 2) == ((*it)[0] / 2)) {
			vector <uint> res;
			
			equations_xor(*it, *it1, res);
			if (!res.empty())
				linear_constraints.insert(res);
			it1 = linear_constraints.erase(it1);
		}
	}
	/// СОХРАНИТЬ СИСТЕМУ В vector <vector <int>> ls
	/// ДЛЯ КАЖДОГО x \in ls СОХРАНИТЬ ВСЕ УРАВНЕНИЯ, СОДЕРЖАЩИЕ x[0] или x[0] ^ 1
	/// ИСПОЛЬЗОВАТЬ vector <vector <int>> equations_by_vars РАЗМЕРА vars_cnt + 1
		
	vector <vector <uint>> ls(all(linear_constraints));
	/* Gauss algorithm stage 2 */
	for (int i = ls.size() - 1; i >= 0; --i) {
		/// ЕСЛИ ls[i] СОДЕРЖИТ ЛИНЕЙНОЕ СООТНОШЕНИЕ
		/// НАД 1 ИЛИ 2 ПЕРЕМЕНЫМИ, НАДО ЕГО СОХРАНИТЬ
		if (ls[i].size() <= 2)
			relations.push_back(ls[i]);

		for (int j : equations_by_var[ls[i][0] / 2]) {
			vector <uint> res;
			equations_xor(ls[i], ls[j], res);
			ls[j] = res;
			if (ls[j].size() <= 2)
				relations.push_back(ls[j]);
		}
	}
	linear_constraints.clear();
	linear_constraints.insert(all(ls));
}


char analyze_relations(vector <vector <uint>> &relations,
		vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu, vector <vector <uint>> &classes)
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


void simplify_aig(vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu, vector <vector <uint>> &classes,
		vector <char> &useless_equations,
		set <vector <uint>> &linear_constraints,
		map <vector <uint>, set <vector <char>>> &learnts)
{
	map <pair <uint, uint>, uint> gate_pairs;

	while (true) {
		char cnt;
		while (true) {
			cnt = 0;
			for (uint i = 0; i < and_equations_cnt; ++i) {
				if (useless_equations[i])
					continue;

				char useless;
				auto e = equations[i];
				cnt |= propagation(e, vars_values, is_def,
					dsu, classes, gate_pairs, learnts, useless);

				if (useless) {
					useless_equations[i] = 1;
					// TODO: STORE equations COPY IN list<and_equation> AND ERASE e FROM IT HERE
				}
			}
			if (!cnt)
				break;
		}
		vector <vector <uint>> relations;
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
}


void build_new_aig(vector <char> &vars_values, vector <char> &is_def,
		vector <uint> &dsu, vector <vector <uint>> &classes,
		vector <and_equation> &new_equations, const vector <char> &useless_equations,
		vector <uint> &new_key_vars, vector <uint> &new_output_vars,
		set <vector <uint>> &linear_constraints,
		map <vector <uint>, set <vector <char>>> &learnts,
		vector <uint> &vars_map)
{
	vars_map.resize(2 * (vars_cnt + 1), 0);
	vector <char> used(2 * (vars_cnt + 1), 0);
	for (uint i = 2; i <= 2 * vars_cnt; i += 2) {
		if (used[dsu[i]])
			continue;
		uint mn = i;
		uint h = dsu[mn];
		if (mn == h) {
			used[mn] = 1;
			used[mn ^ 1] = 1;
			continue;
		}
		swap(classes[h], classes[mn]);
		swap(classes[h ^ 1], classes[mn ^ 1]);
		for (uint x: classes[mn]) {
			dsu[x] = mn;
			dsu[x ^ 1] = mn ^ 1;
		}
		if (is_def[h] && !is_def[mn])
			define_variable_value(mn, vars_values[h], vars_values, is_def, dsu);
		used[mn] = 1;
		used[mn ^ 1] = 1;
	}
	// now dsu[x] <= x \forall x

	vector <uint> useless_variables(2 * (vars_cnt + 1), 1);
	for (uint i = 0; i < equations.size(); ++i) {
		if (!useless_equations[i]) {
			useless_variables[dsu[equations[i].x]] = 0;
			useless_variables[dsu[equations[i].x] ^ 1] = 0;
			useless_variables[dsu[equations[i].y]] = 0;
			useless_variables[dsu[equations[i].y] ^ 1] = 0;
			useless_variables[dsu[equations[i].z]] = 0;
			useless_variables[dsu[equations[i].z] ^ 1] = 0;
		}
	}

	// формирование linear_constraints
	set <vector <uint>> linear_constraints_copy;
	for (auto v: linear_constraints) {
		char b = 0;
		for (auto &x: v) {
			x = dsu[x];
			b ^= x & 1;
			x &= -2;
		}
		v[0] ^= b;
		linear_constraints_copy.insert(v);
	}
	swap(linear_constraints, linear_constraints_copy);
	for (auto &v: linear_constraints) {
		for (auto x: v) {
			useless_variables[dsu[x]] = 0;
			useless_variables[dsu[x] ^ 1] = 0;
		}
	}

	// формирование learnts
	map <vector <uint>, set <vector <char>>> learnts_copy;
	for (auto &p: learnts) {
		auto key = p.first;
		vector <vector <char>> values(all(p.second));
		for (uint i = 0; i < key.size(); ++i) {
			key[i] = dsu[key[i]];
			if (key[i] & 1) {
				key[i] ^= 1;
				for (auto &v: values)
					v[i] ^= 1;
			}
		}
		learnts_copy[key].insert(all(values));
	}
	swap(learnts, learnts_copy);
	for (auto &p: learnts) {
		for (auto x: p.first) {
			useless_variables[dsu[x]] = 0;
			useless_variables[dsu[x] ^ 1] = 0;
		}
	}

	// формирование vars_map
	uint var = 1;	
	for (uint i = 2; i <= 2 * vars_cnt; i += 2) {
		if (!useless_variables[i]) {
			vars_map[i] = 2 * var;
			vars_map[i ^ 1] = 2 * var + 1;
			++var;
		}
	}

	// формирование new_key_vars
	for (uint i = 0; i < key_vars_cnt; ++i) {
		uint key_var = dsu[key_vars[i]];
		if (!useless_variables[key_var]) {
			if (is_def[key_var])
				clog << "warning: var " << key_var << " " << vars_map[key_var] << " is defined but not useless" << endl;
			new_key_vars.push_back(vars_map[key_var]);
		}
	}

	// формирование new_output_vars
	for (uint i = 0; i < output_vars_cnt; ++i) {
		uint out_var = dsu[output_vars[i]];
		if (!useless_variables[out_var]) {
			if (is_def[out_var])
				clog << "warning: var " << out_var << " " << vars_map[out_var] << " is defined but not useless" << endl;
			new_output_vars.push_back(vars_map[out_var]);
		}
	}

	// формирование new_equations
	for (uint i = 0; i < and_equations_cnt; ++i) {
		if (useless_equations[i])
			continue;
		auto e = equations[i];
		uint x = vars_map[dsu[e.x]];
		uint y = vars_map[dsu[e.y]];
		uint z = vars_map[dsu[e.z]];
		new_equations.push_back({x, y, z});
	}
}


void print_aig(const string &out_filename, vector <and_equation> &new_equations,
		vector <uint> &new_key_vars, vector <uint> &new_output_vars, vector <uint> &new_vars)
{
	uint new_key_vars_cnt = new_key_vars.size();
	uint new_output_vars_cnt = new_output_vars.size();
	uint new_vars_cnt = new_equations.size() + new_key_vars_cnt;
	uint new_latches_cnt = 0;

	ofstream fout(out_filename.data());

	fout << "aag " << new_vars_cnt << " " << new_key_vars_cnt << " " << new_latches_cnt << " "
		 << new_output_vars_cnt << " " << new_equations.size() << endl;
	for (uint i = 0; i < new_key_vars_cnt; ++i)
		fout << new_key_vars[i] << "\n";

	for (uint i = 0; i < new_output_vars_cnt; ++i) {
			fout << new_output_vars[i] << "\n";
	}

	for (auto &e: new_equations)
		fout << e.x << " " << min(e.y, e.z) << " " << max(e.y, e.z) << "\n";

	fout.close();
}


void aig_to_aig(const string &substitution_filename, const string &out_filename,
		const string &linear_constraints_filename, const string &learnts_filename,
		const string &core_vars_filename)
{
	clog << "simplifying aig" << endl;

// 	int counter = 0;
//	while (true) {
// cout << counter << endl;
	vector <char> vars_values(2 * (vars_cnt + 1), 0),
		is_def(2 * (vars_cnt + 1), 0);
	vector <uint> dsu(2 * (vars_cnt + 1));
	vector <vector <uint>> classes(2 * (vars_cnt + 1));
	vector <char> useless_equations(and_equations_cnt, 0);
	auto linear_constraints = pattern_linear_constraints;
	auto learnts = pattern_learnts;

	for (uint i = 0; i < dsu.size(); ++i) {
		dsu[i] = i;
		classes[i].push_back(i);
	}

	read_substitution(substitution_filename, vars_values, is_def, dsu);
	vector <uint> core_vars;
	read_core_vars(core_vars_filename, core_vars);

	vars_random_assignment(core_vars, vars_values, is_def, dsu);
// cout << counter << endl;
	simplify_aig(vars_values, is_def, dsu, classes, useless_equations,
		linear_constraints, learnts);

	bool ok = 1;
	for (auto x: output_vars)
		if (!is_def[dsu[x]])
			ok = 0;
	if (!ok)
		cout << "error" << endl;
	else
		cout << "ok" << endl;
// 		++counter;
//	}

	vector <and_equation> new_equations;
	vector <uint> new_key_vars, new_output_vars, vars_map;

	build_new_aig(vars_values, is_def, dsu, classes,
		new_equations, useless_equations,
		new_key_vars, new_output_vars,
		linear_constraints, learnts, vars_map);

	print_aig(out_filename, new_equations, new_key_vars, new_output_vars, vars_map);
	print_new_linear_constraints(linear_constraints,
		linear_constraints_filename + "_new", vars_map);
	print_new_learnts(learnts, learnts_filename + "_new", vars_map);
	print_vars_map("vars_map", vars_map);
	print_new_vars(core_vars, vars_map, "new_core_vars");

	clog << "ok" << endl;

}


/*****************
 * main function *
 *****************/
int main(int argc, char *argv[])
{

	string in_filename, out_filename;
	string substitution_filename, core_vars_filename;
	string linear_constraints_filename, learnts_filename;
	init(argc, argv, in_filename, out_filename,
		substitution_filename, core_vars_filename,
		linear_constraints_filename, learnts_filename);

	read_aig(in_filename);

	find_linear_constraints(pattern_linear_constraints);
	read_linear_constraints(pattern_linear_constraints, linear_constraints_filename);
	reduce_constraints(pattern_linear_constraints);

	read_learnts(pattern_learnts, learnts_filename);

	aig_to_aig(substitution_filename, out_filename,
		linear_constraints_filename, learnts_filename,
		core_vars_filename);

}
