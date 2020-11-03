#include <sstream>
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
#include <map>
#include <set>


#define uif8 uint_fast8_t
#define uif32 uint_fast32_t

using namespace std;


uif32 N = 0;

uif32 cores = thread::hardware_concurrency();

uif32 points_cnt = 0;

double prob_threshold;
uif32 tries_limit;

mt19937 generator(chrono::system_clock::now().time_since_epoch().count());

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

/// МНОЖЕСТВО ВСЕХ УРАВНЕНИЙ
/// СДЕЛАТЬ atomic
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


/// МНОЖЕСТВА ДЛЯ АЛГОРИТМА rise-fall
set <vector <uif8> > tabu_list;

/// МОЖНО МЕНЯТЬ НА unordered_map
map <vector <uif8>, double> complexity_set;
map <vector <uif8>, double> probability_set;


/// ГЛУБИНА ПЕРЕМЕННОЙ
vector <uif32> vars_depth;

/// СЛОИ ПЕРЕМЕННЫХ
vector <vector <uif32> > layers;

/// ЧИСЛО СЛОЁВ
uif32 layers_cnt = 0;


/// ЧТЕНИЕ ПАРАМЕТРОВ ИЗ ТЕРМИНАЛА
void init(int argc, char *argv[], string &in, string &out, string &start_point_filename) {
	for (uif32 i = 1; i < (uif32)argc; ++i) {
		string param;
		uif32 j = 0;

		while (argv[i][j] != '=' && argv[i][j] != 0) {
			param.insert(param.end(), argv[i][j]);
			++j;
		}

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help\t\t\tВывод этой справки и выход.\n";
			cout << "  -i, --input <file>\t\tAIG кодировка КНФ.\n";
			cout << "  -k, --key-size <size>\t\tЧисло бит ключа (только для блочных шифров).\n";
			cout << "  -K, --tries-limit <number>\tПредел числа попыток спуска.\n";
			cout << "  -N, --sample-size <size>\tРазмер выборки.\n";
			cout << "  -o, --output <file>\t\tВыходной файл.\n";
			cout << "  -p, --start-point <file>\tФайл с описанием начальной точки.\n";
			cout << "  -P, --prob-threshold <float>\tПороговое значение вероятности при восхождении.\n";
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
		else if (param == "--prob-threshold" || param == "-P") {
			if (argv[i][j] == 0) {
				prob_threshold = atof(argv[++i]);
			}
			else {
				prob_threshold = atof(argv[i] + j + 1);
			}
		}
		else if (param == "--tries-limit" || param == "-K") {
			if (argv[i][j] == 0) {
				tries_limit = atoi(argv[++i]);
			}
			else {
				tries_limit = atoi(argv[i] + j + 1);
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
		cout << "wrong input format: `aag` expected but `" << header << "` found" << endl;
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
		start_point.resize(guessing_vars_cnt, 0);
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
		if ((is_def[e.y >> 1] ^ 1) & (is_def[e.z >> 1] ^ 1) & ((is_def[e.x >> 1] ^ 1) | (vars_values[e.x >> 1] ^ (e.x & 1)))) {
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

/*
void rise_fall(vector <uif8> start_set) {
	vector <uif8> current_set, best_set;
	double current_complexity;
	double current_probability;
	uif32 start_time = time(0);

	double cbest = DBL_MAX;
	bool NT_is_empty = false;
	priority_queue <
			pair <double, vector <uif8> >,
			vector <pair <double, vector <uif8> > >,
			greater <pair <double, vector <uif8> > >
	> q;

	while (true) {
		cout << "new start:" << endl;
		current_set = start_set;

		// rise
		cout << "rise:" << endl;
		do {
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
			cout << "\npoints has been viewed: " << points_cnt << " (" << time(0) - start_time << " sec)\n\n";
			cout.flush();

			vector <vector <uif8> > top_neighbours;

			for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
				if (current_set[i] ^ 1) {
					current_set[i] = 1;
					if (tabu_list.find(current_set) == tabu_list.end()) {
						top_neighbours.push_back(current_set);
					}
					current_set[i] = 0;
				}
			}

			if (top_neighbours.empty()) {
				cout << "NT is empty" << endl;
				NT_is_empty = true;
				break;
			}

			uif32 ind = rand_int() % top_neighbours.size();
			current_set = top_neighbours[ind];
			current_complexity = complexity(current_set);
			tabu_list.insert(current_set);

		} while (probability_set[current_set] < prob_threshold);

		if (NT_is_empty) {
			continue;
		}

		// fall
		cout << "fall:" << endl;
		uif32 tries = 0;
		q.push({current_complexity, current_set});

		do {
			if (q.empty()) {
				cout << "queue is empty" << endl;
				break;
			}

			current_set = q.top().second;
			current_complexity = q.top().first;
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
			cout << "\npoints has been viewed: " << points_cnt << " (" << time(0) - start_time << " sec)\n\n";
			cout.flush();

			vector <vector <uif8> > bottom_neighbours;

			for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
				if (current_set[i]) {
					current_set[i] = 0;
					if (tabu_list.find(current_set) == tabu_list.end()) {
						bottom_neighbours.push_back(current_set);
					}
					current_set[i] = 1;
				}
			}
			if (bottom_neighbours.size() > 0) {
				uif32 ind = rand_int() % bottom_neighbours.size();
				current_set = bottom_neighbours[ind];

				current_complexity = complexity(current_set);
				tabu_list.insert(current_set);
				q.push({current_complexity, current_set});

				if (current_complexity < cbest) {
					best_set = current_set;
					cbest = current_complexity;
					tries = 0;
				}
				else {
					++tries;
				}
			}
			else {
				q.pop();
			}
		} while (tries < tries_limit && !q.empty());
	}
}
*/


/// UNCHECKED BOTTOM NEIGHBOURS
/// DISTANCE FROM current_set LESS OR EQUALS TO distance
/*vector <vector <uif8> > NB(vector <uif8> &current_set, uif32 distance, vector uif32 start_ind) {
	vector <vector <uif8> > res;
	if (start_ind >= guessing_vars_cnt) {
		return res;
	}

	
}*/



void rise_fall(vector <uif8> start_set) {
	vector <uif8> current_set, best_set;
	double current_complexity;
	double current_probability;
	uif32 start_time = time(0);

	double cbest = DBL_MAX;

	while (true) {
		cout << "new start:" << endl;

		current_set = start_set;
		bool NT_is_empty = false;

		priority_queue <
				pair <double, vector <uif8> >,
				vector <pair <double, vector <uif8> > >,
				greater <pair <double, vector <uif8> > >
		> q;
		set <vector <uif8> > tabu_list;

		tabu_list.insert(current_set);

		// rise
		cout << "rise:" << endl;
		do {
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
			cout << "\npoints has been viewed: " << points_cnt << " (" << time(0) - start_time << " sec)\n\n";
			cout.flush();

			vector <vector <uif8> > top_neighbours;

			for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
				if (current_set[i] ^ 1) {
					current_set[i] = 1;
					if (tabu_list.find(current_set) == tabu_list.end()) {
						top_neighbours.push_back(current_set);
					}
					current_set[i] = 0;
				}
			}

			if (top_neighbours.empty()) {
				cout << "NT is empty" << endl;
				NT_is_empty = true;
				break;
			}

			uif32 ind = rand_int() % top_neighbours.size();
			current_set = top_neighbours[ind];
			current_complexity = complexity(current_set);
			tabu_list.insert(current_set);

		} while (probability_set[current_set] < prob_threshold);

		if (NT_is_empty) {
			continue;
		}

		// fall
		cout << "fall:" << endl;
		uif32 tries = 0;
		q.push({current_complexity, current_set});

		do {
			if (q.empty()) {
				cout << "queue is empty" << endl;
				break;
			}

			current_set = q.top().second;
			current_complexity = q.top().first;
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
			cout << "\npoints has been viewed: " << points_cnt << " (" << time(0) - start_time << " sec)\n\n";
			cout.flush();

			vector <vector <uif8> > bottom_neighbours;

			for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
				if (current_set[i]) {
					current_set[i] = 0;
					if (tabu_list.find(current_set) == tabu_list.end()) {
						bottom_neighbours.push_back(current_set);
					}
					current_set[i] = 1;
				}
			}
			
			vector <vector <uif8> > bottom_neighbours1;
			vector <vector <uif8> > bottom_neighbours2;
			vector <vector <uif8> > bottom_neighbours3;

			for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
				if (current_set[i]) {
					current_set[i] = current_set[i] ^ 1;
					bottom_neighbours1.push_back(current_set);
					current_set[i] = current_set[i] ^ 1;
				}
			}
			
			for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
				for (uif32 j = i + 1; j < guessing_vars_cnt; ++j) {
					if (current_set[i] | current_set[j]) {
						current_set[i] = current_set[i] ^ 1;
						current_set[j] = current_set[j] ^ 1;
						bottom_neighbours2.push_back(current_set);
						current_set[i] = current_set[i] ^ 1;
						current_set[j] = current_set[j] ^ 1;
					}
				}
			}
			
			for (uif32 i = 0; i < guessing_vars_cnt; ++i) {
				for (uif32 j = i + 1; j < guessing_vars_cnt; ++j) {
					for (uif32 k = j + 1; k < guessing_vars_cnt; ++k) {
						if ((current_set[i] & current_set[j]) | (current_set[i] & current_set[k]) | (current_set[j] & current_set[k])) {
							current_set[i] = current_set[i] ^ 1;
							current_set[j] = current_set[j] ^ 1;
							current_set[k] = current_set[k] ^ 1;
							bottom_neighbours3.push_back(current_set);
							current_set[i] = current_set[i] ^ 1;
							current_set[j] = current_set[j] ^ 1;
							current_set[k] = current_set[k] ^ 1;
						}
					}
				}
			}

			bottom_neighbours.insert(bottom_neighbours.end(), bottom_neighbours1.begin(), bottom_neighbours1.end());
			bottom_neighbours.insert(bottom_neighbours.end(), bottom_neighbours2.begin(), bottom_neighbours2.end());
			bottom_neighbours.insert(bottom_neighbours.end(), bottom_neighbours3.begin(), bottom_neighbours3.end());

			if (bottom_neighbours.size() > 0) {
				uif32 ind = rand_int() % bottom_neighbours.size();
				current_set = bottom_neighbours[ind];

				current_complexity = complexity(current_set);
				tabu_list.insert(current_set);
				q.push({current_complexity, current_set});

				if (current_complexity < cbest) {
					best_set = current_set;
					cbest = current_complexity;
					tries = 0;
				}
				else {
					++tries;
				}
			}
			else {
				q.pop();
			}
		} while (tries < tries_limit && !q.empty());
	}
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

	FILE *w_file;
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

	w_file = freopen(out.data(), "w", stdout);

	cout << "search starts..." << endl;
	rise_fall(sp);
	fclose(w_file);

}
