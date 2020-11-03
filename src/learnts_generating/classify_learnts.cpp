#include <bits/stdc++.h>

#define all(x) (x).begin(), (x).end()

typedef unsigned int uint;

using namespace std;


void classify(const string &filename)
{
	map <vector <int>, set <vector <int>>> learnts;

	ifstream fin(filename.data());
	vector <int> clause;
	vector <int> clause_abs;
	int x;
	while (fin >> x) {
		if (x == 0) {
			sort(all(clause), [](int a, int b) {
				return abs(a) < abs(b) || (abs(a) == abs(b) && a < b);
			});
			clause.resize(unique(all(clause)) - clause.begin());
			sort(all(clause_abs));
			clause_abs.resize(unique(all(clause_abs)) - clause_abs.begin());
			if (clause.size() == clause_abs.size())
				learnts[clause_abs].insert(clause);
			clause.clear();
			clause_abs.clear();
		}
		else {
			clause_abs.push_back(abs(x));
			clause.push_back(x);
		}
	}
	fin.close();
/*
	for (auto &p: learnts) {
		for (auto &v: p.second) {
			for (auto x: v)
				cout << x << " ";
			cout << "\n";
		}
		cout << "\n";
	}
	cout << "!" << endl;
*/
	vector <int> counter2(4, 0), counter3(8, 0);
	set <set <vector <int>>> answer;
	for (auto &p: learnts) {
		if (p.first.size() == 2) {
			counter2[4 - p.second.size()]++;
		}
		else if (p.first.size() == 3) {
			auto learnts_set = p.second;
			vector <int> vars = p.first;
			int x = vars[0], y = vars[1], z = vars[2];
			// cout << x << " " << y << " " << z << "\n";
			auto it = learnts.find(vector <int> ({x, y}));
			if (it != learnts.end()) {
				for (auto &v: it -> second) {
					vector <int> learnt;
					learnt = {v[0], v[1], z};
					learnts_set.insert(learnt);
					learnt = {v[0], v[1], -z};
					learnts_set.insert(learnt);
				}
			}
			it = learnts.find(vector <int> ({x, z}));
			if (it != learnts.end()) {
				for (auto &v: it -> second) {
					vector <int> learnt;
					learnt = {v[0], y, v[1]};
					learnts_set.insert(learnt);
					learnt = {v[0], -y, v[1]};
					learnts_set.insert(learnt);
				}
			}
			it = learnts.find(vector <int> ({y, z}));
			if (it != learnts.end()) {
				for (auto &v: it -> second) {
					vector <int> learnt;
					learnt = {x, v[0], v[1]};
					learnts_set.insert(learnt);
					learnt = {-x, v[0], v[1]};
					learnts_set.insert(learnt);
				}
			}
			answer.insert(learnts_set);
			counter3[8 - learnts_set.size()]++;
		}
		else {
			continue;
		}
	}

	ofstream fout("statistics");
	for (uint i = 0; i < counter2.size(); ++i)
		fout << counter2[i] << " ";
	fout << endl;
	for (uint i = 0; i < counter3.size(); ++i)
		fout << counter3[i] << " ";
	fout << endl;
	fout.close();

	fout.open("learnts_by_key");
	for (auto &learnts_set: answer) {
		if (learnts_set.size() < 4)
			continue;
		fout << learnts_set.size() << "\n";
		for (auto &learnt: learnts_set) {
			for (auto var: learnt)
				fout << (var > 0 ? 2 * var : - 2 * var + 1) << " ";
			fout << "\n";
		}
		fout << "\n";
	}
	fout.close();
}


int main(int argc, char *argv[]) {

	classify("learnts"); // learnts in DIMACS format

}
