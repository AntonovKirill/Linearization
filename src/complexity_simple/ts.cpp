#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <climits>
#include <cstdint>
#include <iomanip>
#include <cfloat>
#include <chrono>
#include <cstdio>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <cmath>
#include <mutex>
#include <queue>
#include <list>
#include <map>
#include <set>

#define uif8 uint_fast8_t
#define uif32 uint_fast32_t

using namespace std;


const uif32 GREED = 1;
const uif32 PROB = 2;
const uif32 OPTIMAL = 3;


uif32 N = 0;

uif32 cores = thread::hardware_concurrency();

mt19937 generator(chrono::system_clock::now().time_since_epoch().count());

uif32 points_cnt = 0;

mutex mtx;


/// УРАВНЕНИЕ and
/// x ^ y & z = 0 ( x = y & z )
/// ПОД x, y, z ПОНИМАЮТСЯ ПЕРЕМЕННЫЕ
/// С НОМЕРАМИ x, y, z СООТВЕТСТВЕННО
struct equation {
	uif32 x;
	uif32 y;
	uif32 z;
};


struct cmp_prob {
	bool operator() (const pair <double, vector <uif8> > &a, const pair <double, vector <uif8> > &b) {
		if (a.first > b.first) {
			return true;
		}
		if (a.first < b.first) {
			return false;
		}

		if (a.second.size() < b.second.size()) {
			return true;
		}
		if (a.second.size() > b.second.size()) {
			return false;
		}

		return a.second < b.second;
	}
};


/// МНОЖЕСТВО ВСЕХ УРАВНЕНИЙ
vector <equation> equations;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector <uif32> guessing_vars;
vector <uif32> key_vars;
vector <uif32> plaintext_vars;
vector <uif32> output_vars;
vector <uif32> output_vars_inv;


/// МАССИВЫ ЗНАЧЕНИЙ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector <vector <uif8> > guessing_interp;
vector <vector <uif8> > key_interp;
vector <vector <uif8> > plaintext_interp; 
vector <vector <uif8> > output_interp;


/// КОЛИЧЕСТВО УРАВНЕНИЙ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
uif32 equations_cnt;


/// КОЛИЧЕСТВО ВСЕХ ПЕРЕМЕННЫХ И
/// ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
uif32 vars_cnt = 0;
uif32 guessing_vars_cnt = 0;
uif32 input_vars_cnt = 0;
uif32 key_vars_cnt = 0;
uif32 plaintext_vars_cnt = 0;
uif32 output_vars_cnt = 0;

/// МОЖНО МЕНЯТЬ НА unordered_map
map <vector <uif8>, double> complexity_set;
map <vector <uif8>, double> probability_set;

/// МНОЖЕСТВА ДЛЯ АЛГОРИТМА tabu search
set <vector <uif8> > tabu_list_1;
set <pair <double, vector <uif8> > > tabu_list_2;
map <vector <uif8>, uif32> start_number_set;
set <pair <double, vector <uif8> >, cmp_prob> tabu_list_2_probability;

/// ГЛУБИНА ПЕРЕМЕННОЙ
vector <uif32> vars_depth;

/// СЛОИ ПЕРЕМЕННЫХ
vector <vector <uif32> > layers;

/// ЧИСЛО СЛОЁВ
uif32 layers_cnt = 0;


/// ЧТЕНИЕ ПАРАМЕТРОВ ИЗ ТЕРМИНАЛА
void init(int argc, char *argv[], string &in, string &out, string &start_point_filename, uif32 &mode) {
	for (uif32 i = 1; i < (uif32)argc; ++i) {
		string param;
		uif32 j = 0;

		while (argv[i][j] != '=' && argv[i][j] != 0) {
			param.insert(param.end(), argv[i][j]);
			++j;
		}

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help\t\t\tВывод этой справки и выход.\n";
			cout << "  -i, --input <file>\t\tLS кодировка шифра.\n";
			cout << "  -k, --key-size <size>\t\tЧисло бит ключа (только для блочных шифров).\n";
			cout << "  -m, --mode <g|o|p> \t\tСтратегия выбора следующей точки.\n";
			cout << "  -N, --sample-size <size>\tРазмер выборки.\n";
			cout << "  -o, --output <file>\t\tВыходной файл.\n";
			cout << "  -p, --start-point <file>\tФайл с описанием начальной точки.\n";
			cout << "  -t, --threads <number>\tЧисло потоков. По умолчанию равно thread::hardware_concurrency().\n";
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
		else if (param == "--mode" || param == "-m") { /// СДЕЛАТЬ ПРОВЕРКУ
			string m;
			if (argv[i][j] == 0) {
				m = (string) argv[++i];
			}
			else {
				m = (string) (argv[i] + j + 1);
			}

			if (m == "g") { // GREED
				mode = GREED;
			}
			else if (m == "p") { // PROB
				mode = PROB;
			}
			else if (m == "o") { // OPTIMAL
				mode = OPTIMAL;
			}
			else {
				cout << "parameter " << param << ": unknown value: `" << m << "`\n";
				exit(0);
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
}


uif32 rand_int() {
	return generator();
}


uif8 rand_bool() {
	return generator() & 1;
}


void resize_all(vector <vector <uif8> > &vec, uif32 cnt) {
	#pragma omp parallel for
	for (uif32 i = 0; i < vec.size(); ++i) {
		vec[i].resize(cnt);
	}
}


uif32 bin_set_size(vector <uif8> &a) {
	uif32 res = 0;
	for (uif8 i: a) {
		if (i) {
			++res;
		}
	}

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

	for (uif32 i = 0; i < key_vars_cnt; ++i) {
		uif32 x;
		fin >> x;
		x >>= 1;
		key_vars[i] = x;
	}
	for (uif32 i = 0; i < plaintext_vars_cnt; ++i) {
		uif32 x;
		fin >> x;
		x >>= 1;
		plaintext_vars[i] = x;
	}
	for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
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
	for (uif32 i = 0; i < N; ++i) {
		output_interp[i].resize(output_vars_cnt);
	}

	for (uif32 i = 0; i < output_vars_cnt; ++i) {
		uif32 x;
		fin >> x;
		uif8 inv = (uif8)(x & 1);
		x >>= 1;
		output_vars_inv[i] = inv;
		output_vars[i] = x;
	}

	cout << "ok" << endl;
}


void read_equations(ifstream &fin) {
	cout << "read equations ... ";
	cout.flush();
	for (uif32 i = 0; i < equations_cnt; ++i) {
		uif32 x, y, z;
		fin >> x >> y >> z;
		equations.push_back({x, y, z});
	}
	cout << "ok" << endl;
}


/// ЧТЕНИЕ AIG-ФАЙЛА С ОПИСАНИЕМ КОДИРОВКИ
void read_all(string in) {
cout << "read ..." << endl;
	ifstream fin(in.data());

	read_header(fin);
	read_input(fin);
	read_output(fin);
	read_equations(fin);

	fin.close();
}


/// ЧТЕНИЕ ФАЙЛА С ОПИСАНИЕМ НАЧАЛЬНОЙ ТОЧКИ ПОИСКА
void read_start_point_file(string &start_point_filename, vector <uif8> &start_point) {
	if (!start_point_filename.empty()) {
		cout << "read point (" << start_point_filename << ") ... ";
		cout.flush();

		start_point.resize(guessing_vars_cnt, 0);
		ifstream pin(start_point_filename.data());
		uif32 u;
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
		start_point.resize(guessing_vars_cnt, 1);
	}
}


void join_sets(uif32 x, uif32 y, vector <uif32> &dsu, vector <vector <uif32> > &childs) {
	x = dsu[x];
	y = dsu[y];

	if (childs[x].size() < childs[y].size()) {
		swap(x, y);
	}

	for (uif32 z: childs[y]) {
		dsu[z] = x;
		dsu[z ^ 1] = x ^ 1;
		childs[x].push_back(z);
		childs[x ^ 1].push_back(z ^ 1);
	}
	childs[y].clear();
	childs[y ^ 1].clear();
}


/// ВЫВОД ИЗ КВАДРАТИЧНОГО УРАВНЕНИЯ
/// ВОЗВРАЩАЕТ 1, ЕСЛИ ХОТЬ ЧТО-ТО ВЫВЕЛОСЬ
/// ИНАЧЕ 0
uif32 propagation(equation &e, vector <uif8> &vars_values, vector <uif8> &is_def, vector <uif32> &dsu, vector <vector <uif32> > &childs) {
	uif32 x = dsu[e.x];
	uif32 y = dsu[e.y];
	uif32 z = dsu[e.z];
	uif32 x2 = x / 2;
	uif32 y2 = y / 2;
	uif32 z2 = z / 2;

	uif8 val_x = vars_values[x2] ^ (uif8) (x & 1);
	uif8 val_y = vars_values[y2] ^ (uif8) (y & 1);
	uif8 val_z = vars_values[z2] ^ (uif8) (z & 1);

	if (is_def[x2] & is_def[y2] & is_def[z2]) {
		return 0;
	}

	if (z == y) { // 1
		if (is_def[z2] && (is_def[x2] ^ 1)) {
			is_def[x2] = 1;
			vars_values[x2] ^= val_z ^ val_x;
//			cout << "1.1 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
		if ((is_def[z2] ^ 1) && is_def[x2]) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ val_x;
//			cout << "1.2 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
		if (x == z) {
			// do nothing
//			cout << "1.3 " << e.x << " " << e.y << " " << e.z << endl;
			return 0;
		}
		join_sets(e.x, e.z, dsu, childs);
//		cout << "1.4 " << e.x << " " << e.y << " " << e.z << endl;
		return 1;
	}

	if (z == (y ^ 1)) { // 2
		if ((is_def[x2] ^ 1)) {
			is_def[x2] = 1;
			vars_values[x2] ^= val_x ^ 0;
//			cout << "2.1 " << e.x << " " << e.y << " " << e.z << endl; 
			return 1;
		}
//		cout << "2.2 " << e.x << " " << e.y << " " << e.z << endl;
		return 0;
	}

	if (z == x) { // 3
		if (is_def[z2] && val_z && (is_def[y2] ^ 1)) {
			is_def[y2] = 1;
			vars_values[y2] ^= val_y ^ 1;
//			cout << "3.1 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
		if (is_def[y2] && (val_y ^ 1) && (is_def[z2] ^ 1)) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ 0;
//			cout << "3.2 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
		if (z == (y ^ 1)) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ 0;
//			cout << "3.3 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
//		cout << "3.4 " << e.x << " " << e.y << " " << e.z << endl;
		return 0;
	}

	if (z == (x ^ 1)) { // 4
		if ((is_def[z2] ^ 1) || (is_def[y2] ^ 1)) {
			is_def[z2] = 1;
			is_def[y2] = 1;
			vars_values[z2] ^= val_z ^ 1;
			vars_values[y2] ^= val_y ^ 0;
//			cout << "4.1 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
//		cout << "4.2 " << e.x << " " << e.y << " " << e.z << endl;
		return 0;
	}

	if (y == x) { // 5
		if (is_def[z2] && (val_z ^ 1) && (is_def[y2] ^ 1)) {
			is_def[y2] = 1;
			vars_values[y2] ^= val_y ^ 0;
//			cout << "5.1 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
		if (is_def[y2] && val_y && (is_def[z2] ^ 1)) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ 1;
//			cout << "5.2 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
		if (z == (y ^ 1)) {
			is_def[z2] = 1;
			vars_values[z2] ^= val_z ^ 1;
//			cout << "5.3 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
//		cout << "5.4 " << e.x << " " << e.y << " " << e.z << endl;
		return 0;
	}

	if (y == (x ^ 1)) { // 6
		if ((is_def[z2] ^ 1) || (is_def[y2] ^ 1)) {
			is_def[z2] = 1;
			is_def[y2] = 1;
			vars_values[z2] ^= val_z ^ 0;
			vars_values[y2] ^= val_y ^ 1;
//			cout << "6.1 " << e.x << " " << e.y << " " << e.z << endl;
			return 1;
		}
//		cout << "6.2 " << e.x << " " << e.y << " " << e.z << endl;
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


void solve(uif8 &a, vector <uif8> &vars_values, vector <uif8> &is_def) {
	uif32 cnt = 0;
	vector <uif32> dsu(2 * (vars_cnt + 1));
	vector <vector <uif32> > childs(2 * (vars_cnt + 1));
	for (uif32 i = 0; i < dsu.size(); ++i) {
		dsu[i] = i;
		childs[i].push_back(i);
	}

	while (true) {
		cnt = 0;

		for (auto &e: equations) {
			cnt += propagation(e, vars_values, is_def, dsu, childs);
		}

		if (cnt == 0) {
			break;
		}
	}

	a = 1;
	for (auto &e: equations) {
		uif32 x = dsu[e.x];
		uif8 negx = x & 1;
		uif32 y = dsu[e.y];
		uif32 z = dsu[e.z];

		if ((is_def[y / 2] ^ 1) & (is_def[z / 2] ^ 1) & ((is_def[x / 2] ^ 1) | (vars_values[x / 2] ^ negx ^ 1)) & (y != z) & (y != (z ^ 1))) {
			a = 0;
			return;
		}
	}
}


void save_input_interp(uif32 i, vector <uif8> &vars_values) {
	for (uif32 j = 0; j < key_vars_cnt; ++j) {
		key_interp[i][j] = vars_values[key_vars[j]];
	}

	for (uif32 j = 0; j < plaintext_vars_cnt; ++j) {
		plaintext_interp[i][j] = vars_values[plaintext_vars[j]];
	}
}


void save_output_interp(uif32 i, vector <uif8> &vars_values) {
	for (uif32 j = 0; j < guessing_vars_cnt; ++j) {
		guessing_interp[i][j] = vars_values[guessing_vars[j]];
	}
	for (uif32 j = 0; j < output_vars_cnt; ++j) {
		output_interp[i][j] = vars_values[output_vars[j]];
	}
}


void solve_all(uif32 i, vector <vector <uif8> > &vars_values, vector <vector <uif8> > &is_def) {
	#pragma omp parallel for
	for (uif32 j = 0; j < cores; ++j) {
		if (i + j >= N) {
			continue;
		}

		fill(is_def[j].begin(), is_def[j].end(), 0);

		for (uif32 k = 0; k < key_vars_cnt; ++k) {
			is_def[j][key_vars[k]] = 1;
		}

		for (uif32 k = 0; k < plaintext_vars_cnt; ++k) {
			is_def[j][plaintext_vars[k]] = 1;
		}
	}

	#pragma omp parallel for
	for (uif32 j = 0; j < cores; ++j) {
		if (i + j >= N) {
			continue;
		}

		uif8 a = 0;
		solve(a, vars_values[j], is_def[j]);
	}
}


void gen_random_interp() {
	cout << "gen random sample ... ";
	cout.flush();
	vector <vector <uif8> > vars_values(cores);
	vector <vector <uif8> > is_def(cores);

	resize_all(vars_values, vars_cnt + 1);
	resize_all(is_def, vars_cnt + 1);

	for (uif32 i = 0; i < N; i += cores) {
		#pragma omp parallel for
		for (uif32 j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}

			for (uif32 k = 0; k < key_vars_cnt; ++k) {
				vars_values[j][key_vars[k]] = rand_bool();
			}
			
			for (uif32 k = 0; k < plaintext_vars_cnt; ++k) {
				vars_values[j][plaintext_vars[k]] = rand_bool();
			}

			save_input_interp(i + j, vars_values[j]);
		}
		solve_all(i, vars_values, is_def);

		#pragma omp parallel for
		for (uif32 j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}

			save_output_interp(i + j, vars_values[j]);
		}
	}

	cout << "ok" << endl;
}


void clear_vars_values(uif32 i, vector <uif8> &vars_set, vector <uif8> &vars_values, vector <uif8> &is_def) {
	#pragma omp parallel for
	for (uif32 j = 1; j <= vars_cnt; ++j) {
		is_def[j] = 0;
	}

	if (key_vars_cnt != 0) {
		for (uif32 j = 0; j < plaintext_vars_cnt; ++j) {
			is_def[plaintext_vars[j]] = 1;
			vars_values[plaintext_vars[j]] = plaintext_interp[i][j];
		}
	}

	for (uif32 j = 0; j < guessing_vars_cnt; ++j) {
		is_def[guessing_vars[j]] = vars_set[j];
		vars_values[guessing_vars[j]] = guessing_interp[i][j];
	}

	for (uif32 j = 0; j < output_vars_cnt; ++j) {
		is_def[output_vars[j]] = 1;
		vars_values[output_vars[j]] = output_interp[i][j];
	}
}


/// ОЦЕНОЧНАЯ ФУНКЦИЯ
double complexity(vector <uif8> &vars_set) {
	auto iter = complexity_set.find(vars_set);
	if (iter != complexity_set.end()) {
		return iter -> second;
	}

	uif32 cnt = 0;
	vector <uif8> answer(cores);
	vector <vector <uif8> > vars_values(cores);
	vector <vector <uif8> > is_def(cores);

	resize_all(vars_values, vars_cnt + 1);
	resize_all(is_def, vars_cnt + 1);

	for (uif32 i = 0; i < N; i += cores) {
		#pragma omp parallel for
		for (uif32 j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}

			clear_vars_values(i + j, vars_set, vars_values[j], is_def[j]);
		}

		#pragma omp parallel for
		for (uif32 j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}

			uif8 a;
			solve(a, vars_values[j], is_def[j]);
			answer[j] = a;
		}

		for (uif32 j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}

			if (answer[j]) {
				++cnt;
			}
		}
	}

	double prob = (double) (cnt) / N;
	double res;
	if (prob == 0) {
		res = DBL_MAX;
	}
	else {
		res = 3.0 * pow(2.0, (double) bin_set_size(vars_set)) / prob;
	}

	probability_set[vars_set] = prob;
	complexity_set[vars_set] = res;
	++points_cnt;

	return res;
}


/// ВЫБОР ТОЧКИ ИЗ tabu_list_2
/// ИЗМЕНИТЬ СТРАТЕГИЮ ВЫБОРА
void choose_from_tabu_list_2(vector <uif8> &vars_set, double &cmplx) {
	auto res = tabu_list_2.begin();
	cmplx = res -> first;
	vars_set = res -> second;
}


/// ВЫБОР ТОЧКИ ИЗ tabu_list_2_probability
/// ИЗМЕНИТЬ СТРАТЕГИЮ ВЫБОРА
void choose_from_tabu_list_2_probability(vector <uif8> &vars_set, double &prob) {
	auto res = tabu_list_2_probability.begin();
	prob = res -> first;
	vars_set = res -> second;
}


///  ПОИСК ОПТИМАЛЬНОГО ЛИНЕАРИЗАЦИОННОГО МНОЖЕСТВА (GREED)
void tabu_search_greed(vector <uif8> &start_point) {
	uif32 start_number = 0;
	uif32 start_time = time(0);

	vector <uif8> current_set(start_point);
	double current_complexity = complexity(current_set);

	while (true) {
		cout << "current point:\n";
		cout << "complexity: " << current_complexity << "\n";
		cout << "probability: " << probability_set[current_set] << "\n";
		cout << setw(5) << bin_set_size(current_set) << ":";
		for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
			if (current_set[i]) {
				cout << setw(5) << guessing_vars[i];
			}
		}
		cout << "\npoints have been viewed: " << points_cnt << " (" << time(0) - start_time << " sec)\n\n";
		cout.flush();

		bool to_next_point = false;

		/// ИЗМЕНЕНИЕ ОДНОЙ ПЕРЕМЕННОЙ
		for (uif32 u = start_number; u < guessing_vars_cnt; ++u) {
			current_set[u] = current_set[u] ^ true;

			if (tabu_list_1.find(current_set) != tabu_list_1.end()) {
				current_set[u] = current_set[u] ^ true;
				continue;
			}

			double cmplx = complexity(current_set);
			tabu_list_2.insert({cmplx, current_set});
			start_number_set[current_set] = 0;

			/// ЕСЛИ УЛУЧШИЛИ ОЦЕНКУ,
			/// ПЕРЕХОДИМ В НАЙДЕННУЮ ТОЧКУ
			if (cmplx < current_complexity) {
				if (u + 1 == guessing_vars_cnt) {
					current_set[u] = current_set[u] ^ true;

					tabu_list_1.insert(current_set);

					auto it = tabu_list_2.find({
						current_complexity,
						current_set
					});
					if (it != tabu_list_2.end()) {
						tabu_list_2.erase(it);
						start_number_set.erase(current_set);
					}

					current_set[u] = current_set[u] ^ true;
				}
				else {
					current_set[u] = current_set[u] ^ true;

					tabu_list_2.insert({
						current_complexity,
						current_set
					});
					start_number_set[current_set] = u + 1;

					current_set[u] = current_set[u] ^ true;
				}

				start_number = 0;
				current_complexity = cmplx;
				to_next_point = true;
				break;
			}

			current_set[u] = current_set[u] ^ true;
		}

		/// ПЕРЕХОД, ЕСЛИ БЫЛА НАЙДЕНА ТОЧКА ЛУЧШЕ ПРЕЖНЕЙ
		if (to_next_point) {
			continue;
		}

		/// ЕСЛИ ВСЕ СОСЕДИ ПРОСМОТРЕННЫ, НАДО
		/// ДОБАВИТЬ ТОЧКУ В tabu_list_1 И
		/// УДАЛИТЬ ИЗ tabu_list_2
		tabu_list_2.erase({
			current_complexity,
			current_set
		});
		start_number_set.erase(current_set);
		tabu_list_1.insert(current_set);

		if (tabu_list_2.empty()) {
			cout << "all points have been viewed\n";
			return;
		}

		/// ВЫБРАТЬ ТОЧКУ ИЗ 2-ГО СПИСКА
		choose_from_tabu_list_2(current_set, current_complexity);
		start_number = start_number_set[current_set];
	}
}


///  ПОИСК ОПТИМАЛЬНОГО ЛИНЕАРИЗАЦИОННОГО МНОЖЕСТВА (OPTIMAL)
void tabu_search_optimal(vector <uif8> &start_point) {
	vector <uif8> current_set(start_point);
	double current_complexity;
	uif32 start_time = time(0);

	vector <uif8> best_set;
	double best_complexity;

	while (true) {
		current_complexity = complexity(current_set);

		cout << "current point:\n";
		cout << "complexity: " << current_complexity << "\n";
		cout << "probability: " << probability_set[current_set] << "\n";
		cout << setw(5) << bin_set_size(current_set) << ":";
		for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
			if (current_set[i]) {
				cout << setw(5) << guessing_vars[i];
			}
		}
		cout << "\npoints have been viewed: " << points_cnt << " (" << time(0) - start_time << " sec)\n\n";
		cout.flush();

		best_complexity = DBL_MAX;
		/// ИЗМЕНЕНИЕ ОДНОЙ ПЕРЕМЕННОЙ
		for (uif32 u = 0; u < guessing_vars_cnt; ++u) {
			current_set[u] = current_set[u] ^ true;

			if (tabu_list_1.find(current_set) != tabu_list_1.end()) {
				current_set[u] = current_set[u] ^ true;
				continue;
			}

			double cmplx = complexity(current_set);
			tabu_list_2.insert({cmplx, current_set});
			start_number_set[current_set] = 0;

			/// ЕСЛИ УЛУЧШИЛИ ОЦЕНКУ,
			/// МЕНЯЕМ best_set И best_complexity
			if (cmplx < best_complexity) {
				best_set = current_set;
				best_complexity = cmplx;
			}

			current_set[u] = current_set[u] ^ true;
		}

		/// ДОБАВИТЬ ТОЧКУ В tabu_list_1 И
		/// УДАЛИТЬ ИЗ tabu_list_2
		tabu_list_1.insert(current_set);
		tabu_list_2.erase({
			current_complexity,
			current_set
		});

		/// ПЕРЕХОД, ЕСЛИ БЫЛА НАЙДЕНА ТОЧКА ЛУЧШЕ ПРЕЖНЕЙ
		if (best_complexity < current_complexity) {
			current_set = best_set;
			continue;
		}

		if (tabu_list_2.empty()) {
			cout << "all points have been viewed\n";
			return;
		}

		/// ВЫБРАТЬ ТОЧКУ ИЗ 2-ГО СПИСКА
		choose_from_tabu_list_2(current_set, current_complexity);
	}
}


///  ПОИСК ОПТИМАЛЬНОГО ЛИНЕАРИЗАЦИОННОГО МНОЖЕСТВА (PROBABILITY)
void tabu_search_probability(vector <uif8> &start_point) {
	vector <uif8> current_set(start_point);
	double current_probability;
	double current_complexity;
	uif32 start_time = time(0);

	vector <uif8> best_set;
	double best_probability;

	while (true) {
		current_complexity = complexity(current_set);
		current_probability = probability_set[current_set];

		cout << "current point:\n";
		cout << "complexity: " << current_complexity << "\n";
		cout << "probability: " << current_probability << "\n";
		cout << setw(5) << bin_set_size(current_set) << ":";
		for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
			if (current_set[i]) {
				cout << setw(5) << guessing_vars[i];
			}
		}
		cout << "\npoints have been viewed: " << points_cnt << " (" << time(0) - start_time << " sec)\n\n";
		cout.flush();

		best_probability = 0;
		/// ИЗМЕНЕНИЕ ОДНОЙ ПЕРЕМЕННОЙ
		for (uif32 u = 0; u < guessing_vars_cnt; ++u) {
			current_set[u] = current_set[u] ^ true;

			if (tabu_list_1.find(current_set) != tabu_list_1.end()) {
				current_set[u] = current_set[u] ^ true;
				continue;
			}

			complexity(current_set);
			double prob = probability_set[current_set];
			tabu_list_2_probability.insert({prob, current_set});
			start_number_set[current_set] = 0;

			/// ЕСЛИ УЛУЧШИЛИ ОЦЕНКУ,
			/// МЕНЯЕМ best_set И best_probability
			if (prob > best_probability) {
				best_set = current_set;
				best_probability = prob;
			}

			current_set[u] = current_set[u] ^ true;
		}

		/// ДОБАВИТЬ ТОЧКУ В tabu_list_1 И
		/// УДАЛИТЬ ИЗ tabu_list_2_probability
		tabu_list_1.insert(current_set);
		tabu_list_2_probability.erase({
			current_probability,
			current_set
		});

		/// ПЕРЕХОД, ЕСЛИ БЫЛА НАЙДЕНА ТОЧКА ЛУЧШЕ ПРЕЖНЕЙ
		if (best_probability > current_probability) {
			current_set = best_set;
			continue;
		}

		if (tabu_list_2_probability.empty()) {
			cout << "all points have been viewed\n";
			return;
		}

		/// ВЫБРАТЬ ТОЧКУ ИЗ 2-ГО СПИСКА
		choose_from_tabu_list_2_probability(current_set, current_probability);
	}
}


void tabu_search(vector <uif8> &start_point, uif32 mode, string &out) {
	FILE* w_file = freopen(out.data(), "w", stdout);
	cout << "search starts..." << endl;
	
	if (mode == GREED) {
		tabu_search_greed(start_point);
	}
	else if (mode == OPTIMAL) {
		tabu_search_optimal(start_point);
	}
	else if (mode == PROB) {
		tabu_search_probability(start_point);
	}
	
	fclose(w_file);
}


/// ВЫЧИСЛЕНИЕ ГЛУБИНЫ ВСЕХ ПЕРЕМЕННЫХ
void calculate_depth() {
	cout << "\ncalculate depth ... ";
	cout.flush();

	vars_depth.resize(vars_cnt + 1, 0);
	uif32 max_depth = 0;
	while (true) {
		bool f = false;
		for (auto e: equations) {
			uif32 x = e.x >> 1;
			uif32 y = e.y >> 1;
			uif32 z = e.z >> 1;
			
			uif32 vd = vars_depth[x];
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
	for (uif32 x = 1; x <= vars_cnt; ++x) {
		layers[vars_depth[x]].push_back(x);
	}

cout << "ok" << endl;
}


/// ВЫВОД ВСЕХ ПЕРЕМННЫХ ПО СЛОЯМ
void print_layers() {
	ofstream lout("layers");

	for (uif32 i = 0; i < layers.size(); ++i) {
		lout << "layer " << i << endl;
		for (uif32 x: layers[i]) {
			lout << x << " ";
		}
		lout << endl << endl;
	}

	lout.close();
}


int main(int argc, char *argv[]) {

	string in;
	string out;
	string spf;
	uif32 mode;

	init(argc, argv, in, out, spf, mode);

	ios::sync_with_stdio(false);

	read_all(in);

	calculate_depth();

	vector <uif8> sp;
	read_start_point_file(spf, sp);

	gen_random_interp();

	tabu_search(sp, mode, out);

}
