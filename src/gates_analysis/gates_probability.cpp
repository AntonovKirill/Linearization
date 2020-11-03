#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

using namespace std;


int N = 0;

int cores = thread::hardware_concurrency();

mt19937 generator(chrono::system_clock::now().time_since_epoch().count());

mutex cout_mtx, log_mtx, dists_mtx;

ofstream llog("logs", ofstream::app);

/// УРАВНЕНИЕ and
/// x ^ y & z = 0 ( x = y & z )
/// ПОД x, y, z ПОНИМАЮТСЯ ПЕРЕМЕННЫЕ
/// С НОМЕРАМИ x, y, z СООТВЕТСТВЕННО
struct equation {
	int x;
	int y;
	int z;
};

struct eq_value {
	char x;
	char y;
	char z;
};

operator< (const eq_value &a, const eq_value &b) {
	return ((((a.x << 1) ^ a.y) << 1) ^ a.z) < ((((b.x << 1) ^ b.y) << 1) ^ b.z);
}

struct dist {
	int eq_ind;
	map <eq_value, int> values;
	dist(int i): eq_ind(i) {}
};

operator< (const dist &a, const dist &b) {
	if (a.values.size() != b.values.size())
		return a.values.size() < b.values.size();
	vector <int> avals, bvals;
	for (auto &p: a.values)
		avals.push_back(p.second);
	for (auto &p: b.values)
		bvals.push_back(p.second);
	sort(all(avals));
	sort(all(bvals));
	return avals < bvals;
}

/// МНОЖЕСТВО ВСЕХ УРАВНЕНИЙ
vector <equation> equations;
vector <dist> dists;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector <int> key_vars;
vector <int> plaintext_vars;
vector <int> output_vars;
vector <int> output_vars_inv;


/// КОЛИЧЕСТВО УРАВНЕНИЙ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int equations_cnt;


/// КОЛИЧЕСТВО ВСЕХ ПЕРЕМЕННЫХ И
/// ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int vars_cnt = 0;
int input_vars_cnt = 0;
int key_vars_cnt = 0;
int plaintext_vars_cnt = 0;
int output_vars_cnt = 0;

void init(int argc, char* argv[], string &in, string &out) {
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
		else {
			cout << "unknown parameter: " << param << "\n";
			exit(0);
		}
	}

	if (cores == 0) {
		cores = 1;
	}
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

	key_vars.resize(key_vars_cnt);
	plaintext_vars.resize(plaintext_vars_cnt);

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

	cout << "ok" << endl;
}


void read_output(ifstream &fin) {
	cout << "read output ... ";
	cout.flush();
	output_vars.resize(output_vars_cnt);
	output_vars_inv.resize(output_vars_cnt);

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
		dist d(i);
		dists.push_back(d);
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


void find_output(vector <char> &vars_values, vector <char> &is_def) {
	int cnt = 0;

	while (true) {
		cnt = 0;
		for (auto &e: equations)
			cnt += propagation(e, vars_values, is_def);
		if (cnt == 0)
			break;
	}
	
	for (int i = 0; i < equations_cnt; ++i) {
		auto e = equations[i];
		eq_value ev = {(char)(vars_values[e.x / 2] ^ (e.x & 1)), (char)(vars_values[e.y / 2] ^ (e.y & 1)), (char)(vars_values[e.z / 2] ^ (e.z & 1))};
		dists_mtx.lock();
		++dists[i].values[ev];
		dists_mtx.unlock();
	}
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
		}
		solve_all(i, vars_values, is_def);

		#pragma omp parallel for
		for (int j = 0; j < cores; ++j) {
			if (i + j >= N) {
				continue;
			}
		}
	}

	cout << "ok" << endl;
}


void print_dist(dist &d, ostream &os) {
	auto e = equations[d.eq_ind];
	os << e.x << " " << e.y << " " << e.z << "\n";
	os << d.values.size() << ": ";
	for (auto &p: d.values)
		os << "(" << (int)p.first.x << (int)p.first.y << (int)p.first.z << "," << 1.0 * p.second / N << ") ";
	os << "\n";
}


void print_result(const string &out) {
	ofstream fout(out.data());
	sort(all(dists));
	for (auto &d: dists)
		print_dist(d, fout);
	fout.close();
}


int main(int argc, char *argv[]) {

	ios::sync_with_stdio(0);

	string in;
	string out;

	init(argc, argv, in, out);

	read_all(in);

	gen_random_interp();

	print_result(out);

	llog.close();

}
