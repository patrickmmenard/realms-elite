#pragma once
#include <vector>
#include <string>
#include <utility>

extern std::vector<std::string> govt_names;
std::string random_govt_type(const std::vector<std::string>& govt_names);
std::pair<int, int> random_coord();

int rand_int(int min, int max);
