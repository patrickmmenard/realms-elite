#include "Enemy.h"
#include "random_utils.h"
using namespace std;

	Enemy:: Enemy() {
		type = random_govt_type(govt_names);
		capital = random_coord();
		coord = capital;
	}
	string Enemy::get_type() const {
		return type;
	}

	pair<int, int> Enemy::get_coord() const { return coord; }
	void Enemy::set_coord(pair<int, int> p) { coord = p; }

	pair<int, int> Enemy::get_capital() const { return capital; }

