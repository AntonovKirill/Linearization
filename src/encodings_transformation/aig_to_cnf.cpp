#include <bits/stdc++.h>

using namespace std;

/// УРАВНЕНИЕ and
/// x ^ y & z = 0 ( x = y & z )
/// ПОД x, y, z ПОНИМАЮТСЯ ПЕРЕМЕННЫЕ
/// С НОМЕРАМИ x, y, z СООТВЕТСТВЕННО
class AndEquation {
public:
	int x;
	int y;
	int z;
};

vector<AndEquation> equations;


/// МАССИВЫ НОМЕРОВ ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ
vector<int> input_vars;
vector<int> output_vars;
vector<int> all_vars;


/// КОЛИЧЕСТВО УРАВНЕНИЙ
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА ФАЙЛА
int and_equations_cnt = 0;


/// КОЛИЧЕСТВО ВСЕХ ПЕРЕМЕННЫХ И
/// ВХОДНЫХ / ВЫХОДНЫХ ПЕРЕМЕННЫХ.
/// СЧИТЫВАЕТСЯ ИЗ ЗАГОЛОВКА AIG ФАЙЛА
int vars_cnt = 0;
int input_vars_cnt = 0;
int output_vars_cnt = 0;
int latches_cnt = 0;


/// ЧТЕНИЕ ПАРАМЕТРОВ ИЗ ТЕРМИНАЛА
void init(int argc, char *argv[], string &in_filename,
		string &out_filename, char &simple)
{
	simple = -1;

	for (int i = 1; i < argc; ++i) {
		string param;
		int j = 0;

		for (; argv[i][j] != '=' && argv[i][j] != 0; ++j)
			param.insert(param.end(), argv[i][j]);

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help                         Вывод этой справки и выход.\n";
			cout << "  -i, --input <file>                 Файл с описанием И-Не графа.\n";
			cout << "  -o, --output <file>                Файл для вывода КНФ.\n";
			cout << "  -s, --simple [0|1]    default=1    0 - требуется вывод доп. информации, выход в последние переменные; 1 - нет.";
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
		else if (param == "--simple" || param == "-s") {
			if (argv[i][j] == 0)
				simple = atoi(argv[++i]);
			else
				simple = atoi(argv[i] + j + 1);
		}
		else {
			clog << "unknown parameter: " << param << "\n";
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
	if (simple != 0 && simple != 1) {
		simple = 1;
		clog << "warning: void init(): parameter -s " <<
			"has been set to default value \'1\'\n";
	}
}


/********************
 * AIG-file reading *
 ********************/
/// HEADER INCLUDES NUMBER OF INPUT VARIABLES,
/// OUTPUT VARIABLES AND TOTAL NUMBER OF VARIABLES
void read_header(ifstream &fin)
{
	clog << " reading header ... ";
	clog.flush();

	string header;
	fin >> header;

	if (header == "aag") {
		fin >> vars_cnt >> input_vars_cnt >> latches_cnt >> output_vars_cnt >> and_equations_cnt;
	}
	else {
		cerr << "error: void read_header(): " <<
			"wrong input format: \'aag\' expected but \'" << header << "\' found\n";
		exit(0);
	}

	clog << "ok" << endl;
}


void read_input(ifstream &fin)
{
	clog << " reading input ... ";
	clog.flush();

	input_vars.clear();
	input_vars.resize(input_vars_cnt);

	for (int i = 0; i < input_vars_cnt; ++i) {
		int x;
		fin >> x;
		input_vars[i] = x;
		all_vars.push_back(x);
	}

	clog << "ok" << endl;
}


void read_output(ifstream &fin)
{
	clog << " reading output ... ";
	clog.flush();

	output_vars.resize(output_vars_cnt);

	for (int i = 0; i < output_vars_cnt; ++i) {
		int x;
		fin >> x;
		output_vars[i] = x;
		all_vars.push_back(x);
	}

	clog << "ok" << endl;
}


void read_equations(ifstream &fin)
{
	clog << " reading equations ... ";
	clog.flush();

	equations.resize(and_equations_cnt);
	for (int i = 0; i < and_equations_cnt; ++i) {
		int x, y, z;
		fin >> x >> y >> z;
		equations[i] = {x, min(y, z), max(y, z)};
		all_vars.push_back(x);
	}

	clog << "ok" << endl;
}


void read_aig(const string &in_filename)
{
	clog << "reading from \'" << in_filename << "\' ..." << endl;
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
void build_cnf_encoding(vector<vector<int> > &disjuncts, vector<int> &cnf_output_vars,
		int &literals, int &cnf_vars_cnt, const char add)
{
	vector<int> disjunct;
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

	if (add) {
		for (int i = 0; i < output_vars_cnt; ++i)
			cnf_vars_cnt = max(cnf_vars_cnt, output_vars[i] / 2);

		int var_num = cnf_vars_cnt;
		for (int i = 0; i < output_vars_cnt; ++i) {
			++var_num;
			cnf_output_vars.push_back(2 * var_num);
			int x = output_vars[i];
			x = (x & 1 ? -(x / 2) : x / 2);
			disjunct = {var_num, -x};
			disjuncts.push_back(disjunct);
			literals += 2;
			disjunct = {-var_num, x};
			disjuncts.push_back(disjunct);
			literals += 2;
		}

		cnf_vars_cnt += output_vars_cnt;
	}
	else {
		cnf_output_vars = output_vars;
	}
}


void print_cnf_header(ofstream &fout, int vars_cnt, int disjuncts_cnt)
{
	fout << "p cnf " << vars_cnt << " " << disjuncts_cnt << "\n";
}


void print_cnf_comments(ofstream &fout, vector<int> &cnf_output_vars, int literals)
{
	fout << "c input variables " << input_vars_cnt << "\n";
	fout << "c output variables " << output_vars_cnt << "\n";
	fout << "c literals " << literals << "\n";
	fout << "c input vars ";
	for (int x: input_vars)
		fout << (x & 1 ? -(x / 2) : x / 2) << " ";
	fout << "\n";
	fout << "c output vars ";
	for (int x: cnf_output_vars)
		fout << (x & 1 ? -(x / 2) : x / 2) << " ";
	fout << "\n";
}


void print_cnf_disjuncts(ofstream &fout, vector<vector<int> > &disjuncts)
{
	for (auto &d: disjuncts) {
		for (int x: d)
			fout << x << " ";
		fout << "0\n";
	}
}


void print_cnf(const string &filename, int vars_cnt, int literals_cnt,
		vector<int> &cnf_output_vars, vector<vector<int>> &disjuncts,
		const char add)
{
	ofstream fout(filename.data());

	if (add)
		print_cnf_header(fout, vars_cnt, disjuncts.size());
	if (add)
		print_cnf_comments(fout, cnf_output_vars, literals_cnt);
	print_cnf_disjuncts(fout, disjuncts);

	fout.close();
}


void aig_to_cnf(const string cnf_filename, const char add)
{
	clog << "aig to cnf ... ";
	clog.flush();

	vector<vector<int>> disjuncts;
	vector<int> cnf_output_vars;
	int literals = 0;
	int cnf_vars_cnt = 0;
	build_cnf_encoding(disjuncts, cnf_output_vars, literals, cnf_vars_cnt, add);
	print_cnf(cnf_filename, cnf_vars_cnt, literals, cnf_output_vars, disjuncts, add);

	clog << "ok" << endl;
}


/*****************
 * main function *
 *****************/
int main(int argc, char *argv[])
{

	string in_filename, out_filename;
	char simple;
	init(argc, argv, in_filename, out_filename, simple);

	read_aig(in_filename);
	aig_to_cnf(out_filename, simple ^ 1);

}
