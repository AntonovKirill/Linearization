#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

using namespace std;


const int GREED = 1;
const int PROB = 2;
const int OPTIMAL = 3;


int N = 0;

int cores = thread::hardware_concurrency();

mt19937 generator(chrono::system_clock::now().time_since_epoch().count());

int points_cnt = 0;

mutex mtx, log_mtx;

ofstream llog("logs", ofstream::app);

/// genetic algorithm parameters
int n;
int L = 0;
int G = 0;
int H = 0;

/// УРАВНЕНИЕ and
/// x ^ y & z = 0 ( x = y & z )
/// ПОД x, y, z ПОНИМАЮТСЯ ПЕРЕМЕННЫЕ
/// С НОМЕРАМИ x, y, z СООТВЕТСТВЕННО
struct equation {
	int x;
	int y;
	int z;
};


vector <pair <vector <int>, int> > pattern_linear_equations_set;


/// МНОЖЕСТВО ВСЕХ УРАВНЕНИЙ
vector <equation> equations;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector <int> guessing_vars;
vector <int> key_vars;
vector <int> plaintext_vars;
vector <int> output_vars;
vector <int> output_vars_inv;


/// МАССИВЫ ЗНАЧЕНИЙ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector <vector <char> > guessing_interp;
vector <vector <char> > key_interp;
vector <vector <char> > plaintext_interp;
vector <vector <char> > output_interp;


/// КОЛИЧЕСТВО УРАВНЕНИЙ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int equations_cnt;


/// КОЛИЧЕСТВО ВСЕХ ПЕРЕМЕННЫХ И
/// ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int vars_cnt = 0;
int guessing_vars_cnt = 0;
int input_vars_cnt = 0;
int key_vars_cnt = 0;
int plaintext_vars_cnt = 0;
int output_vars_cnt = 0;

/// МОЖНО МЕНЯТЬ НА unordered_map
map <vector <char>, double> complexity_set;
map <vector <char>, double> probability_set;


/// ГЛУБИНА ПЕРЕМЕННОЙ
vector <int> vars_depth;

/// СЛОИ ПЕРЕМЕННЫХ
vector <vector <int> > layers;

/// ЧИСЛО СЛОЁВ
int layers_cnt = 0;


void init(int argc, char* argv[], string &in, string &out, string &start_point_filename) {
	for (int i = 1; i < argc; ++i) {
		string param;
		int j = 0;

		while (argv[i][j] != '=' && argv[i][j] != 0) {
			param.insert(param.end(), argv[i][j]);
			++j;
		}

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help\t\t\tВывод этой справки и выход.\n";
			cout << "  -N, --sample-size <size>\tРазмер выборки.\n";
			cout << "  -t, --threads <number>\tЧисло потоков. По умолчанию равно thread::hardware_concurrency().\n";
			cout << "  -i, --input <file>\t\tAIG кодировка КНФ.\n";
			cout << "  -o, --output <file>\t\tВыходной файл.\n";
			cout << "  -k, --start-point <file>\tФайл с описанием начальной точки.\n";
			cout << "  -L <number>\t\t\tКоличество лучших представителей популяции, переходящих в следующую.\n";
			cout << "  -H <number>\t\t\tКоличество мутаций.\n";
			cout << "  -G <number>\t\t\tКоличество скрещиваний.\n";
			cout << "  -k, --key-size <size>\t\tЧисло бит ключа.\n";
			exit(0);
		}
		else if (param == "--sample-size" || param == "-N") { /// СДЕЛАТЬ ПРОВЕРКУ НА НЕОТРИЦАТЕЛЬНОСТЬ
			if (argv[i][j] == 0) {
				N = atoi(argv[++i]);
			}
			else {
				N = atoi(argv[i] + j + 1);
			}
		}
		else if (param == "--threads" || param == "-t") { /// СДЕЛАТЬ ПРОВЕРКУ НА НЕОТРИЦАТЕЛЬНОСТЬ
			if (argv[i][j] == 0) {
				cores = atoi(argv[++i]);
			}
			else {
				cores = atoi(argv[i] + j + 1);
			}
		}
		else if (param == "--input" || param == "-i") {
			if (argv[i][j] == 0) {
				in = (string) (argv[++i]);
			}
			else {
				in = (string) (argv[i] + j + 1);
			}
		}
		else if (param == "--output" || param == "-o") {
			if (argv[i][j] == 0) {
				out = (string) (argv[++i]);
			}
			else {
				out = (string) (argv[i] + j + 1);
			}
		}
		else if (param == "--start-point" || param == "-p") {
			if (argv[i][j] == 0) {
				start_point_filename = (string) argv[++i];
			}
			else {
				start_point_filename = (string) (argv[i] + j + 1);
			}
		}
		else if (param == "--key-size" || param == "-k") { /// СДЕЛАТЬ ПРОВЕРКУ НА НЕОТРИЦАТЕЛЬНОСТЬ
			if (argv[i][j] == 0) {
				key_vars_cnt = atoi(argv[++i]);
			}
			else {
				key_vars_cnt = atoi(argv[i] + j + 1);
			}
		}
		else if (param == "-L") { /// СДЕЛАТЬ ПРОВЕРКУ НА НЕОТРИЦАТЕЛЬНОСТЬ
			if (argv[i][j] == 0) {
				L = atoi(argv[++i]);
			}
			else {
				L = atoi(argv[i] + j + 1);
			}
		}
		else if (param == "-G") { /// СДЕЛАТЬ ПРОВЕРКУ НА НЕОТРИЦАТЕЛЬНОСТЬ
			if (argv[i][j] == 0) {
				G = atoi(argv[++i]);
			}
			else {
				G = atoi(argv[i] + j + 1);
			}
		}
		else if (param == "-H") { /// СДЕЛАТЬ ПРОВЕРКУ НА НЕОТРИЦАТЕЛЬНОСТЬ
			if (argv[i][j] == 0) {
				H = atoi(argv[++i]);
			}
			else {
				H = atoi(argv[i] + j + 1);
			}
		}
		else {
			cout << "unknown parameter: " << param << "\n";
			exit(0);
		}
	}

	if (cores == 0) {
		cores = 1;
	}

	n = L + G + H;
}


int rand_int() {
	return generator();
}


bool rand_bool() {
	return generator() & 1;
}


void resize_all(vector <vector <char> > &vec, int cnt) {
	#pragma omp parallel for
	for (int i = 0; i < (int) vec.size(); ++i) {
		vec[i].resize(cnt);
	}
}


int bin_set_size(const vector <char> &a) {
	int res = 0;
	for (char i: a)
		res += i;

	return res;
}


/// ЗАГОЛОВОК ВКЛЮЧАЕТ ЧИСЛО ВСЕХ И
/// ЧИСЛО ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
void read_header(ifstream &fin) {
	cout << "read header ... ";
	cout.flush();
	string header;
	fin >> header;

	if (header == "aag") {
		fin >> vars_cnt >> input_vars_cnt >> output_vars_cnt >> output_vars_cnt >> equations_cnt;
	}
	else {
		cout << "wrong input format" << endl;
		exit(0);
	}

	cout << "ok" << endl;
}


void read_input(ifstream &fin) {
	cout << "read input ... ";
	cout.flush();
	plaintext_vars_cnt = input_vars_cnt - key_vars_cnt;
	if (key_vars_cnt == 0) {
		guessing_vars_cnt = plaintext_vars_cnt;
	}
	else {
		guessing_vars_cnt = key_vars_cnt;
	}

	key_vars.resize(key_vars_cnt);
	plaintext_vars.resize(plaintext_vars_cnt);
	guessing_vars.resize(guessing_vars_cnt);

	guessing_interp.resize(N);
	key_interp.resize(N);
	plaintext_interp.resize(N);

	resize_all(guessing_interp, guessing_vars_cnt);
	resize_all(key_interp, key_vars_cnt);
	resize_all(plaintext_interp, plaintext_vars_cnt);

	for (int i = 0; i < key_vars_cnt; ++i) {
		int x;
		fin >> x;
		x >>= 1;
		key_vars[i] = x;
	}
	for (int i = 0; i < plaintext_vars_cnt; ++i) {
		int x;
		fin >> x;
		x >>= 1;
		plaintext_vars[i] = x;
	}
	for (int i = 0; i < guessing_vars_cnt; ++i) {
		if (key_vars_cnt == 0) {
			guessing_vars[i] = plaintext_vars[i];
		}
		else {
			guessing_vars[i] = key_vars[i];
		}
	}

	cout << "ok" << endl;
}


void read_output(ifstream &fin) {
	cout << "read output ... ";
	cout.flush();
	output_vars.resize(output_vars_cnt);
	output_vars_inv.resize(output_vars_cnt);

	output_interp.resize(N);
	#pragma omp parallel for
	for (int i = 0; i < N; ++i) {
		output_interp[i].resize(output_vars_cnt);
	}

	for (int i = 0; i < output_vars_cnt; ++i) {
		int x;
		fin >> x;
		char inv = (char)(x & 1);
		x >>= 1;
		output_vars_inv[i] = inv;
		output_vars[i] = x;
	}

	cout << "ok" << endl;
}


void read_equations(ifstream &fin) {
	cout << "read equations ... ";
	cout.flush();
	for (int i = 0; i < equations_cnt; ++i) {
		int x, y, z;
		fin >> x >> y >> z;
		equations.push_back({x, min(y, z), max(y, z)});
	}
	cout << "ok" << endl;
}


/// ЧТЕНИЕ AIG-ФАЙЛА С ОПИСАНИЕМ КОДИРОВКИ
void read_all(string in) {
cout << "read all ..." << endl;
	ifstream fin(in.data());

	read_header(fin);
	read_input(fin);
	read_output(fin);
	read_equations(fin);

	fin.close();
}


/// ЧТЕНИЕ ФАЙЛА С ОПИСАНИЕМ НАЧАЛЬНОЙ ТОЧКИ ПОИСКА
void read_start_point_file(string &start_point_filename, vector <char> &start_point) {
	if (!start_point_filename.empty()) {
		cout << "read point (" << start_point_filename << ") ... ";
		cout.flush();

		start_point.resize(guessing_vars_cnt, 0);
		ifstream pin(start_point_filename.data());
		int u;
		while (pin >> u) {
			if (u <= guessing_vars_cnt) {
				start_point[u - 1] = 1;
			}
			else {
				guessing_vars.push_back(u);
				start_point.push_back(1);
			}
		}

		guessing_vars_cnt = guessing_vars.size();
		resize_all(guessing_interp, guessing_vars_cnt);

		pin.close();
		cout << "ok" << endl;
	}
	else {
		start_point.resize(guessing_vars_cnt);
		for (int i = 0; i < guessing_vars_cnt; ++i)
			start_point[i] = 1;
	}
}


template <class T>
void order(vector <T> &v) {
	sort(all(v));
	v.resize(unique(all(v)) - v.begin());
	v.shrink_to_fit();
}


void join_sets(int x, int y, vector <int> &dsu, vector <vector <int> > &childs) {
	x = dsu[x];
	y = dsu[y];
	if (x == y)
		return;

	if (childs[x].size() < childs[y].size())
		swap(x, y);

	for (int z: childs[y]) {
		dsu[z] = x;
		dsu[z ^ 1] = x ^ 1;
		childs[x].push_back(z);
		childs[x ^ 1].push_back(z ^ 1);
	}
	childs[y].clear();
	childs[y ^ 1].clear();
}


int propagation(const equation e, vector <char> &vars_values, vector <char> &is_def) {
	int x = e.x;
	int y = e.y;
	int z = e.z;

	char val_x = vars_values[x / 2] ^ (x & 1);
	char val_y = vars_values[y / 2] ^ (y & 1);
	char val_z = vars_values[z / 2] ^ (z & 1);

	x = x / 2;
	y = y / 2;
	z = z / 2;

	if (is_def[x] && is_def[y] && is_def[z])
		return 0;

	if (!is_def[x]) {
		if ((is_def[y] && !val_y) || (is_def[z] && !val_z)) {
			is_def[x] = 1;
			vars_values[x] = vars_values[x] ^ val_x;
			return 1;
		}
		if (is_def[y] && val_y && is_def[z] && val_z) {
			is_def[x] = 1;
			vars_values[x] = vars_values[x] ^ val_x ^ 1;
			return 1;
		}
		return 0;
	}
	// is_def[x] == true
	if (!val_x) {
		if (is_def[y] && val_y) {
			is_def[z] = 1;
			vars_values[z] = vars_values[z] ^ val_z;
			return 1;
		}
		if (is_def[z] && val_z) {
			is_def[y] = 1;
			vars_values[y] = vars_values[y] ^ val_y;
			return 1;
		}
		return 0;
	}
	// val_x == 1
	if (!is_def[y]) {
		is_def[y] = 1;
		vars_values[y] = vars_values[y] ^ val_y ^ 1;
	}
	if (!is_def[z]) {
		is_def[z] = 1;
		vars_values[z] = vars_values[z] ^ val_z ^ 1;
	}
	return 1;
}


/// ВЫВОД ИЗ КВАДРАТИЧНОГО УРАВНЕНИЯ
/// ВОЗВРАЩАЕТ 1, ЕСЛИ ХОТЬ ЧТО-ТО ВЫВЕЛОСЬ
/// ИНАЧЕ 0
int propagation(equation e, vector <char> &vars_values, vector <char> &is_def, vector <int> &dsu, vector <vector <int> > &childs) {
	int x = dsu[e.x];
	int y = dsu[e.y];
	int z = dsu[e.z];
	int x2 = x / 2;
	int y2 = y / 2;
	int z2 = z / 2;

	char val_x = vars_values[x2] ^ (char) (x & 1);
	char val_y = vars_values[y2] ^ (char) (y & 1);
	char val_z = vars_values[z2] ^ (char) (z & 1);

	if (is_def[x2] & is_def[y2] & is_def[z2]) {
		return 0;
	}

	if (z == y) {
		if (is_def[z2] && (is_def[x2] ^ 1)) {
			is_def[x2] = 1;
			vars_values[x2] ^= val_z ^ val_x;
			return 1;
		}
		if ((is_def[z2] ^ 1) && is_def[x2]) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ val_x;
			return 1;
		}
		if (x == z) {
			// do nothing
			// TODO: add 2-literal learnt x & -y = 0
			return 0;
		}
		join_sets(x, z, dsu, childs);
		return 1;
	}

	if (z == (y ^ 1)) {
		if ((is_def[x2] ^ 1)) {
			is_def[x2] = 1;
			vars_values[x2] ^= val_x ^ 0;
			return 1;
		}
		return 0;
	}

	if (z == x) {
		if (is_def[z2] && val_z && (is_def[y2] ^ 1)) {
			is_def[y2] = 1;
			vars_values[y2] ^= val_y ^ 1;
			return 1;
		}
		if (is_def[y2] && (val_y ^ 1) && (is_def[z2] ^ 1)) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ 0;
			return 1;
		}
		if (z == (y ^ 1)) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ 0;
			return 1;
		}
		return 0;
	}

	if (z == (x ^ 1)) {
		if ((is_def[z2] ^ 1) || (is_def[y2] ^ 1)) {
			is_def[z2] = 1;
			is_def[y2] = 1;
			vars_values[z2] ^= val_z ^ 1;
			vars_values[y2] ^= val_y ^ 0;
			return 1;
		}
		return 0;
	}

	if (y == x) {
		if (is_def[z2] && (val_z ^ 1) && (is_def[y2] ^ 1)) {
			is_def[y2] = 1;
			vars_values[y2] ^= val_y ^ 0;
			return 1;
		}
		if (is_def[y2] && val_y && (is_def[z2] ^ 1)) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ 1;
			return 1;
		}
		if (z == (y ^ 1)) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ 1;
			return 1;
		}
		return 0;
	}

	if (y == (x ^ 1)) {
		if ((is_def[z2] ^ 1) || (is_def[y2] ^ 1)) {
			is_def[z2] = 1;
			is_def[y2] = 1;
			vars_values[z2] ^= val_z ^ 0;
			vars_values[y2] ^= val_y ^ 1;
			return 1;
		}
		return 0;
	}

	x = x2;
	y = y2;
	z = z2;


	if (is_def[x] ^ 1) {
		if ((is_def[y] & (val_y ^ 1)) | (is_def[z] & (val_z ^ 1))) {
			is_def[x] = 1;
			vars_values[x] ^= val_x ^ 0;
			return 1;
		}
		else if (is_def[y] & val_y & is_def[z] & val_z) {
			is_def[x] = 1;
			vars_values[x] ^= val_x ^ 1;
			return 1;
		}
		else if (is_def[y] & val_y) {
			join_sets(e.x, e.z, dsu, childs);
			return 1;
		}
		else if (is_def[z] & val_z) {
			join_sets(e.x, e.y, dsu, childs);
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if (val_x ^ 1) {
			if (is_def[y] & val_y) {
				is_def[z] = 1;
				vars_values[z] ^= val_z ^ 0;
				return 1;
			}
			else if (is_def[z] & val_z) {
				is_def[y] = 1;
				vars_values[y] ^= val_y ^ 0;
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			if (is_def[y] ^ 1) {
				is_def[y] = 1;
				vars_values[y] ^= val_y ^ 1;
			}
			if (is_def[z] ^ 1) {
				is_def[z] = 1;
				vars_values[z] ^= val_z ^ 1;
			}
			return 1;
		}
	}
}


int propagation(equation e,
	vector <char> &vars_values, vector <char> &is_def,
	vector <int> &dsu, vector <vector <int> > &childs,
	map <pair <int, int>, int> &gate_pairs)
{
	int x = dsu[e.x];
	int y = dsu[e.y];
	int z = dsu[e.z];

	if (is_def[x / 2] && is_def[y / 2] && is_def[z / 2])
		return 0;

	if (y > z)
		swap(y, z);
	pair <int, int> key = {y, z};

	auto it = gate_pairs.find(key);
	int res = 0;
	if (it != gate_pairs.end()) {
		if (dsu[it -> second] != x) {
			join_sets(it -> second, x, dsu, childs);
			it -> second = dsu[x];
			res = 1;
		}
	}

	return max(res, propagation(e, vars_values, is_def, dsu, childs));
}

/*
void trace_history(pair <vector <int>, int> e, map <pair <vector <int>, int>, pair <pair <vector <int>, int>, pair <vector <int>, int> > > &history, set <pair <vector <int>, int> > &used, ostream &os, int indent = 0, bool unroll = true) {
	used.insert(e);
	for (int i = 0 ; i < indent; ++i)
		os << " ";
	os << "{";
	for (auto x: e.first)
		os << x << " ";
	os << "} = " << e.second << "\n";

	if (!unroll)
		return;
	auto it = history.find(e);

	if (it == history.end())
		return;
	if (used.find((it -> second).first) == used.end())
		unroll = true;
	else
		unroll = false;
	trace_history((it -> second).first, history, used, os, indent + 1, unroll);

	if (used.find((it -> second).second) == used.end())
		unroll = true;
	else
		unroll = false;
	trace_history((it -> second).second, history, used, os, indent + 1, unroll);
}
*/

void equations_xor(const vector <int> &e1, const vector <int> &e2, vector <int> &res) {
	res.clear();
	auto it1 = e1.begin();
	auto it2 = e2.begin();
	while (it1 != e1.end() && it2 != e2.end()) {
		if (*it1 == *it2) {
			++it1;
			++it2;
		}
		else if (*it1 < *it2) {
			res.push_back(*it1);
			++it1;
		}
		else { // *it1 > *it2
			res.push_back(*it2);
			++it2;
		}
	}
	while (it1 != e1.end()) {
		res.push_back(*it1);
		++it1;
	}
	while (it2 != e2.end()) {
		res.push_back(*it2);
		++it2;
	}
}

void analyse_linear_system(set <pair <vector <int>, int> > &linear_system,
	vector <pair <vector <int>, int> > &relations)
{
	// map <pair <vector <int>, int>, pair <pair <vector <int>, int>, pair <vector <int>, int> > > history;
	vector <vector <int> > equations_by_var(vars_cnt + 1);
	int counter = 0;
	/* stage 1 */
	for (auto it = linear_system.begin(); it != linear_system.end(); ++it) {
		/// ЕСЛИ it СОДЕРЖИТ ЛИНЕЙНОЕ СООТНОШЕНИЕ
		/// НАД 1 ИЛИ 2 ПЕРЕМЕНЫМИ, НАДО ЕГО СОХРАНИТЬ
		if ((it -> first).size() <= 2)
			relations.push_back(*it);
		/// СОХРАНИТЬ СИСТЕМУ В vector <pair <vector <int>, int> > ls
		/// ДЛЯ КАЖДОГО x \in ls СОХРАНИТЬ ВСЕ УРАВНЕНИЯ, СОДЕРЖАЩИЕ (x.first)[0]
		/// ИСПОЛЬЗОВАТЬ vector <vector <int> > РАЗМЕРА vars_cnt + 1
		for (int i = 1; i < (int) (it -> first).size(); ++i)
			equations_by_var[(it -> first)[i]].push_back(counter);
		++counter;
		auto it1 = it;
		++it1;
		while (it1 != linear_system.end() && *((it1 -> first).begin()) == *((it -> first).begin())) {
			int b = (it -> second) ^ (it1 -> second);
			vector <int> res;
			equations_xor(it -> first, it1 -> first, res);
			/*
			history[{res, b}] = {*it, *it1};
			*/
			if (res.empty() && b != 0) {
				mtx.lock();
					/*
					set <pair <vector <int>, int> > used;
					ofstream hist("history");
					trace_history({res, b}, history, used, hist);
					hist.close();
					*/
					cerr << "wrong equation has been deduced: 0 = 1\n";
					exit(0);
				mtx.unlock();
			}
			if (!res.empty())
				linear_system.insert({res, b});
			it1 = linear_system.erase(it1);
		}
	}
	/**/
	vector <pair <vector <int>, int> > ls(all(linear_system));
	/* stage 2 */
	for (int i = ls.size() - 1; i >= 0; --i) {
		/// ЕСЛИ ls[i] СОДЕРЖИТ ЛИНЕЙНОЕ СООТНОШЕНИЕ
		/// НАД 1 ИЛИ 2 ПЕРЕМЕНЫМИ, НАДО ЕГО СОХРАНИТЬ
		if (ls[i].first.size() <= 2)
			relations.push_back(ls[i]);

		vector <int> res;
		for (int j : equations_by_var[ls[i].first[0]]) {
			equations_xor(ls[i].first, ls[j].first, res);
			ls[j].first = res;
			ls[j].second ^= ls[i].second;
		}
	}
	/**/
/*
	log_mtx.lock();
	llog << "end\n";
	for (auto it = linear_system.begin(); it != linear_system.end(); ++it) {
		for (auto x: it -> first)
			llog << x << " ";
		llog << "= " << (it -> second) << "\n";
	}
	llog << "\n";
	log_mtx.unlock();
*/
}

void find_output(vector <char> &vars_values, vector <char> &is_def) {
	int cnt = 0;

	while (true) {
		cnt = 0;
		for (auto &e: equations)
			cnt += propagation(e, vars_values, is_def);
		if (cnt == 0)
			break;
	}
}

void solve(char &a, vector <char> &vars_values, vector <char> &is_def, pair <int, int> &unsolved) {
	int cnt = 0;
	vector <int> dsu(2 * (vars_cnt + 1));
	vector <vector <int> > childs(2 * (vars_cnt + 1));
	map <pair <int, int>, int> gate_pairs;
	for (int i = 0; i < (int) dsu.size(); ++i) {
		dsu[i] = i;
		childs[i].push_back(i);
	}

	// TODO: use vector<vector<...>>(vars_cnt + 1) instead of set<...>
	// TODO: use counting sort and unique() instead of set built-in sor
	set <pair <vector <int>, int> > linear_system(all(pattern_linear_equations_set)), linear_system_next;

	while (true) {
		while (true) {
			cnt = 0;
			for (auto &e: equations) {
				// cnt += propagation(e, vars_values, is_def, dsu, childs);
				cnt += propagation(e, vars_values, is_def, dsu, childs, gate_pairs);
			}
			if (cnt == 0)
				break;
		}

		linear_system_next.clear();
		for (auto it = linear_system.begin(); it != linear_system.end(); ++it) {
			map <int, int> equation_map;
			vector <int> equation_vector;
			int b = it -> second;
			for (auto x: it -> first) {
				x = dsu[2 * x];
				if (is_def[x / 2])
					b ^= vars_values[x / 2];
				else
					++equation_map[x / 2];
				b ^= x;
			}
			for (auto &p: equation_map) { // set {p.first} is ordered by increasing
				if (p.second & 1)
					equation_vector.push_back(p.first);
			}
			if (!equation_vector.empty())
				linear_system_next.insert({equation_vector, b & 1});
		}
		// order(linear_system_next);

		vector <pair <vector <int>, int> > relations;
		analyse_linear_system(linear_system_next, relations);
		swap(linear_system, linear_system_next);

		if (relations.empty())
			break;
		for (auto &p: relations) {
			if (p.first.size() == 1) {
				is_def[p.first[0]] = 1;
				vars_values[p.first[0]] = p.second;
			}
			else if (p.first.size() == 2) {
				join_sets(2 * p.first[0], 2 * p.first[1] ^ p.second, dsu, childs);
			}
		}
	}

	a = 1;
	for (auto &e: equations) {
		int x = dsu[e.x];
		char negx = x & 1;
		int y = dsu[e.y];
		int z = dsu[e.z];

		if ((is_def[y / 2] ^ 1) & (is_def[z / 2] ^ 1) & ((is_def[x / 2] ^ 1) | (vars_values[x / 2] ^ negx ^ 1)) & (y != z) & (y != (z ^ 1))) {
			a = 0;
			break;
		}
	}

	unsolved = {0, 0};
	for (int x: guessing_vars) {
		if (!is_def[dsu[2 * x] / 2])
			++unsolved.first;
	}
	for (int x = 1; x <= vars_cnt; ++x) {
		if (dsu[2 * x] == 2 * x && !is_def[x])
			++unsolved.second;
	}
	unsolved.second -= linear_system.size();
}


void save_input_interp(int i, vector <char> &vars_values) {
	for (int j = 0; j < key_vars_cnt; ++j)
		key_interp[i][j] = vars_values[key_vars[j]];
	for (int j = 0; j < plaintext_vars_cnt; ++j)
		plaintext_interp[i][j] = vars_values[plaintext_vars[j]];
}


void save_output_interp(int i, vector <char> &vars_values) {
	for (int j = 0; j < guessing_vars_cnt; ++j)
		guessing_interp[i][j] = vars_values[guessing_vars[j]];
	for (int j = 0; j < output_vars_cnt; ++j)
		output_interp[i][j] = vars_values[output_vars[j]];
}


void solve_all(int i, vector <vector <char> > &vars_values, vector <vector <char> > &is_def) {
	#pragma omp parallel for
	for (int j = 0; j < cores; ++j) {
		if (i + j >= N) {
			continue;
		}

		fill(is_def[j].begin(), is_def[j].end(), 0);

		for (int k = 0; k < key_vars_cnt; ++k) {
			is_def[j][key_vars[k]] = 1;
		}

		for (int k = 0; k < plaintext_vars_cnt; ++k) {
			is_def[j][plaintext_vars[k]] = 1;
		}
	}

	#pragma omp parallel for
	for (int j = 0; j < cores; ++j) {
		if (i + j >= N) {
			continue;
		}
		find_output(vars_values[j], is_def[j]);
	}
}


void gen_random_interp() {
	cout << "gen random sample ... ";
	cout.flush();
	vector <vector <char> > vars_values(cores);
	vector <vector <char> > is_def(cores);

	resize_all(vars_values, vars_cnt + 1);
	resize_all(is_def, vars_cnt + 1);

	for (int i = 0; i < N; i += cores) {
		#pragma omp parallel for
		for (int j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}

			for (int k = 0; k < key_vars_cnt; ++k) {
				vars_values[j][key_vars[k]] = rand_bool();
			}

			for (int k = 0; k < plaintext_vars_cnt; ++k) {
				vars_values[j][plaintext_vars[k]] = rand_bool();
			}

			save_input_interp(i + j, vars_values[j]);
		}
		solve_all(i, vars_values, is_def);

		#pragma omp parallel for
		for (int j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}

			save_output_interp(i + j, vars_values[j]);
		}
	}

	cout << "ok" << endl;
}


void clear_vars_values(int i, const vector <char> &vars_set, vector <char> &vars_values, vector <char> &is_def) {
	#pragma omp parallel for
	for (int j = 1; j <= vars_cnt; ++j) {
		is_def[j] = 0;
	}

	if (key_vars_cnt != 0) {
		for (int j = 0; j < plaintext_vars_cnt; ++j) {
			is_def[plaintext_vars[j]] = 1;
			vars_values[plaintext_vars[j]] = plaintext_interp[i][j];
		}
	}

	for (int j = 0; j < guessing_vars_cnt; ++j) {
		is_def[guessing_vars[j]] = vars_set[j];
		vars_values[guessing_vars[j]] = guessing_interp[i][j];
	}

	for (int j = 0; j < output_vars_cnt; ++j) {
		is_def[output_vars[j]] = 1;
		vars_values[output_vars[j]] = output_interp[i][j];
	}
}


/// ОЦЕНОЧНАЯ ФУНКЦИЯ
double complexity(const vector <char> &vars_set, ofstream &fout) {
	auto it = complexity_set.find(vars_set);
	if (it != complexity_set.end()) {
		return it -> second;
	}

	int cnt = 0;
	vector <char> answer(cores);
	vector <pair <int, int> > unsolved_sizes(cores);
	map <pair <int, int>, int> ranks;
	vector <vector <char> > vars_values(cores);
	vector <vector <char> > is_def(cores);

	resize_all(vars_values, vars_cnt + 1);
	resize_all(is_def, vars_cnt + 1);

	for (int i = 0; i < N; i += cores) {
		#pragma omp parallel for
		for (int j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}

			clear_vars_values(i + j, vars_set, vars_values[j], is_def[j]);
		}

		#pragma omp parallel for
		for (int j = 0; j < cores; ++j) {
			if (i + j >= N)
				continue;
			// solve(answer[j], vars_values[j], is_def[j]);
			solve(answer[j], vars_values[j], is_def[j], unsolved_sizes[j]);
		}

		for (int j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}
			++ranks[unsolved_sizes[j]];
			if (answer[j])
				++cnt;
		}
	}

//	if (cnt < 5) {
//		cnt = 0;
//	}

	double prob = (double) cnt / N;
	double res;
	if (prob == 0)
		res = DBL_MAX;
	else
		res = 3.0 * pow(2.0, (double) bin_set_size(vars_set)) / prob;

	probability_set[vars_set] = prob;
	complexity_set[vars_set] = res;
	++points_cnt;

	log_mtx.lock();
	llog << bin_set_size(vars_set) << ": ";
	for (int i = 0; i < guessing_vars_cnt; ++i) {
		if (vars_set[i])
			llog << guessing_vars[i] << " ";
	}
	llog << "\n";
	llog << "complexity: " << res << "\n";
	llog << "probability: " << prob << "\n";
	for (auto &p: ranks)
		llog << "(" << p.first.first << ", " << p.first.second << " : " << p.second << ") ";
	llog << "\n" << endl;
	log_mtx.unlock();

	return res;
}



vector <char> mutation(vector <char> &v) {
	int prob_inv = guessing_vars_cnt;
	vector <char> res = v;
	for (int i = 0; i < guessing_vars_cnt; ++i) {
		if (rand_int() % prob_inv == 0)
			res[i] ^= 1;
	}
	return res;
}


vector <char> crossover(vector <char> &a, vector <char> &b) {
	vector <char> res(guessing_vars_cnt);
	for (int i = 0; i < guessing_vars_cnt; ++i) {
		if (rand_bool()) {
			res[i] = a[i];
		}
		else {
			res[i] = b[i];
		}
	}

	return res;
}


void set_population_keys(vector <pair <pair <double, int>, vector <char> > > &p) {
	for (auto &ind: p)
		ind.first.second = rand_int();
}


///  ПОИСК ОПТИМАЛЬНОГО ЛИНЕАРИЗАЦИОННОГО МНОЖЕСТВА
void run_genetic(const vector <char> &start_point, ofstream &fout) {
	double cmplx = complexity(start_point, fout);
	int start_time = time(0);

	vector <pair <pair <double, int>, vector <char> > > current_population(n, {{cmplx, 0}, start_point});

	vector <char> best_set;
	while (true) {
		fout << "points have been viewed: " << points_cnt << "(" << time(0) - start_time << " sec)\n";

		set_population_keys(current_population);
		sort(current_population.begin(), current_population.end());

		if (best_set != current_population[0].second) {
			best_set = current_population[0].second;

			fout << "best current point:" << endl;
			fout << "complexity: " << current_population[0].first.first << endl;
			fout << "probability: " << probability_set[current_population[0].second] << "\n";
			fout << setw(4) << bin_set_size(current_population[0].second) << ":";
			for (int i = 0; i < guessing_vars_cnt; ++i) {
				if (current_population[0].second[i])
					fout << setw(4) << guessing_vars[i];
			}
			fout << "\n" << endl;
		}

		vector <pair <pair <double, int>, vector <char> > > next_population;
		for (int i = 0; i < L; ++i) {
			next_population.push_back(current_population[i]);
		}

		vector <double> dist;
		double prob_sum = 0;
		for (auto ind: current_population) {
			double p = 1.0 / ind.first.first;
			prob_sum += p;
			dist.push_back(p);
		}
		for (int i = 1; i < n; ++i) {
			dist[i] += dist[i - 1];
			dist[i - 1] /= prob_sum;
		}
		dist[n - 1] = 1;

		for (int i = 0; i < H; ++i) {
			double num = rand_int() * 1.0 / mt19937::max();

			for (int j = 0; j < n; ++j) {
				if (num < dist[j]) {
					auto mutant = mutation(current_population[j].second);
					cmplx = complexity(mutant, fout);
					next_population.push_back({{cmplx, 0}, mutant});
					break;
				}
			}
		}

		for (int i = 0; i < G; ++i) {
			double num;

			num = rand_int() * 1.0 / mt19937::max();
			vector <char> parent1, parent2;

			for (int j = 0; j < n; ++j) {
				if (num < dist[j]) {
					parent1 = current_population[j].second;
					break;
				}
			}

			num = rand_int() * 1.0 / mt19937::max();
			for (int j = 0; j < n; ++j) {
				if (num < dist[j]) {
					parent2 = current_population[j].second;
					break;
				}
			}

			auto child = crossover(parent1, parent2);
			cmplx = complexity(child, fout);
			next_population.push_back({{cmplx, 0}, child});
		}

		current_population = next_population;
	}
}


void genetic(const vector <char> &start_point, const string &out) {
	ofstream fout(out.data());
	fout << "search starts..." << endl;

	run_genetic(start_point, fout);

	fout.close();
}


/// ВЫЧИСЛЕНИЕ ГЛУБИНЫ ВСЕХ ПЕРЕМЕННЫХ
void calculate_depth() {
	cout << "calculate depth ... ";
	cout.flush();

	vars_depth.resize(vars_cnt + 1, 0);
	int max_depth = 0;
	while (true) {
		bool f = false;
		for (auto e: equations) {
			int x = e.x >> 1;
			int y = e.y >> 1;
			int z = e.z >> 1;

			int vd = vars_depth[x];
			vars_depth[x] = max(vars_depth[y], vars_depth[z]) + 1;
			max_depth = max(max_depth, vars_depth[x]);

			if (vd != vars_depth[x]) {
				f = true;
			}
		}

		if (!f) {
			break;
		}
	}

	layers_cnt = max_depth + 1;
	layers.resize(layers_cnt);
	for (int x = 1; x <= vars_cnt; ++x) {
		layers[vars_depth[x]].push_back(x);
	}

cout << "ok" << endl;
}


void read_linear_equations() {
	cout << "read linear ... ";
	cout.flush();
	ifstream fin("linear_equations");
	stringstream ss;
	string line;
	while (getline(fin, line)) {
		if (line.empty())
			continue;
		ss.clear();
		ss << line;
		int x;
		int b = 0;
		vector <int> equation;
		while (ss >> x) {
			equation.push_back(x / 2);
			b ^= x;
		}
		sort(all(equation));
		pattern_linear_equations_set.push_back({equation, b & 1});
	}
	order(pattern_linear_equations_set);
	fin.close();
	cout << "ok (" << pattern_linear_equations_set.size() << " equations)" << endl;
}


int main(int argc, char *argv[]) {

	string in;
	string out;
	string spf;

	init(argc, argv, in, out, spf);
	read_linear_equations();

	ios::sync_with_stdio(0);

	read_all(in);

	calculate_depth();

	vector <char> sp;
	read_start_point_file(spf, sp);

	gen_random_interp();

	genetic(sp, out);

	llog.close();

}
