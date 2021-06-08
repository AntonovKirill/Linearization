#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

typedef long long ll;

using namespace std;

int N = 0;

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


const vector<int> gate_values = {0, 1, 2, 7};
vector<vector<vector<int>>> gate_constraints;

vector<vector<vector<vector<Bit>>>> selectors;
map<long long, vector<vector<vector<Bit>>>> selectors_map;

vector<vector<long long>> bs_hint;

/// ЧТЕНИЕ ПАРАМЕТРОВ ИЗ ТЕРМИНАЛА
void init(int argc, char *argv[], string &in_filename, string &core_filename,
		string &lin_filename, string &lt_filename, int &k, int &n)
{
	in_filename   = "";
	core_filename = "";
	lin_filename  = "";
	lt_filename   = "";
	k = 0;
	n = 1;

	for (int i = 1; i < (int) argc; ++i) {
		string param;
		int j = 0;

		for (; argv[i][j] != '=' && argv[i][j] != 0; ++j)
			param.insert(param.end(), argv[i][j]);

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help                                              Вывод этой справки и выход.\n";
			cout << "  -i, --input <file>                                      Файл с описанием И-Не графа.\n";
			cout << "  -N, --sample-size <size>                                Размер выборки.\n";
			cout << "  -c, --core <file>              default=core             Файл с описанием множества переменных ядра.\n";
			cout << "  -L, --linear <file>            default=linear           Файл дополнительных линейных ограничений.\n";
			cout << "  -lt, --lin-table <file>        default=lin-table        Файл с селекторными ограничениями.\n";
			cout << "  -k, --order <int>              default=0                Порядок селекторов, по которым выводятся ограничения.\n";
			cout << "  -n, --vars <int>               default=1                Число переменных линейных ограничений.\n";
			exit(0);
		}
		else if (param == "--input" || param == "-i") {
			if (argv[i][j] == 0)
				in_filename = (string) (argv[++i]);
			else
				in_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--sample-size" || param == "-N") {
			if (argv[i][j] == 0)
				N = atoi(argv[++i]);
			else
				N = atoi(argv[i] + j + 1);
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
		else if (param == "--lin-table" || param == "-lt") {
			if (argv[i][j] == 0)
				lt_filename = (string) argv[++i];
			else
				lt_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "--order" || param == "-k") {
			if (argv[i][j] == 0)
				k = atoi(argv[++i]);
			else
				k = atoi(argv[i] + j + 1);
		}
		else if (param == "--vars" || param == "-n") {
			if (argv[i][j] == 0)
				n = atoi(argv[++i]);
			else
				n = atoi(argv[i] + j + 1);
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
	if (N <= 0) {
		cerr << "error: void init(): " <<
			"value of parameter -N (--sample-size) must be a positive number\n";
		exit(0);
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
	if (lt_filename.empty()) {
		lt_filename = "lin-table";
		clog << "warning: void init(): " <<
			"lt_filename has been set to default value \'lin-table\'\n";
	}
	if (k < 0) {
		k = 0;
		clog << "warning: void init(): value of k has been set to default value 0\n";
	}
	if (n <= 0) {
		n = 1;
		clog << "warning: void init(): value of n has been set to default value 1\n";
	}
}


/***********************
 * general subprograms *
 ***********************/
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


int random_int()
{
	gen_mtx.lock();
	int res = generator();
	gen_mtx.unlock();
	return res;
}


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

	input_vars.resize(input_vars_cnt);
	
	for (int i = 0; i < input_vars_cnt; ++i)
		fin >> input_vars[i];
	
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

	gate_constraints.assign(and_equations_cnt, vector<vector<int>> (4));

	equations.resize(and_equations_cnt);
	for (int i = 0; i < and_equations_cnt; ++i) {
		int x, y, z;
		fin >> x >> y >> z;
		equations[i] = {x, min(y, z), max(y, z)};
		
		gate_constraints[i][0] = {x ^ 1, y, z};
		gate_constraints[i][1] = {x, z};
		gate_constraints[i][2] = {x, y};
		gate_constraints[i][3] = {x};

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
	stringstream ss;

	while (getline(fin, line)) {
		ss.clear();
		ss << line;
	
		int x, r = 0;
		vector<int> equation;

		while (ss >> x) {
			equation.push_back(x & -2);
			r ^= x & 1;

			all_vars_set.insert(x & -2);
		}

		if (equation.empty())
			continue;

		sort(all(equation));
		equation[0] ^= r;

		linear_constraints.insert(equation);
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


void read_lin_table(const std::string &lt_filename,
		std::map<std::vector<Bit>, std::vector<std::vector<int>>> &lt)
{
	std::clog << "reading selector constraints from \'" << lt_filename << "\' . . . ";

	std::ifstream fin(lt_filename.data());
	
	std::vector<Bit> key;
	std::string line;
	
	while (std::getline(fin, line)) {
		if (line.empty())
			continue;
		
		std::stringstream ss;
		ss << line;
		
		if (line[0] == '#') {
			key.clear();

			char sharp;
			ss >> sharp;
			
			std::vector<int> nums;
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
		std::vector<int> equation;

		while (ss >> x) {
			equation.push_back(x & -2);
			r ^= x & 1;
		}
		
		if (equation.empty())
			continue;

		std::sort(all(equation));
		equation[0] ^= r;
		lt[key].push_back(equation);
	}

	std::clog << "ok" << std::endl;
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

	if (x == (y ^ 1))
		throw runtime_error("error: void join_sets(): equation 0 = 1 has been deduced\n");

	if (is_def[x] && is_def[y]) {
		if (vars_values[x] != vars_values[y])
			throw runtime_error("error: void join_sets(): equation 0 = 1 has been deduced\n");

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
	for (int var: vars) {
		if (!is_def[var])
			define_variable_value(var, random_bool(), vars_values, is_def);
	}
}


void vars_random_assignment(const vector<int> &vars,
		vector<char> &vars_values, vector<char> &is_def, vector<int> &dsu)
{
	for (int var: vars) {
		if (!is_def[var])
			define_variable_value(var, random_bool(), vars_values, is_def, dsu);
	}
}


/// ВЫВОД ИЗ КВАДРАТИЧНОГО УРАВНЕНИЯ.
/// ВОЗВРАЩАЕТ 1, ЕСЛИ ХОТЬ ЧТО-ТО ВЫВЕЛОСЬ.
/// ИНАЧЕ 0. ИСПОЛЬЗУЕТ КЛАССЫ ЭКВИВАЛЕНТНОСТИ.
char propagation(const AndEquation &e,
		vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes,
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
		// useless = 1;
		return 0;
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
		// useless = 1;
		return 0;
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
		// useless = 1;
		return 0;
	}
	// val_x == 1
	if (!is_def[y])
		define_variable_value(y, 1, vars_values, is_def, dsu);
	if (!is_def[z])
		define_variable_value(z, 1, vars_values, is_def, dsu);
	// useless = 1;
	return 1;
}


void simple_linear_propagation(set<vector<int>> &linear_constraints,
		vector<vector<int>> &relations, vector<char> &vars_values,
		vector<char> &is_def, vector<int> &dsu)
{
	vector<vector<int>> changes;

	for (auto it = linear_constraints.begin(); it != linear_constraints.end(); ) {
		char useless = 0, value = 0, change = 0;
		vector<int> equation_vector;
		map<int, int> equation_map;

		for (auto x: *it) {
			if (x != dsu[x])
				change = 1;

			x = dsu[x];

			if (is_def[x]) {
				value ^= vars_values[x];
				change = 1;
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
			it = linear_constraints.erase(it);
			continue;
		}
		else {
			++it;
		}
	}

	linear_constraints.insert(all(changes));
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

			res |= 1;
		}
		
		if (relation.size() == 2) {
			try {
				join_sets(relation[0], relation[1], vars_values, is_def, dsu, classes);
			}
			catch (exception &e) {
				cerr << e.what() << endl;
			}

			res |= 1;
		}
	}

	relations.clear();

	return res;
}

char analyze_equations(vector<char> &vars_values, vector<char> &is_def,
		vector<int> &dsu, vector<vector<int>> &classes,
		vector<char> &useless_equations)
{
	char res = 0;

	for (int i = 0; i < and_equations_cnt; ++i) {
		if (useless_equations[i])
			continue;

		char useless;
		auto e = equations[i];
		res |= propagation(e, vars_values, is_def, dsu, classes, useless);
		if (useless) {
			useless_equations[i] = 1;
			// TODO: STORE equations COPY IN list<AndEquation> AND ERASE e FROM IT HERE
		}
	}

	return res;
}


void find_output(char &a, vector<char> &vars_values, vector<char> &is_def)
{
	vector<int> dsu(2 * (vars_cnt + 1));
	vector<vector<int>> classes(2 * (vars_cnt + 1));
	vector<char> useless_equations(and_equations_cnt, 0);
	auto linear_constraints = pattern_linear_constraints;

	for (int i = 0; i < (int)dsu.size(); ++i) {
		dsu[i] = i;
		classes[i].push_back(i);
	}

	while (true) {
		char cnt = analyze_equations(vars_values, is_def, dsu, classes, useless_equations);

		vector<vector<int>> relations;
		simple_linear_propagation(linear_constraints, relations, vars_values, is_def, dsu);
		cnt |= analyze_relations(relations, vars_values, is_def, dsu, classes);

		if (!cnt)
			break;
	}

	for (int x: all_vars) {
		is_def[x] = is_def[dsu[x]];
		is_def[x ^ 1] = is_def[dsu[x ^ 1]];
		
		vars_values[x] = vars_values[dsu[x]];
		vars_values[x ^ 1] = vars_values[dsu[x ^ 1]];
	}
}


void gen_random_sample(vector<char> &save)
{
	vector<char> vars_values(2 * (vars_cnt + 1));
	vector<char> is_def(2 * (vars_cnt + 1));

	is_def.assign(is_def.size(), 0);

	vars_random_assignment(core_vars, vars_values, is_def);

	char a;
	find_output(a, vars_values, is_def);

	for (auto x: all_vars) {
		save[x] = vars_values[x];
		save[x ^ 1] = vars_values[x ^ 1];
	}
}


pair<int, int> check_gate_constraint(const vector<int> &vec, const vector<int> &equations_indicies)
{
	int x = 0;
	for (int _x: vec)
		x = max(x, _x / 2);

	int ind = equations_indicies[x];
	
	if (ind < 0 || ind >= and_equations_cnt)
		return {-1, -1};
	
	if (equations[ind].x / 2 != x)
		return {-1, -1};
	
	int val = -1;
	for (int i = 0; i < 4; ++i) {
		auto lin = gate_constraints[ind][i];
		sort(all(lin));

		int r = 0;
		for (int &var: lin) {
			r ^= var & 1;
			var &= -2;
		}
		lin[0] ^= r;

		if (lin == vec) {
			val = i;
			break;
		}
	}

	return {ind, val};
}


void init_selectors_vector_gates()
{
	vector<int> equations_indicies(vars_cnt + 1);
	for (int i = 0; i < and_equations_cnt; ++i) {
		auto e = equations[i];
		equations_indicies[e.x / 2] = i;
	}

	selectors.clear();
	selectors.resize(and_equations_cnt);

	for (auto &sel: selectors)
		sel.resize(4);

	for (auto &p: lin_table) {
		auto key = p.first;

		for (auto &vec: p.second) {
			auto ind_val = check_gate_constraint(vec, equations_indicies);
			
			int ind = ind_val.first;
			int val = ind_val.second;

			if (val != -1)
				selectors[ind][val].push_back(key);
		}
	}
}


long long get_ind(const vector<int> &lin)
{
	long long res = 0;
	
	for (int j = 0; j < (int)lin.size(); ++j) {
		res += bs_hint[j][lin[j] - j];
		res -= bs_hint[j + 1][lin[j] - j];
	}

	return res;
}


void init_selectors_map_all(int n)
{
	vector <int> all_vars_inv(vars_cnt + 1, 0);
	for (int i = 0; i < (int)all_vars.size(); ++i)
		all_vars_inv[all_vars[i] / 2] = i;

	selectors_map.clear();

	for (const auto &v: pattern_linear_constraints) {
		if ((int)v.size() != n)
			continue;
		
		auto lin = v;
		int r = 0;
		for (int &x: lin) {
			r ^= x & 1;
			x = all_vars_inv[x / 2];
		}

		long long ind = get_ind(lin);
		selectors_map[ind].resize(2);
		selectors_map[ind][r].push_back({});
	}

	for (auto &p: lin_table) {
		auto key = p.first;

		for (auto &v: p.second) {
			if ((int)v.size() != n)
				continue;

			auto lin = v;
			int r = 0;
			for (int &x: lin) {
				r ^= x & 1;
				x = all_vars_inv[x / 2];
			}

			long long ind = get_ind(lin);
			selectors_map[ind].resize(2);
			selectors_map[ind][r].push_back(key);
		}
	}
}


template <typename T>
bool find_subset(const vector<vector<T>> &sets, const vector<T> &s)
{
	for (auto &_s: sets) {
		if (includes(all(s), all(_s)))
			return true;
	}

	return false;
}


long long total_counter = 0;


void print_selector_results(const vector<int> &v, const vector<vector<vector<int>>> &res)
{
	for (int bits = 0; bits < (int)res.size(); ++bits) {
		if (res[bits].empty())
			continue;
		
		total_counter += res[bits].size();

		clog << "# ";
		for (int b: v)
			clog << b << " ";
		clog << bits << "\n";

		for (auto &r: res[bits]) {
			for (int x: r)
				clog << x << " ";
			clog << "\n";
		}
	}
}


void enumerate_selectors_gates(const int k, const vector<vector<char>> &save, vector<int> &v)
{
	if ((int)v.size() < k) {
		int last = 0;

		if (!v.empty())
			last = v.back() + 1;

		for (int i = last; i < output_vars_cnt; ++i) {
			v.push_back(i);
			enumerate_selectors_gates(k, save, v);
			v.pop_back();
		}

		return;
	}

	vector<vector<Bit>> keys;
	for (int bits = 0; bits < (1 << k); ++bits) {
		vector<Bit> key;
		for (int i = 0; i < k; ++i)
			key.push_back({v[i], (bits >> (k - 1 - i)) & 1});
		
		keys.push_back(key);
	}

	vector<vector<vector<vector<int>>>> res(and_equations_cnt, vector<vector<vector<int>>>(1 << k));

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < and_equations_cnt; ++i) {
		auto e = equations[i];
		int y = e.y, z = e.z;

		vector<int> excl(1 << k, 0);

		for (int j = 0; j < N; ++j) {
			int bits = 0;
			for (int num: v) {
				bits <<= 1;
				bits |= save[j][output_vars[num]];
			}

			int val = (save[j][y] << 1) ^ save[j][z];
			val = 1 << val;

			excl[bits] |= val;
		}

		for (int bits = 0; bits < (1 << k); ++bits) {
			for (int val = 0; val < 4; ++val) {
				if (((excl[bits] >> val) & 1) == 0 && !find_subset(selectors[i][val], keys[bits]))
					res[i][bits].push_back(gate_constraints[i][val]);
			}
		}
	}

	vector<vector<vector<int>>> _res(1 << k);
	for (auto &rs: res) {
		for (int bits = 0; bits < (1 << k); ++bits) {
			for (auto &r: rs[bits])
				_res[bits].push_back(r);
		}
	}

	print_selector_results(v, _res);
}


void enumerate_selectors_all(const int k, const vector<vector<char>> &save, vector<int> &v, const int n)
{
	if ((int)v.size() < k) {
		int last = 0;

		if (!v.empty())
			last = v.back() + 1;

		for (int i = last; i < output_vars_cnt; ++i) {
			v.push_back(i);
			enumerate_selectors_all(k, save, v, n);
			v.pop_back();
		}

		return;
	}

	vector<vector<Bit>> keys;
	for (int bits = 0; bits < (1 << k); ++bits) {
		vector<Bit> key;
		for (int i = 0; i < k; ++i)
			key.push_back({v[i], (bits >> (k - 1 - i)) & 1});
		
		keys.push_back(key);
	}

	long long bound = bs_hint[0].back();

	vector<vector<vector<int>>> res(1 << k);

	#pragma omp parallel for schedule(dynamic, 64)
	for (long long i = 0; i < bound; ++i) {
		vector<int> lin(n);
		long long num = i;
		
		for (int j = 0; j < n; ++j) {
			int L = 0, R = bs_hint[j].size();

			while (L + 1 != R) {
				int M = (L + R) >> 1;
			
				if (bs_hint[j][M] <= num)
					L = M;
				else
					R = M;
			}

			num -= bs_hint[j][L];
			num += bs_hint[j + 1][L];
			lin[j] = L + j;
		}

		for (int &x: lin)
			x = all_vars[x];

		vector<int> excl(1 << k, 0);

		for (int j = 0; j < N; ++j) {
			int bits = 0;
			for (int num: v) {
				bits <<= 1;
				bits |= save[j][output_vars[num]];
			}

			int val = 1;
			for (int x: lin)
				val ^= save[j][x];

			val = 1 << val;

			excl[bits] |= val;
		}

		for (int bits = 0; bits < (1 << k); ++bits) {
			if ((excl[bits] & 1) == 0) {
				auto it = selectors_map.find(i);
				if (it == selectors_map.end() ||
					!find_subset((it -> second)[0], keys[bits]))
				{
					#pragma omp critical
					{
						res[bits].push_back(lin);
					}
				}
			}

			if ((excl[bits] >> 1) == 0) {
				auto it = selectors_map.find(i);
				if (it == selectors_map.end() ||
					!find_subset((it -> second)[1], keys[bits]))
				{
					lin[0] ^= 1;
					#pragma omp critical
					{
						res[bits].push_back(lin);
					}
					lin[0] ^= 1;
				}
			}
		}
	}

	print_selector_results(v, res);
}


void predict_gates(const int k)
{
	init_selectors_vector_gates();

	if (k < 0) {
		cerr << "error: void predict_gates(): k must not be a negative\n";
		return;
	}

	vector<vector<char>> save(N, vector<char> (2 * (vars_cnt + 1)));

	clog << "sample generating ... ";
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < N; ++i)
		gen_random_sample(save[i]);
	clog << "ok" << endl;

	vector<int> bits;
	auto start = chrono::system_clock::now();
	enumerate_selectors_gates(k, save, bits);
	auto delta = chrono::system_clock::now() - start;
	clog << setprecision(3) << fixed << delta.count() / 1e9 << " " << total_counter << endl;
}


void init_bs_hint(const int n)
{
	bs_hint.assign(n + 1, vector<long long> (all_vars.size() - n + 2, 0));

	bs_hint[n].back() = 1LL;

	for (int i = n - 1; i >= 0; --i) {
		for (int j = 1; j <= (int)all_vars.size() - n + 1; ++j)
			bs_hint[i][j] = bs_hint[i][j - 1] + bs_hint[i + 1].back() - bs_hint[i + 1][j - 1];
	}
}


void predict_all(const int k, const int n)
{
	init_bs_hint(n);

	init_selectors_map_all(n);

	if (k < 0) {
		cerr << "error: void predict_all(): k must not be a negative\n";
		return;
	}

	vector<vector<char>> save(N, vector<char> (2 * (vars_cnt + 1)));

	clog << "sample generating ... ";
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < N; ++i)
		gen_random_sample(save[i]);
	clog << "ok" << endl;

	vector<int> bits;
	auto start = chrono::system_clock::now();
	enumerate_selectors_all(k, save, bits, n);
	auto delta = chrono::system_clock::now() - start;
	clog << setprecision(3) << fixed << delta.count() / 1e9 << " " << total_counter << endl;
}


/*****************
 * main function *
 *****************/
int main(int argc, char *argv[])
{

	string in_filename;
	string core_vars_filename;
	string linear_constraints_filename;
	string lin_table_filename;
	int k, n;

	init(argc, argv, in_filename, core_vars_filename,
		linear_constraints_filename, lin_table_filename,
		k, n);

	read_core_vars(core_vars_filename);
	read_aig(in_filename);

	read_linear_constraints(pattern_linear_constraints, linear_constraints_filename);

	read_lin_table(lin_table_filename, lin_table);

	all_vars = vector<int>(all(all_vars_set));

	predict_all(k, n);
	// predict_gates(k);

}
