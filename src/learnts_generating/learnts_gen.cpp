#include <bits/stdc++.h>
#include <omp.h>

/*
#ifdef __linux__
	#include <sys/wait.h>
#endif
*/

using namespace std;

class learnt {
public:
	vector <int> lits;
	learnt() {}
	learnt(const learnt &l): lits(l.lits) {}
};


class relation {
public:
	vector <learnt> learnts;
	relation() {}
	
	void add(learnt &l) {
		learnts.push_back(l);
	}
};


void init(int argc, char *argv[], int &size, int &offset, string &relations_filename, string &cnf_filename, string &bin_filename, int &time_limit) {
	for (int i = 1; i < argc; ++i) {
		string param;
		int j = 0;

		for (; argv[i][j] != '=' && argv[i][j] != 0; ++j)
			param.insert(param.end(), argv[i][j]);

		if (param == "--help" || param == "-h") {
			cout << "  -h, --help             Вывод этой справки и выход.\n";
			cout << "  -cnf <file>            Путь к файлу с КНФ.\n";
			cout << "  -offset <int>          Номер начального соотношения (в некоторой нумерации).\n";
			cout << "  -size <int> [1..3]     Число переменных в соотношениях.\n";
			cout << "  -relations <file>      Файл, содержащий дополнительные соотношения в формате DIMACS (без заголовка).\n";
			cout << "  -bin <file>            Файл решателя.\n";
			cout << "  -time_lim <int>        Ограничение по времени для решателя в секундах.\n";
			exit(0);
		}
		else if (param == "-offset") {
			if (argv[i][j] == 0) {
				if (i + 1 >= argc)
					throw invalid_argument("error: offset parameter value not found");
				offset =  atoi(argv[++i]);
			}
			else
				offset = atoi(argv[i] + j + 1);
		}
		else if (param == "-relations") {
			if (argv[i][j] == 0) {
				if (i + 1 >= argc)
					throw invalid_argument("error: relations parameter value not found");
				relations_filename = (string) (argv[++i]);
			}
			else
				relations_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "-cnf") {
			if (argv[i][j] == 0) {
				if (i + 1 >= argc)
					throw invalid_argument("error: cnf parameter value not found");
				cnf_filename = (string) (argv[++i]);
			}
			else
				cnf_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "-size") {
			if (argv[i][j] == 0) {
				if (i + 1 >= argc)
					throw invalid_argument("error: size parameter value not found");
				size = atoi(argv[++i]);
			}
			else
				size = atoi(argv[i] + j + 1);
		}
		else if (param == "-bin") {
			if (argv[i][j] == 0) {
				if (i + 1 >= argc)
					throw invalid_argument("error: bin parameter value not found");
				bin_filename = (string) argv[++i];
			}
			else
				bin_filename = (string) (argv[i] + j + 1);
		}
		else if (param == "-time_lim") {
			if (argv[i][j] == 0) {
				if (i + 1 >= argc)
					throw invalid_argument("error: time_lim parameter value not found");
				time_limit = atoi(argv[++i]);
			}
			else
				time_limit = atoi(argv[i] + j + 1);
		}
		else {
			throw invalid_argument("unknown parameter: " + (string)(param));
		}
	}
}

void read_cnf(string &cnf, const string &filename) {
	cnf.clear();
	ifstream fin(filename.data());
	string line;
	while (getline(fin, line))
		cnf += line + "\n";
	fin.close();
}

void read_header(int &vars_cnt, const string &cnf) {
	string header = cnf.substr(0, cnf.find("\n"));
	stringstream ss;
	ss << header;

	string title;
    ss >> title;

    if (title == "p") {
        ss >> title;
        if (title == "cnf") {
	        ss >> vars_cnt;
	        return;
	    }
    }
   	throw logic_error("wrong input format: header expected in .cnf file");
}


void add_relation(vector <int> &d, string &cnf) {
	for (int x: d)
		cnf += to_string(-x) + " 0\n";
	/*
	int n = d.size();
	int val = 0;
	for (auto x: d) {
		val *= 2;
		if (x < 0)
			++val;
	}

	for (int i = 0; i < val; ++i) {
		for (int j = 0; j < n; ++j) {
			int var = ((i ^ val) & (1 << (n - j - 1)) ? -d[j] : d[j]);
			cnf += to_string(var) + " ";
		}
		cnf += "0\n";
	}
	for (int i = val + 1; i < (1 << n); ++i) {
		for (int j = 0; j < n; ++j) {
			int var = ((i ^ val) & (1 << (n - j - 1))? -d[j] : d[j]);
			cnf += to_string(var) + " ";
		}
		cnf += "0\n";
	}
	*/
}


int parallel_increase(int &val, mutex &mtx) {
	int res;
	mtx.lock();
		res = val;
		++val;
	mtx.unlock();
	return res;
}


void read_relations(string &relations_filename, vector <vector <int> > &r) {
	int x;
	vector <int> v;
	ifstream fin(relations_filename.data());
	while (fin >> x) {
		if (x == 0) {
			r.push_back(v);
			v.clear();
		}
		else {
			v.push_back(x);
		}
	}
	fin.close();
}


ostream& operator<< (ostream &os, vector <int> &d) {
	for (int x: d)
		os << x << " ";
	os << "0" << endl;
	return os;
}


int run_solver(string &bin_filename, string &cnf_filename, int time_limit = 1) {
	// return system((bin_filename + " " + cnf_filename + " out -time_lim=" + to_string(time_limit) + " > logs").data()) / 256;
	// return system((bin_filename + " " + cnf_filename + " out" + " > logs").data()) / 256;
	int code = system((bin_filename + " " + cnf_filename + " out > logs").data());
	// cout << code << endl;
	return code;
	// return WEXITSTATUS(...);
}


void print_progress(ostream &os, mutex &os_mtx, int progress, int cnt) {
	os_mtx.lock();
		os << progress << " / " << cnt << "\r";
		os.flush();
	os_mtx.unlock();
}


int main(int argc, char *argv[]) {

	ios::sync_with_stdio(0);
	cin.tie(0);
	cout.tie(0);

	string relations_filename, cnf_filename, bin_filename = "./minisat";
	int offset = 0, size = 0, vars_cnt = 0, counter = 0, time_limit = 1, error_count = 0, unknown_count = 0;
	mutex counter_mtx, learnts_mtx, cout_mtx, unknown_mtx, error_mtx;

	try {
		init(argc, argv, size, offset, relations_filename, cnf_filename, bin_filename, time_limit);
	}
	catch (invalid_argument &e) {
		cerr << e.what() << endl;
		return -1;
	}

	cout << cnf_filename << endl;
	cout << relations_filename << endl;
	cout << bin_filename << endl;

	string cnf;
	read_cnf(cnf, cnf_filename);
	try {
		read_header(vars_cnt, cnf);
	}
	catch (logic_error &e) {
		cerr << e.what() << endl;
		return -1;
	}
	ofstream learnts("learnts", ofstream::app);
	ofstream unknown("unknown");
	ofstream error("error");
	int start_time = time(0);

	// SOLVING RELATIONS FROM relations_filename //

	vector <vector <int>> relations;
	read_relations(relations_filename, relations);

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int) relations.size(); ++i) {
		// int thread_num = omp_get_thread_num();
		int thread_num = 0;
		int cnt = parallel_increase(counter, counter_mtx);
		if (cnt % 10 == 0)
			print_progress(cout, cout_mtx, cnt, relations.size());
		string cnf_copy = cnf;
		add_relation(relations[i], cnf_copy);
		string filename = to_string(thread_num) + ".cnf";
		ofstream fout(filename.data());
		fout << cnf_copy;
		fout.close();

		int code = run_solver(bin_filename, filename);
		if (code == 0) { // UNKNOWN
			unknown_mtx.lock();
				unknown << relations[i];
			unknown_mtx.unlock();
		}
		else if (code == 10) { // SAT
			// do nothing
		}
		else if (code == 20) { // UNSAT
			learnts_mtx.lock();
				learnts << relations[i];
			learnts_mtx.unlock();
		}
		else { // ERROR
			error_mtx.lock();
				error << relations[i];
			error_mtx.unlock();
			// throw logic_error("minisat error: unusual return code " + to_string(code));
		}
	}

/*
	counter = 0;
	if (size == 1) {
		#pragma omp parallel for schedule(dynamic)
		for (int i = 1; i <= vars_cnt; ++i) {
 			int thread_num = omp_get_thread_num();
			// int thread_num = 0;
			for (int neg = 0; neg < (1 << size); ++neg) {
				int cnt = parallel_increase(counter, counter_mtx);
				if (cnt % 1000 == 0)
					print_progress(cout, cout_mtx, cnt, vars_cnt * (1 << size));
				string cnf_copy = cnf;
				vector <int> d = {(neg == 0 ? -i : i)};
				add_relation(d, cnf_copy);
				string filename = to_string(thread_num) + "_" + filename;
				ofstream fout(filename.data());
				fout << cnf_copy;
				fout.close();
				
				int code = run_solver(bin_filename, filename);
				if (code == 0) { // UNKNOWN
					unknown_mtx.lock();
						unknown << d;
					unknown_mtx.unlock();
				}
				else if (code == 10) { // SAT
					// do nothing
				}
				else if (code == 20) { // UNSAT
					learnts_mtx.lock();
						learnts << d;
					learnts_mtx.unlock();
				}
				else { // ERROR
					error_mtx.lock();
						error << d;
					error_mtx.unlock();
					// throw logic_error("minisat error: unusual return code " + to_string(code));
				}
			}
		}
	}
	else if (size == 2) {
		for (int i = 1; i <= vars_cnt; ++i) {
			#pragma omp parallel for schedule(dynamic)
			for (int j = i + 1; j <= vars_cnt; ++j) {
	 			int thread_num = omp_get_thread_num();
				// int thread_num = 0;
				for (int neg = 0; neg < (1 << size); ++neg) {
					int cnt = parallel_increase(counter, counter_mtx);
					if (cnt % 1000 == 0)
						print_progress(cout, cout_mtx, cnt, vars_cnt * (vars_cnt - 1) / 2 * (1 << size));
					string cnf_copy = cnf;
					vector <int> d = {(neg & 2 ? -i : i), (neg & 1 ? -j : j)};
					add_relation(d, cnf_copy);
					string filename = to_string(thread_num) + "_" + cnf_filename;
					ofstream fout(filename.data());
					fout << cnf_copy;
					fout.close();
					
					int code = run_solver(bin_filename, filename);
					if (code == 0) { // UNKNOWN
						unknown_mtx.lock();
							unknown << d;
							++unknown_count;
						unknown_mtx.unlock();
					}
					else if (code == 10) { // SAT
						// do nothing
					}
					else if (code == 20) { // UNSAT
						learnts_mtx.lock();
							learnts << d;
						learnts_mtx.unlock();
					}
					else { // ERROR
						error_mtx.lock();
							error << d;
							++error_count;
						error_mtx.unlock();
						// throw logic_error("minisat error: unusual return code " + to_string(code));
					}
				}
			}
		}
	}
	else if (size == 3) {
		
	}
	else {
		throw invalid_argument("size value must be in range [1..3]");
	}
*/

	cout << "done: " << time(0) - start_time << " sec.\n";
	cout << "unknowns count: " << unknown_count << "\n";
	cout << "errors count: " << error_count << "\n";
	learnts.close();
	unknown.close();
	error.close();

}
