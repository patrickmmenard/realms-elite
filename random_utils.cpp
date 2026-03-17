#include "random_utils.h"
#include <vector>
#include <string>
#include <utility>
#include <random>
using namespace std;

vector<string> govt_names = { "Republic", "Dictatorship", "Negan" };

string random_govt_type(const vector<string>& govt_names) {
	static mt19937 generator(random_device{}());
	uniform_int_distribution<int> dist(0, govt_names.size() - 1);
	int index = dist(generator);
	return govt_names[index];
}

pair<int, int> random_coord() {
	pair<int, int>p;
	do {
		p = { rand_int(-20, 20), rand_int(-20,20) };
	} while (p.first == 0 && p.second == 0);
	return p;
}

int rand_int(int min, int max) {
	static mt19937 gen(random_device{}());
	uniform_int_distribution<int> dist(min, max);
	return dist(gen);
}