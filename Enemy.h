#pragma once
#include <string>
#include <utility>


// enemy.h, member functions and variables declared here

class Enemy {
public:
	Enemy();

	std::string get_type() const;
	std::pair<int, int> get_coord() const;
	void set_coord(std::pair<int, int> p);
	std::pair<int, int> get_capital() const;

	int e1_coord_first;
	int e1_coord_second; 
	int e2_coord_first;
	int e2_coord_second;

private:
	std::string type;
	std::pair<int, int> capital;
	std::pair<int, int> coord;
};