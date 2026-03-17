#pragma once
#include <string>
#include <utility>

class Enemy {
public:
	Enemy();

	std::string get_type() const;
	std::pair<int, int> get_coord() const;
	void set_coord(std::pair<int, int> p);
	std::pair<int, int> get_capital() const;


private:
	std::string type;
	std::pair<int, int> capital;
	std::pair<int, int> coord;
};