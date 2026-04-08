#pragma once

struct GameState {
	
	long long population = 10000000;
	long long money = 1000000;
	long long food = 10000000;
	int controlled_surface = 1;
	double interest_rate = 0.0;
	long long debt = 0;
	long long new_debt = 0;
	string state_of_war;
	string direction;
	bool prod_changed_this_year = false;
	double capacity = 100.0;
	int current_x = 0;
	int current_y = 0;

	map<pair<int, int>, char> tiles;
	unordered_set <pair<int, int>, PairHash> occupied;
	vector<pair<int, int>> frontier;
	vector<pair<int, int>> frontier_e1;
	vector<pair<int, int>> frontier_e2;
	
	map<string, double> inventory = { //rounding rules later...
		{"Jets", 0.0},
		{"Tanks", 0.0},
		{"Troops", 0.0},
		{"Trucks", 0.0},
		{"Industrial Robots", 0.0},
		{"Cars", 0.0}
	};

	bool gameover = 0;
	bool victory = 0;
	int year = 0;
	int day = 0;

	bool tank_active = 0;
	int tank_x = 1;
	int tank_y = 0;
	string tank_dir;
	char reset_answer = 'y';

	bool jet_active = 1;
	int jet_x = -20;
	int jet_y = 20;
	int jet_target = jet_x;
	int jet_dir = 1;

	bool bomb_is_flying = false; 
	int bomb_start_x;
	int bomb_start_y;
	int bomb_target_x;
	int bomb_target_y;
	int bomb_x;
	int bomb_y;
	int bomb_dir;

	Bounds world;				//yes, a struct inside a struct...
	bool pause = false;
};
