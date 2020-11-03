#include <bits/stdc++.h>

typedef long long ll;
typedef unsigned int uint;

using namespace std;

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

vector <and_equation> equations;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector <uint> key_vars;
vector <uint> iv_vars;
vector <uint> input_vars;
vector <uint> output_vars;
vector <char> output_vars_inv;


/// КОЛИЧЕСТВО УРАВНЕНИЙ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
uint and_equations_cnt = 0;


/// КОЛИЧЕСТВО ВСЕХ ПЕРЕМЕННЫХ И
/// ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ.
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА AIG ФАЙЛА
uint vars_cnt = 0;
uint input_vars_cnt = 0;
uint key_vars_cnt = 0;
uint iv_vars_cnt = 0;
uint output_vars_cnt = 0;
uint latches_cnt = 0;


/// ЧТЕНИЕ ПАРАМЕТРОВ ИЗ ТЕРМИНАЛА
void init(int argc, char *argv[], string &in_filename, string &out_filename) {
	for (uint i = 1; i < (uint) argc; ++i) {
		string param;
		uint j = 0;

		for (; argv[i][j] != '=' && argv[i][j] != 0; ++j)
			param.insert(param.end(), argv[i][j]);

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help										  Вывод этой справки и выход.\n";
			cout << "  -i, --input <file>								  Файл с описанием И-Не графа.\n";
			cout << "  -o, --output <file>								  Файл для вывода КНФ.\n";
			cout << "  -k, --key-size <size>	default=input_vars_cnt	  Число бит ключа.\n";
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
		else if (param == "--key-size" || param == "-k") { /// СДЕЛАТЬ ПРОВЕРКУ НА НЕОТРИЦАТЕЛЬНОСТЬ
			if (argv[i][j] == 0)
				key_vars_cnt = (uint) atoi(argv[++i]);
			else
				key_vars_cnt = (uint) atoi(argv[i] + j + 1);
		}
		else {
			cout << "unknown parameter: " << param << "\n";
			exit(0);
		}
	}
}


/********************
 * AIG-file reading *
 ********************/
/// HEADER INCLUDES NUMBER OF INPUT VARIABLES,
/// OUTPUT VARIABLES AND TOTAL NUMBER OF VARIABLES
void read_header(ifstream &fin) {
	cout << "reading header ... ";
	cout.flush();

	string header;
	fin >> header;

	if (header == "aag") {
		fin >> vars_cnt >> input_vars_cnt >> latches_cnt >> output_vars_cnt >> and_equations_cnt;
	}
	else {
		cout << "wrong input format: `aag` expected but `" << header << "` found" << endl;
		exit(0);
	}

	cout << "ok" << endl;
}

void read_input(ifstream &fin) {
	cout << "reading input ... ";
	cout.flush();

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
		uint x;
		fin >> x;
		x /= 2;
		key_vars[i] = x;
		input_vars[i] = x;
	}
	for (uint i = 0; i < iv_vars_cnt; ++i) {
		uint x;
		fin >> x;
		x /= 2;
		iv_vars[i] = x;
		input_vars[key_vars_cnt + i] = x;
	}

	cout << "ok" << endl;
}

void read_output(ifstream &fin) {
	cout << "reading output ... ";
	cout.flush();

	output_vars.resize(output_vars_cnt);
	output_vars_inv.resize(output_vars_cnt);

	for (uint i = 0; i < output_vars_cnt; ++i) {
		uint x;
		fin >> x;
		char inv = x & 1;
		x /= 2;
		output_vars_inv[i] = inv;
		output_vars[i] = x;
	}

	cout << "ok" << endl;
}

void read_equations(ifstream &fin) {
	cout << "reading equations ... ";
	cout.flush();

	equations.resize(and_equations_cnt);
	for (uint i = 0; i < and_equations_cnt; ++i) {
		uint x, y, z;
		fin >> x >> y >> z;
		equations[i] = {x, min(y, z), max(y, z)};
	}

	cout << "ok" << endl;
}

void read_aig(const string &in_filename) {
	cout << "reading from \'" << in_filename << "\'..." << endl;
	ifstream fin(in_filename.data());

	read_header(fin);
	read_input(fin);
	read_output(fin);
	read_equations(fin);

	fin.close();
}


/**************
 * aig to cnf *
 **************/
void build_cnf_encoding(vector <vector <int> > &disjuncts, vector <int> &cnf_output_vars,
		uint &literals, uint &cnf_vars_cnt)
{
	vector <int> disjunct;
	for (auto &e: equations) {
		int x = e.x / 2;
		x = (e.x & 1 ? -x : x);
		int y = e.y / 2;
		y = (e.y & 1 ? -y : y);
		int z = e.z / 2;
		z = (e.z & 1 ? -z : z);

		disjunct = {-x, y};
		disjuncts.push_back(disjunct);
		disjunct = {-x, z};
		disjuncts.push_back(disjunct);
		disjunct = {x, -y, -z};
		disjuncts.push_back(disjunct);
		literals += 7;
	}

	for (uint i = 0; i < output_vars_cnt; ++i)
		cnf_vars_cnt = max(cnf_vars_cnt, output_vars[i]);

	int var_num = cnf_vars_cnt;
	for (uint i = 0; i < output_vars_cnt; ++i) {
		++var_num;
		cnf_output_vars.push_back(var_num);
		int x = output_vars[i];
		disjunct = {var_num, (output_vars_inv[i] ? x : -x)};
		disjuncts.push_back(disjunct);
		literals += 2;
		disjunct = {-var_num, (output_vars_inv[i] ? -x : x)};
		disjuncts.push_back(disjunct);
		literals += 2;
	}
	cnf_vars_cnt += output_vars_cnt;
}

void print_cnf_header(ofstream &fout, uint vars_cnt, uint disjuncts_cnt) {
	fout << "p cnf " << vars_cnt << " " << disjuncts_cnt << endl;
}

void print_cnf_comments(ofstream &fout, vector <int> &cnf_output_vars, uint literals) {
	fout << "c input variables " << input_vars_cnt << "\n";
	fout << "c output variables " << output_vars_cnt << "\n";
	fout << "c literals " << literals << "\n";
	fout << "c key vars ";
	for (uint i = 0; i < key_vars_cnt; ++i)
		fout << key_vars[i] << " ";
	fout << "\n";
	if (iv_vars_cnt > 0) {
		fout << "c initial value vars ";
		for (uint i = 0; i < iv_vars_cnt; ++i)
			fout << iv_vars[i] << " ";
		fout << "\n";
	}
	fout << "c output vars ";
	for (uint i = 0; i < output_vars_cnt; ++i)
		fout << cnf_output_vars[i] << " ";
	fout << endl;
}

void print_cnf_disjuncts(ofstream &fout, vector <vector <int> > &disjuncts) {
	for (auto &d: disjuncts) {
		for (int x: d)
			fout << x << " ";
		fout << "0\n";
	}
}


void print_cnf(const string &filename, uint vars_cnt, uint literals_cnt,
		vector <int> &cnf_output_vars, vector <vector <int>> &disjuncts)
{
	ofstream fout(filename.data());

	print_cnf_header(fout, vars_cnt, disjuncts.size());
	print_cnf_comments(fout, cnf_output_vars, literals_cnt);
	print_cnf_disjuncts(fout, disjuncts);

	fout.close();
}

void aig_to_cnf(string cnf_filename) {
	cout << "aig to cnf ... ";
	cout.flush();

	vector <vector <int>> disjuncts;
	vector <int> cnf_output_vars;
	uint literals = 0;
	uint cnf_vars_cnt = 0;
	build_cnf_encoding(disjuncts, cnf_output_vars, literals, cnf_vars_cnt);
	print_cnf(cnf_filename, cnf_vars_cnt, literals, cnf_output_vars, disjuncts);

	cout << "ok" << endl;
}


/*****************
 * main function *
 *****************/
int main(int argc, char *argv[]) {

	string in_filename, out_filename;
	init(argc, argv, in_filename, out_filename);

	read_aig(in_filename);
	aig_to_cnf(out_filename);

}
