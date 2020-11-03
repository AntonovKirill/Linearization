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


/// one-plus-one evolutionary algorithm parameters
uif32 max_steps;


/// УРАВНЕНИЕ and
/// x ^ y & z = 0 ( x = y & z )
/// ПОД x, y, z ПОНИМАЮТСЯ ПЕРЕМЕННЫЕ
/// С НОМЕРАМИ x, y, z СООТВЕТСТВЕННО
struct equation {
	uif32 x;
	uif32 y;
	uif32 z;
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


/// ГЛУБИНА ПЕРЕМЕННОЙ
vector <uif32> vars_depth;

/// СЛОИ ПЕРЕМЕННЫХ
vector <vector <uif32> > layers;

/// ЧИСЛО СЛОЁВ
uif32 layers_cnt = 0;


void init(uif32 argc, char* argv[], string &in, string &out, string &start_point_filename) {
	for (uif32 i = 1; i < argc; ++i) {
		string param;
		uif32 j = 0;

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
		else if (param == "--max-steps" || param == "-m") {
			if (argv[i][j] == 0) {
				max_steps = atoi(argv[++i]);
			}
			else {
				max_steps = atoi(argv[i] + j + 1);
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


/// ВЫВОД ИЗ КВАДРАТИЧНОГО УРАВНЕНИЯ
/// ВОЗВРАЩАЕТ 1, ЕСЛИ ХОТЬ ЧТО-ТО ВЫВЕЛОСЬ
/// ИНАЧЕ 0
uif32 propagation(equation &e, vector <uif8> &vars_values, vector <uif8> &is_def) {
	uif32 x = e.x >> 1;
	uif32 y = e.y >> 1;
	uif32 z = e.z >> 1;

	uif8 val_x = vars_values[x] ^ (uif8) (e.x & 1);
	uif8 val_y = vars_values[y] ^ (uif8) (e.y & 1);
	uif8 val_z = vars_values[z] ^ (uif8) (e.z & 1);
	
	if (is_def[x] & is_def[y] & is_def[z]) {
		if (val_x ^ (val_y & val_z) ^ 1) {
			return 0;
		}
		else {
mtx.lock();
cout << "wrong 1" << endl;
cout << e.x << " " << e.y << " " << e.z << endl;
cout << (uif32) val_x << " " << (uif32) val_y << " " << (uif32) val_z << endl;
mtx.unlock();
			exit(0);
		}
	}

	if (is_def[x] ^ 1) {
		if ((is_def[y] & (val_y ^ 1)) | (is_def[z] & (val_z ^ 1))) {
			is_def[x] = 1;
			vars_values[x] = e.x & 1;
			return 1;
		}
		else if (is_def[y] & val_y & is_def[z] & val_z) {
			is_def[x] = 1;
			vars_values[x] = (e.x & 1) ^ 1;
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
				vars_values[z] = e.z & 1;
				return 1;
			}
			else if (is_def[z] & val_z) {
				is_def[y] = 1;
				vars_values[y] = e.y & 1;
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			if ((is_def[y] & (val_y ^ 1)) | (is_def[z] & (val_z ^ 1))) {
mtx.lock();
cout << "wrong 2" << endl;
cout << e.x << " " << e.y << " " << e.z << endl;
cout << (uif32) val_x << " " << (uif32) val_y << " " << (uif32) val_z << endl;
mtx.unlock();
				exit(0);
			}
			else {
				if (is_def[y] ^ 1) {
					is_def[y] = 1;
					vars_values[y] = (e.y & 1) ^ 1;
				}
				if (is_def[z] ^ 1) {
					is_def[z] = 1;
					vars_values[z] = (e.z & 1) ^ 1;
				}
				return 1;
			}
		}
	}
}


void solve(uif8 &a, vector <uif8> &vars_values, vector <uif8> &is_def) {
	uif32 cnt = 0;

	while (true) {
		cnt = 0;

		for (auto &eq: equations) {
			cnt += propagation(eq, vars_values, is_def);
		}

		if (cnt == 0) {
			break;
		}
	}

	a = 1;

	for (auto &e: equations) {
		if ((is_def[e.y >> 1] ^ 1) & (is_def[e.z >> 1] ^ 1) & ((is_def[e.x >> 1] ^ 1) | (vars_values[e.x >> 1] ^ (e.x & 1) ^  1))) {
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


vector <uif8> mutation(vector <uif8> &v) {
	double prob = 1.0 / guessing_vars_cnt;
	vector <uif8> res(guessing_vars_cnt);

	#pragma omp parallel for
	for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
		if ((rand_int() * 1.0 / mt19937::max()) < prob)
			res[i] = v[i] ^ 1;
		else
			res[i] = v[i];
	}

	return res;
}


///  ПОИСК ОПТИМАЛЬНОГО ЛИНЕАРИЗАЦИОННОГО МНОЖЕСТВА
void run_one_plus_one(vector <uif8> start_point) {
	vector <uif8> parent = start_point, mutant;
	uif32 step = 0;
	uif32 start_time = time(0);

	double cmplx;
	cmplx = complexity(parent);

	cout << "current point:\n";
	cout << "complexity: " << cmplx << "\nprobability: " << probability_set[parent] << "\n";
	cout << setw(4) << bin_set_size(parent) << ":";
	for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
		if (parent[i]) {
			cout << setw(4) << guessing_vars[i];
		}
	}
	cout << "\n\n";
	cout.flush();

	while (true) {
		cout << "points has been viewed: " << points_cnt << "(" << time(0) - start_time << " sec)\n";

		mutant = mutation(parent);
		double new_cmplx = complexity(mutant);

		++step;
		if (new_cmplx < cmplx) {
			cmplx = new_cmplx;
			parent = mutant;

			cout << "attempts: " << step << "\n";
			cout << "current point:\n";
			cout << "complexity: " << cmplx << "\nprobability: " << probability_set[parent] << "\n";
			cout << setw(4) << bin_set_size(parent) << ":";
			for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
				if (parent[i]) {
					cout << setw(4) << guessing_vars[i];
				}
			}
			cout << "\n\n";
			cout.flush();

			step = 0;
		}

		if (step >= max_steps) {
			parent = start_point;
			cmplx = complexity(parent);
			step = 0;

			cout << "***new start***\n";
		}

		cout.flush();
	}

}


void one_plus_one(vector <uif8> &start_point, string out) {
	FILE* w_file = freopen(out.data(), "w", stdout);
	cout << "search starts..." << endl;

	run_one_plus_one(start_point);

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


int main(int argc, char *argv[]) {

	string in;
	string out;
	string spf;

	init(argc, argv, in, out, spf);

	ios::sync_with_stdio(false);

	read_all(in);

	calculate_depth();

	vector <uif8> sp;
	read_start_point_file(spf, sp);

	gen_random_interp();

	one_plus_one(sp, out);

}
