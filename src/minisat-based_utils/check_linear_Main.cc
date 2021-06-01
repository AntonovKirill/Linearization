/*****************************************************************************************[Main.cc]
Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include <errno.h>
#include <zlib.h>

#include "minisat/utils/System.h"
#include "minisat/utils/ParseUtils.h"
#include "minisat/utils/Options.h"
#include "minisat/core/Dimacs.h"
#include "minisat/core/Solver.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <algorithm>
#include <chrono>
#include <random>



using namespace Minisat;

#define all(x) (x).begin(), (x).end()

//=================================================================================================


static Solver* solver;
// Terminate by notifying the solver and back out gracefully. This is mainly to have a test-case
// for this feature of the Solver as it may take longer than an immediate call to '_exit()'.
static void SIGINT_interrupt(int) { solver->interrupt(); }

// Note that '_exit()' rather than 'exit()' has to be used. The reason is that 'exit()' calls
// destructors and may cause deadlocks if a malloc/free function happens to be running (these
// functions are guarded by locks for multithreaded use).
static void SIGINT_exit(int) {
    printf("c \n"); printf("c *** INTERRUPTED ***\n");
    if (solver->verbosity > 0){
        solver->printStats();
        printf("c \n"); printf("c *** INTERRUPTED ***\n"); }
    _exit(1); }


class AndEquation {
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
	char bit;
	
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

bool operator< (const std::vector<Bit> &a, const std::vector<Bit> &b)
{
	if (a.size() < b.size())
		return true;
	if (a.size() > b.size())
		return false;
	
	for (int i = 0; i < (int)a.size(); ++i) {
		if (a[i] < b[i])
			return true;
		if (b[i] < a[i])
			return false;
	}

	return false;
}


int vars_cnt = 0;
int input_vars_cnt = 0;
int latches_cnt = 0;
int output_vars_cnt = 0;
int and_equations_cnt = 0;
std::vector<int> input_vars;
std::vector<int> output_vars;
std::vector<int> all_vars;

std::vector<AndEquation> equations;
// std::set <std::vector<int>> pattern_linear_constraints;

std::map<std::vector<Bit>, std::vector<std::vector<int>>> lin_table;


/********************
 * AIG-file reading *
 ********************/
/// HEADER INCLUDES NUMBER OF INPUT VARIABLES,
/// OUTPUT VARIABLES AND TOTAL NUMBER OF VARIABLES
int read_header(std::ifstream &fin)
{
	std::string header;
	fin >> header;

	if (header == "aag") {
		fin >> vars_cnt >> input_vars_cnt >> latches_cnt >> output_vars_cnt >> and_equations_cnt;
		return 0;
	}
	else {
		std::cerr << "wrong input format: `aag` expected but `" << header << "` found" << std::endl;
		return 1;
	}
}

void read_input(std::ifstream &fin)
{
	input_vars.resize(input_vars_cnt);
	
	for (int i = 0; i < input_vars_cnt; ++i) {
		fin >> input_vars[i];
		
		all_vars.push_back(input_vars[i]);
	}
}

void read_output(std::ifstream &fin)
{
	output_vars.resize(output_vars_cnt);
	
	for (int i = 0; i < output_vars_cnt; ++i)
		fin >> output_vars[i];
}

void read_equations(std::ifstream &fin)
{
	equations.resize(and_equations_cnt);

	for (int i = 0; i < and_equations_cnt; ++i) {
		int x, y, z;
		fin >> x >> y >> z;
		equations[i] = {x, std::min(y, z), std::max(y, z)};
		
		all_vars.push_back(x);
	}
}

int read_aig(const char* in_filename)
{
	std::ifstream fin(in_filename);
	
	if (read_header(fin))
		return 1;
	
	read_input(fin);
	read_output(fin);
	read_equations(fin);
	
	fin.close();
	
	return 0;
}


int signed_half(int x)
{
	return (x & 1 ? -(x / 2) : x / 2);
}


void read_lin_table(const std::string &lt_filename,
		std::map<std::vector<Bit>, std::vector<std::vector<int>>> &lt)
{
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
			nums.pop_back();
			int n = nums.size();
			
			for (int i = 0; i < (int)nums.size(); ++i)
				key.push_back({nums[i], (x >> (n - 1 - i)) & 1});

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
}


//=================================================================================================
// Main:


int main(int argc, char** argv)
{

    try {
        setUsageHelp("USAGE: %s [options] <input-file> <result-output-file>\n\n  where input may be either in plain or gzipped DIMACS.\n");
        setX86FPUPrecision();

        // Extra options:
        //
        IntOption    verb   ("MAIN", "verb",   "Verbosity level (0=silent, 1=some, 2=more).", 1, IntRange(0, 2));
        IntOption    cpu_lim("MAIN", "cpu-lim","Limit on CPU time allowed in seconds.\n", 0, IntRange(0, INT32_MAX));
        IntOption    mem_lim("MAIN", "mem-lim","Limit on memory usage in megabytes.\n", 0, IntRange(0, INT32_MAX));
        BoolOption   strictp("MAIN", "strict", "Validate DIMACS header during parsing.", false);
        BoolOption   model  ("MAIN", "model", "Print the values for the model in case of satisfiable.", true);
        StringOption proof  ("MAIN", "proof",  "Given a filename, a DRAT proof will be written there.");
        
        parseOptions(argc, argv, true);

        Solver S;
        double initial_time = cpuTime();

        S.verbosity = verb;
        
        solver = &S;
        // Use signal handlers that forcibly quit until the solver will be able to respond to
        // interrupts:
        sigTerm(SIGINT_exit);

        // Try to set resource limits:
        if (cpu_lim != 0) limitTime(cpu_lim);
        if (mem_lim != 0) limitMemory(mem_lim);
        
        if (argc == 1)
            printf("c Reading from standard input... Use '--help' for help.\n");
        
        gzFile in = (argc == 1) ? gzdopen(0, "rb") : gzopen(argv[1], "rb");
        if (in == NULL)
            printf("c ERROR! Could not open file: %s\n", argc == 1 ? "<stdin>" : argv[1]), exit(1);
        
        if (S.verbosity > 0){
            printf("c ============================[ Problem Statistics ]=============================\n");
            printf("c |                                                                             |\n"); }
        
        if((const char *)proof)
            if(!S.openProofFile((const char *)proof)){
                printf("c ERROR! Could not open proof file: %s\n", (const char *)proof);
                exit(1);
            }

        parse_DIMACS(in, S, (bool)strictp);
        gzclose(in);
        FILE* res = (argc >= 3) ? fopen(argv[2], "wb") : NULL;
        
        if (S.verbosity > 0){
            printf("c |  Number of variables:  %12d                                         |\n", S.nVars());
            printf("c |  Number of clauses:    %12d                                         |\n", S.nClauses()); }
        
        double parsed_time = cpuTime();
        if (S.verbosity > 0){
            printf("c |  Parse time:           %12.2f s                                       |\n", parsed_time - initial_time);
            printf("c |                                                                             |\n"); }
 
        // Change to signal-handlers that will only notify the solver and allow it to terminate
        // voluntarily:
        sigTerm(SIGINT_interrupt);
       
        if (!S.simplify()){
            S.finalizeProof(true);
            if (res != NULL) fprintf(res, "UNSAT\n"), fclose(res);
            if (S.verbosity > 0){
                printf("c ===============================================================================\n");
                printf("c Solved by unit propagation\n");
                S.printStats();
                printf("c \n"); }
            printf("s UNSATISFIABLE\n");
            exit(20);
        }

		fclose(res);



		// auto cnf_filename = argv[1]; // ./<enc>.cnf
		// argv[2] - out filename

		auto aig_filename = argv[3]; // ./<enc>.aig
        read_aig(aig_filename);

		// int nodes_cnt = atoi(argv[4]); // 7
		// int node_num = atoi(argv[5]); // 0-6
		// int threads_cnt = atoi(argv[6]); // 36
		// int thread_num = atoi(argv[7]); // 0-35
		// int tasks_cnt = threads_cnt * nodes_cnt;
		// int task_num = threads_cnt * node_num + thread_num;

		read_lin_table("lin-table", lin_table);

		lbool ret;

		for (auto &p: lin_table) {
			auto key = p.first;

			std::vector<std::vector<int>> res;
			for (auto &v: p.second) {
				int n = v.size();

				bool skip = 0;

				for (int i = 0; i < (1 << n); ++i) {
					int r = 0;
					std::vector<int> assumption;

					for (int j = 0; j < n; ++j) {
						int b = (i >> j) & 1;
						r ^= b;
						assumption.push_back(v[j] ^ b ^ 1);
					}

					if (r == 0)
						continue;

					vec <Lit> dummy;

					for (auto b: key) {
						int x = signed_half(output_vars[b.n] ^ b.bit ^ 1);

						Lit l = (x > 0 ? mkLit(x - 1) : ~mkLit(-x - 1));
						dummy.push(l);
					}

					for (auto x: assumption) {
						x = signed_half(x);
						Lit l = (x > 0 ? mkLit(x - 1) : ~mkLit(-x - 1));
						dummy.push(l);
					}

					ret = S.solveLimited(dummy);
					dummy.clear();

					if (ret == l_False)
						continue;

					skip = 1;
					break;
				}

				if (skip)
					continue;

				res.push_back(v);
			}

			if (res.empty())
				continue;

			std::clog << "# ";
			int bits = 0;
			for (auto &b: key) {
				std::clog << b.n << " ";

				bits <<= 1;
				bits ^= b.bit;
			}
			std::clog << bits << "\n";

			for (auto &v: res) {
				for (int x: v)
					std::clog << x << " ";
				std::clog << "\n";
			}
		}
    } catch (OutOfMemoryException&){
        printf("s UNKNOWN\n");
        exit(0);
    }
}
