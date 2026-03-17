
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <random>
#include <utility>
#include <cstdlib>

#include <chrono>
#include <thread>
#include <iostream>
#include <unordered_set>

#include "Enemy.h"
#include "random_utils.h"
//#include <function>
using namespace std;
using namespace std::chrono;

namespace colors {
	const string green =	"\033[32m";
	const string red =		"\033[31m";
	const string blue =		"\033[34m";
	const string blue_bg = "\033[44m";
	const string grey =		"\033[90m";
	const string color_reset = "\033[0m";
}

namespace terminal {
	const string clear = "\x1b[2J";
	const string home = "\x1b[H";
	const string clear_and_home = "\x1b[2J\x1b[H";
}

//besser waere:
//namespace terminal {
//	inline constexpr const char* clear = "\x1b[2J";
//	inline constexpr const char* home = "\x1b[H";
//	inline constexpr const char* clear_and_home = "\x1b[2J\x1b[H";
//}

struct Glyph {
	char symbol;
	const string& color;
	bool use_color = true;
};

const Glyph cap_player{ 'X', colors::color_reset };
const Glyph cap_dict{'D', colors::color_reset};
const Glyph cap_negan{'N', colors::color_reset};
const Glyph cap_repub{'R', colors::color_reset};
const Glyph occ_player{'C', colors::blue};
const Glyph empty_tile{'.', colors::green};
//const Glyph cap_enemy_1{ 'R' || 'N' || 'D', colors::red };
const Glyph occ_enemy_1{'1', colors::red};
const Glyph occ_enemy_2{'2', colors::red};
const Glyph road{'-', colors::grey};
const Glyph spacer{ ' ',"", false };
const Glyph water_tile{' ', colors::blue_bg};

bool flashing = false;

void draw(const Glyph& g) {
	if (flashing) {
		cout << colors::red << g.symbol << colors::color_reset; 
	}

	else if (g.use_color && !g.color.empty()){
		cout << g.color
		<< g.symbol
		<< colors::color_reset;
	}
	else { cout << g.symbol; }
	//return true;
}

//Patrick's little helpers!
void blank_lines(int n) {
	cout << string(n, '\n');
}

template <typename T>
string format_number(T n) {
	string s = to_string(n);
	int pos = s.length() - 3;
	while (pos > 0) {
		s.insert(pos, ",");
		pos -= 3;
	}
	return s;
}

int shooting_distance(const Enemy* enemy){
	int distance = abs(0 - enemy->get_coord().first) + 
					abs(0 - enemy->get_coord().second);
	return distance;
};


vector<string> attack_direction = {"north", "south", "east", "west"};

struct GovtInfo {
	char symbol;
	int tank_bonus;
	int jet_bonus;
};

map<string, GovtInfo> info =  //more to come...
{
		{"Republic",	{'R', 1, 0}},
		{"Dictatorship",{'D', 3, 0}},
		{"Negan",		{'N', 2, 1}},
};

string random_dir() {// WORKING HERE
	int randomDir = rand() % 4;
	return attack_direction[randomDir];
}

//bool e1_attack_neighbour(GameState_enemy& state, map<pair<int, int>, char>& tiles);

//int e1_coord_first;
//int e1_coord_second;
//int e2_coord_first;
//int e2_coord_second;
struct GameState_enemy;

//make a class later...


struct GameState_enemy {
	int year = 0;
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

	map<string, double> inventory = { //rounding rules later...
		{"Jets", 0.0},
		{"Tanks", 0.0},
		{"Troops", 0.0},
		{"Trucks", 0.0},
		{"Industrial Robots", 0.0},
		{"Cars", 0.0}
	};
};

struct PairHash{
	size_t operator()(const pair<int,int>& p) const {
		return hash<int>{}(p.first) ^ (hash<int>{}(p.second) << 1);
		}
};

struct GameState {
	int year = 0;
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

	map<string, double> inventory = { //rounding rules later...
		{"Jets", 0.0},
		{"Tanks", 0.0},
		{"Troops", 0.0},
		{"Trucks", 0.0},
		{"Industrial Robots", 0.0},
		{"Cars", 0.0}
	};
};

inline void set_tile(GameState& s, pair<int,int> pos, char c) {
	s.tiles[pos] = c;
	s.occupied.insert(pos);
}

inline void erase_tile(GameState& s, pair<int, int> pos) {
	s.tiles.erase(pos);
	s.occupied.erase(pos);
}

//
class Player {
public:
	int x = 0;
	int y = 0;
	long long money = 1000000;
	long long food = 10000000;
};
	
bool enemy_attack_once(GameState& state, Enemy* enemy, char sym) {

	auto [x,y] = enemy->get_coord();
	//vector<pair<int, int>> sources;

	for (int tries = 0; tries< 4; ++tries) {
		int nx = x;
		int ny = y;

		string randir = random_dir();

		if (randir == "east")		nx = x + 1;
		else if (randir == "west")	nx = x - 1;
		else if (randir == "north") ny = y + 1;
		else if (randir == "south") ny = y - 1;
		else continue;
		
		if (state.tiles.contains({ nx, ny })) {
			char t = state.tiles.at({ nx, ny });
			if (t == info[enemy->get_type()].symbol ||
				t == sym || t == 'L') continue;
		}
		
		for (auto& [pos,tile] : state.tiles) {
			if (tile != 'C') continue;

			int x = pos.first;
			int y = pos.second;

			if (!state.occupied.contains({x + 1, y}) ||
				!state.occupied.contains({x - 1, y}) ||
				!state.occupied.contains({x, y + 1}) ||
				!state.occupied.contains({ x, y - 1 })) {
				state.frontier.push_back(pos);
			}
		}

		pair<int, int> pos{ nx,ny };

		if (state.occupied.contains(pos)) continue;

		enemy->set_coord({nx, ny});

		set_tile(state,pos,sym);
		//enemy.set_coord({ nx, ny });
		//state.tiles[{nx, ny}] = sym;  // newly occupied territory receives its FLAG here. 
		
		return true;
	}
	return false;
};



struct Bounds{
	int max_x;
	int max_y;
	int min_x;
	int min_y;
};

int count_controlled_tiles(const GameState& state);

void show_status(const GameState& state) {
	cout << "Year: " << state.year << endl
		<< "Population: " << state.population << endl
		<< "Money: " << state.money << endl
		<< "Food: " << state.food << endl
		<< "Controlled Surface: " << count_controlled_tiles(state) << endl
		<< "Interest Rate: " << state.interest_rate << endl
		<< "Debt: " << state.debt << endl
		<< "New Debt: " << state.new_debt << endl;
}

map<string, double> prod_percentages = {
		{"Jets", 0.0},
		{"Tanks", 0.0},
		{"Troops", 0.0},
		{"Trucks", 0.0},
		{"Industrial Robots", 0.0},
		{"Cars", 0.0}
};

map<int, string> sys_menu = {
		{1, "Production"},
		{2, "Diplomacy"},
		{3, "War"},
		{4, "Bank"},
		{5, "Civilian Affairs"},
		{6, "End Year Turn"}, 
		{7, "Status"},
		{8, "Testing"},
		{999, "Quit"}
};

map<int, string> bank_menu = {
	{1, "Set Interest Rate"},
	{2, "Take a loan"},
	{3, "Invest"},
	{4, "Print money"},
	{5, "Buy Food"},
	{6, "Sell Food"}, 
	{9, "Return to System Menu"}
};

map<int, string> war_menu = {
		{1, "Attack"},
		{2, "Buy Tanks"},
		{3, "Buy Turrets"}, 
		{4, "Show current map"},
		{9, "Return to System Menu"}
};

map<int, string> test_menu = {
		{1, "End of Year"},
		//{2, "Diplomacy"},
		{2, "War"},
		{3, "Missile"},
		{4, "Gain Access to Water"},
		{5, "Flash Screen aka BOOM"},
		{9, "Quit"}
};

//unordered_map<string, function<void(GameState&)>> directionMap = {
//	{"east",  },
//	{"west", },
//	{"north", },
//	{"south", }
//};

void show_beta_banner() {
	cout << R"(
####################################
#      PATRICK REALMS ELITE        #
#                                  #
#              BETA                #
####################################
   
)";
}

int show_sys_menu(map<int, string> sys_menu) {
	int choice = 0;
	cout << "_____" << "System Menu" << "_____" << endl;
	for (auto [number, menu] : sys_menu) {
		cout << number << " " << menu << endl;
	}
	cin >> choice;
	return choice;
}

int show_bank_menu() {
	int choice = 0;
	cout << "_____" << "Bank Menu" << "_____" << endl;
	for (auto[number, item ] :bank_menu) {
		cout << number << " " << item << endl;
	}
	cin >> choice;
	return choice;
}

int show_war_menu() {
	int choice = 0;
	cout << "_____" << "War Menu" << "_____" << endl;
	for (auto [number, item]: war_menu) {
		cout << number << ": " << item << endl;
	}
	cin >> choice;
	return choice;
}

int show_test_menu(map<int, string> test_menu) {
	int choice = 0;
	cout << "_____" << "Test Menu" << "_____" << endl;
	for (auto [number, menu] : test_menu) {
		cout << number << " " << menu << endl;
	}
	cin >> choice;
	return choice;
}

void set_interest_rate(GameState& state) {
	//double interest_rate = 0.0;
	cout << " Interest Rate is: " << state.interest_rate << "%" << endl;
	cout << "Enter the new interest rate: " << endl;
	cin >> state.interest_rate;
	/*int choice = 9;
	return choice;*/  // die Funktion soll doch nur ein Double ODER ein int return!
}

void take_a_loan(GameState& state) {
	cout << "How much would you like to borrow?" << endl;
	cin >> state.new_debt;
	state.money += state.new_debt;
	state.debt += state.new_debt;
}

int show_prod_menu(map<int, string> prod_menu, GameState& state) {
	int choice = 0;
	// values need to be reset somewhere ... "what better place than here? what better time than now?"
	for (auto& [item, percentage]:prod_percentages) {
		percentage = 0.0;
	}
	cout << "_____" << "Production Menu" << "_____" << endl;
	for (auto [number, item] : prod_menu) {
		cout << number << " " << item << endl;
	}
	double input = 0.0;
	double total_percentage = 0.0;
	 
	cin >> choice; //this was missing.

	for (auto& [item, percentage] : prod_percentages) {

		double percentage_left = 100.0 - total_percentage;

		cout << item << "% (" << percentage_left << "% left): " << endl;
		cin >> input;
		state.prod_changed_this_year = true;
		if (input < 0.0) input = 0.0;
		if (input > percentage_left) input = percentage_left;

		percentage = input;
		total_percentage += input;

		if (total_percentage >= 100) {
			cout << "All production has been allocated for this year." << endl;
			cout << "0 to get back to the System Menu." << endl;
			cin >> choice;
			
			break;
		}
	}
	return choice;
}

void show_inventory(const map<string, double>& inventory );

int show_end_year_menu(GameState& state, const map<int, string>& prod_menu) { // WORKING HERE.
	char year_choice;
	double total_production = 0.0;
	//int choice = 0;
	cout << "_____" << "End Year Turn" << "_____" << endl;
	if (state.prod_changed_this_year == false) {
		char reset_prod;
		cout << "Production goals haven't changed this year; do you want to re-set production goals? y/n";
			cin >> reset_prod;
			if (reset_prod == 'y') {				
				show_prod_menu(prod_menu, state);
			}	
	}
	cout << "Do you want to end this year turn? (y/n)" << endl;
	cin >> year_choice;
	blank_lines(1);

	if (year_choice == 'y') {
		state.year += 1;
		cout << "Happy January 1st of year " << state.year << " !" << endl;
		cout << "Last year, your empire has produced: " << endl;

		for (auto [item,percentage]: prod_percentages) {
			cout << percentage << " " << item << endl;
			total_production += percentage;
			
		}
		
		if (total_production < 100) {
			cout << "(You should probably set production goals this year.)" << endl;
		}
		state.population = static_cast<long long>(state.population * 1.05); // growth to come here. 
		cout << "Population has grown to: " <<  format_number(state.population) << endl;
		state.money = static_cast<long long>(state.money * 1.00); //- ( state.debt/5);  // inflation to come here. 
		cout << "You now have " << format_number(state.money) << "$. " << endl;
		cout << "You now owe " << state.debt << "$." << endl;
		state.new_debt = 0;
		cout << "Your empire now controls: " << count_controlled_tiles(state) << " km^2 of land." << endl;
		blank_lines(2);
		for (auto& [item, units]: state.inventory) {
			units += state.capacity * (prod_percentages.at(item)/100);  //.at() to avoid creating garbage data; lets throw an exception instead. 
		}	
		show_inventory(state.inventory);
		state.prod_changed_this_year = false;
		blank_lines(2);
		return 1;
	}	
	else { return 0; }
}

Bounds normalize(Bounds b) {
	b.max_x = max(b.max_x, 1);
	b.max_y = max(b.max_y, 1);
	b.min_x = min(b.min_x, -1);
	b.min_y = min(b.min_y, -1);

	return b;
}
Bounds find_bounds(const GameState& state);
Bounds find_bounds_player(const GameState& state);

//void show_minimap(const map<pair<int, int>, char>& tiles, const Bounds& b) {
//	const int mini_w = 21;
//	const int mini_h = 21;
//
//	//Bounds world = find_bounds_player(tiles);
//	//Bounds world = find_player_bounds(tiles);
//
//	const Bounds& world = b;
//
//	int width = world.max_x - world.min_x  + 1;
//	int height = world.max_y - world.min_y + 1;
//
//	int span = max(width, height);
//	int k = (span + mini_w - 1) / mini_w; 
//	//if (k < 1) k = 1;
//	//k = max(1, min(k, 2));
//
//	for (int my = mini_h - 1; my >= 0; --my) {
//		for (int mx = 0; mx < mini_w; ++mx) {
//			int cap_x = 0;
//			int cap_y = 0; 
//
//			int half_w = mini_w / 2;
//			int half_h = mini_h / 2; 
//
//			int x0 = cap_x - half_w * k;
//			int y0 = cap_y - half_h * k;
//
//			int x_start = x0 + mx * k;
//			int y_start = y0 + my * k;
//
//			char out = '.';
//			
//			for (int dy = 0; dy < k && out == '.'; ++dy) {
//				for (int dx = 0; dx < k && out == '.'; ++dx) {
//					pair<int, int> p{ x_start + dx, y_start + dy};
//					if (tiles.contains(p)) out = tiles.at(p);
//				}
//			}
//			if (out == '.') cout << colors::green;
//			cout << "-" << out << " ";
//			if(out == '.') cout << colors::color_reset;
//			}
//		cout << "\n";
//	}
//}

void show_minimap(const GameState& state, const Bounds& b) {
	blank_lines(2);

	Bounds world = find_bounds(state);
	
	for (int y = world.max_y; y >= world.min_y; --y) {
		for (int x = world.min_x; x <= world.max_x; ++x) {
			pair<int, int> pos{ x,y };

			if (state.tiles.contains(pos)) {
				char t = state.tiles.at(pos);
				//draw(spacer);
				if (t == 'X') {
					draw(cap_player);
					draw(spacer);
				}
				else if (t == 'R') {
					draw(cap_repub);
					draw(spacer);
				}
				else if (t == 'D') {
					draw(cap_dict);
					draw(spacer);
				}
				else if (t == 'N') {
					draw(cap_negan);
					draw(spacer);
				}
				else if (t == 'C') {
					draw(occ_player);
					draw(road);
				}
				else if (t == '1') {
					draw(occ_enemy_1);
					draw(road);
				}
				else if (t == '2') {
					draw(occ_enemy_2);
					draw(road);
				}
				else if (t == 'L') {
					/*for (int i = 0; i < 5; i++) {
						draw(water_tile);
					}*/
					draw(water_tile);
					draw(water_tile);
				}
				else cout << t;
				//draw(spacer);
			}
			else {
				//draw(spacer);
				draw(empty_tile);
				draw(spacer);
				//cout << colors::green << " . " << colors::color_reset;
			}
			
		}
		cout << endl;
	}
	
	blank_lines(2);
}


void repeat_end_year(GameState& state, const map<int, string>& prod_menu) { // WORKING HERE.
	int years = 0;
	cout << "How many years/turns at once?" << endl;
	cin >> years;
	for (int i = 0; i < years; ++i) {
		show_end_year_menu(state, prod_menu);
	}
};

bool attack_neighbour(GameState& state);

bool attack_once(GameState& state) {
	auto b = find_bounds(state);
	
	if (state.direction == "east")		state.tiles[{b.max_x + 1, 0}] = 'C';
	else if (state.direction == "west") state.tiles[{b.min_x - 1, 0}] = 'C';
	else if (state.direction == "north")state.tiles[{0, b.max_y + 1}] = 'C';
	else if (state.direction == "south")state.tiles[{0, b.min_y - 1}] = 'C';
	else return false;

	return true;
};

bool attack_neighbour_dir(GameState& state, Player* player,  string dir, Enemy* e1, Enemy* e2);

//int count_controlled_tiles(const GameState& state);
struct EnemyCounts {
	int e1;
	int e2;
};
EnemyCounts count_enemy_tiles(const GameState& state);

void repeat_attack(GameState& state, Player* player, Enemy* e1, Enemy* e2) {
	int years = 0;
	
	cout << "How many years/turns?" << endl;
	cin >> years;
	cout << "Direction? north/south/east/west/random/access to water" << endl;
	cin >> state.direction;
	
	for (int i = 0; i < years; ++i) {
		string dir = state.direction;
		if (dir == "random") {
			dir = random_dir();
			attack_neighbour_dir(state, player, dir, e1, e2);
		}
		/*else if (dir == "access to water") {
			
		}*/
		else { 
			attack_neighbour_dir(state,player, dir, e1, e2); }
	}
	cout << "You now control " << count_controlled_tiles(state) << " territories/ km^2." << endl;
	cout << "Enemy 1 now controls " << count_enemy_tiles(state).e1 << " territories/ km^2." << endl;
	cout << "Enemy 2 now controls " << count_enemy_tiles(state).e2 << " territories/ km^2." << endl;
	cout << "The distance to Enemy 1 is: " << shooting_distance(e1) << endl;
};

//void repeat_attack(GameState& state, map<pair<int, int>, char>& tiles) {
//	int years = 0;
//	state.direction = "north";
//	cout << "For how many years/turns do you want to attack in a row?" << endl;
//	cin >> years;
//	
//	cout << "In what direction? north/south/east/west" << endl;
//	cin >> state.direction;
//	for (int i = 0; i < years; ++i) {
//		state.state_of_war = "yes";
//		attack_neighbour(state, tiles);
//	};

//};

void show_map(const GameState& state,const Bounds& b) { 
	blank_lines(2);

	for (int y = b.max_y; y >= b.min_y; --y) {
		for (int x = b.min_x; x <= b.max_x; ++x) {
			pair<int, int> pos{x,y};
		
			if (state.tiles.contains(pos)) {
				cout << " " << state.tiles.at(pos) << " ";
			}
			else {
				cout << " . ";
			}
		}
		cout << endl;
	}
	blank_lines(2);
}

void show_inventory(const map<string,double >& inventory ) { 
	for (auto [item, units]:inventory) {
		cout << item << " : " << units << endl;
	}
}

Bounds find_bounds(const GameState& state) {
	int max_x = 0;
	int max_y = 0;
	int min_x = 0;
	int min_y = 0;
	
	for (auto [pos,tile]: state.tiles) {
		
		max_x = max(max_x, pos.first);
		max_y = max(max_y, pos.second);
		min_x = min(min_x, pos.first);
		min_y = min(min_y, pos.second);
	}
	return Bounds{max_x+10, max_y+10, min_x-10, min_y-10};
};

Bounds find_bounds_player(const GameState& state) {
	bool first = true;
	Bounds b{0,0,0,0};

	for (auto& [pos, tile]: state.tiles) {
		if (tile != 'X' && tile != 'C') continue;

		if (first) {
			b = { pos.first, pos.second, pos.first, pos.second };
			first = false;
		}
		else {
			b.max_x = max(b.max_x, pos.first);
			b.max_y = max(b.max_y, pos.second);
			b.min_x = min(b.min_x, pos.first);
			b.min_y = min(b.min_y, pos.second);
		}
	}
	if (first) return Bounds{0,0,0,0};
	return b; 
}

EnemyCounts count_enemy_tiles(const GameState& state);
void flash_screen(const GameState& state, Bounds b);

bool attack_neighbour_dir(GameState& state, Player* player,string dir,Enemy* e1, Enemy* e2) {

	bool success = false;
	auto b_before = find_bounds_player(state);
	char old_tile = 'w';
	int sx, sy;

	
		if (state.frontier.empty()) {
			sx = player->x;
			sy = player->y;
		}
		else {
			int i = rand() % state.frontier.size();
			sx = state.frontier[i].first;
			sy = state.frontier[i].second;
		}

		int nx = sx;
		int ny = sy;

		int attempts = 0;

		while (!success && attempts < 50) {

			attempts++;
			old_tile = 'w';			// old_tile reset to avoid that a value stays stale across iterations. 
			nx = sx;				//same thinking.
			ny = sy;

			if (dir == "east")			nx = sx + 1;
			else if (dir == "west")		nx = sx - 1;
			else if (dir == "north")	ny = sy + 1;
			else if (dir == "south")	ny = sy - 1;

			if (state.tiles.contains({ nx,ny })) {
				old_tile = state.tiles.at({ nx,ny });
			}

			if (state.occupied.contains({ nx,ny }) && // if already occupied
				!(old_tile == '1' || old_tile == '2') // and not an enemy tile
				) {
				dir = random_dir();					//roll the dice again. 
				continue;							//restart loop.
			}

			if (!(nx == player->x && ny == player->y)) {
				if (old_tile == '1' || old_tile == '2') {
					flash_screen(state, b_before);
				}
				set_tile(state, { nx,ny }, 'C');
				success = true;
			}
	}

		state.frontier.clear();

		for (auto& [pos, tile] : state.tiles) {  //in all tiles
			if (tile != 'C' && tile != 'X') continue; // if not a player's tile

			int x = pos.first;
			int y = pos.second;

			if (!state.occupied.contains({ x + 1, y }) || //check if frontier
				!state.occupied.contains({ x - 1, y }) ||
				!state.occupied.contains({ x, y + 1 }) ||
				!state.occupied.contains({ x, y - 1 })) {
				state.frontier.push_back(pos);            //add to state.frontier
			}
		}

	auto b_after = find_bounds_player(state);

	cout << terminal::clear_and_home;
	show_minimap(state, b_after);

	/*cout << "sx: " << sx << " sy: " << sy << endl;
	cout << "nx: " << nx << " ny: " << ny << endl;*/

	cout << "You now control " << count_controlled_tiles(state) << " territories/ km^2." << endl;
	/*cout << "Enemy 1 now controls " << count_enemy_tiles(state).e1 << " territories/ km^2." << endl;
	cout << "Enemy 2 now controls " << count_enemy_tiles(state).e2 << " territories/ km^2." << endl;
	cout << "The distance to Enemy 1 is: " << shooting_distance(e1) << endl;*/
	this_thread::sleep_for(milliseconds(20));
	cout << flush;
	return success;
}

bool attack_neighbour(GameState& state,
	Player* player,
	Enemy* e1, Enemy* e2) {
	cout << "Do you wish to attack a neighbor? ('yes' or 'no') If yes, in which direction? 'north/south/west/east' ?" << endl;
	cin >> state.state_of_war >> state.direction;

	if (state.state_of_war != "yes") return false; 

	return attack_neighbour_dir(state, player, state.direction, e1, e2);
	} 

int count_controlled_tiles(const GameState& state) {
	int count = 0;
	for (auto [pos,tile]: state.tiles) {
		if (tile == 'C') {
			count += 1;
		}
	}
	return count;
};

EnemyCounts count_enemy_tiles(const GameState& state) {
	EnemyCounts counts{0,0};
	for (auto [pos, tile] : state.tiles) {
		if (tile == '1' ) {
			counts.e1 += 1;
		}
		else if (tile == '2') {
			counts.e2 += 1;
		}
	}
	return counts;
}
void flash_screen(const GameState& state, Bounds b);

void launch_missile(GameState& state, pair<int, int> start,pair<int, int> target,Bounds b) {
	int x = start.first;
	int y = start.second;

	while (x != target.first || y != target.second) {
		if (x < target.first) ++x;
		else if (x > target.first) --x;
		else if (y < target.second) ++y;
		else if (y > target.second) --y;

		pair<int, int> pos{ x,y };

		//char old = tiles.contains(pos) ? tiles.at(pos) : '.';

		char old;
		if (state.tiles.contains(pos)) {
			old = state.tiles.at(pos);
		}
		else { old = '.'; };

		state.tiles[pos] = '*';
		show_minimap(state, b);
		cout << terminal::clear_and_home;

		this_thread::sleep_for(milliseconds(50));

		if (old == '.') state.tiles.erase(pos);
		else state.tiles[pos] = old;
	}
		
		cout << "HIT!" << endl;
		flash_screen(state, b);
		}

void access_water(pair<int, int> start,pair<int, int> target,GameState& state, Enemy* e1, Enemy* e2) {
	int x = start.first;
	int y = start.second;

	auto b_before = find_bounds_player(state);

	while (x != target.first || y != target.second) {
		if (x < target.first) ++x;
		else if (x > target.first) --x;
		else if (y < target.second) ++y;
		else if (y > target.second) --y;

		//pair<int, int> pos{ x,y };

		state.tiles[{x, y}] = 'C';
		state.current_x = x;
		state.current_y = y;

		enemy_attack_once(state, e1, '1');  //occupied territories are given their "flags" in enemy_attack_once; given as parameter here. 
		enemy_attack_once(state, e2, '2');

		auto b_after = find_bounds_player(state);

		cout << terminal::clear_and_home;
		show_minimap(state, b_after);
		this_thread::sleep_for(milliseconds(20));
		cout << flush;
		/*cout << "You now control " << count_controlled_tiles(state) << " territories/ km^2." << endl;
		cout << "Enemy 1 now controls " << count_enemy_tiles(state).e1 << " territories/ km^2." << endl;
		cout << "Enemy 2 now controls " << count_enemy_tiles(state).e2 << " territories/ km^2." << endl;
		cout << "The distance to Enemy 1 is: " << shooting_distance(e1) << endl;*/
		//return true;
	}

	cout << "WATER!" << endl;
}
//pair<int, int> lake_seed = random_coord();

//map<pair<int, int>, char> tiles;
pair<int, int> lake_seed = random_coord();

void basic_lake(GameState& state, pair<int, int>lake_seed, int number) {
	pair<int,int> l = lake_seed;
	for (int dx = 0; dx < number; dx++) {
		for (int dy = 0; dy < number; dy++) {
			//state.tiles[{l.first + dx, l.second - dy }] = 'L';
			set_tile(state, { l.first + dx, l.second - dy }, 'L');
		}
	}
}

void flash_screen(const GameState& state, Bounds b ) {
	
	for (int tries = 0; tries < 6; tries++) {
		cout << terminal::clear_and_home;
		flashing = true;
		show_minimap(state, b);
		flashing = false;
		cout << std::flush;
		this_thread::sleep_for(milliseconds(10));
		
		cout << terminal::clear_and_home;
		flashing = false;
		show_minimap(state, b);
		cout << std::flush;
		this_thread::sleep_for(milliseconds(10));
	}
}

int main()
{
	srand(time(nullptr));  //avoids making the same "random" path every time.

	GameState state;
	Player* player = new Player();  // "()" means constructor 

	//Enemy e1;
	Enemy* e1 = new Enemy(); 
	cout << "First enemy govt type: " << e1->get_type() << endl;
	cout << "Enemy capital coord: "
		<< e1->get_coord().first << "," << e1->get_coord().second << endl;
	cout << "e1's first attack should be to the: " << random_dir() << endl;
	blank_lines(2);

	//Enemy e2;
	Enemy* e2 = new Enemy();
	cout << "Second enemy govt type: " << e2->get_type() << endl;
	cout << "Second enemy coord: "
		<< e2->get_coord().first << "," << e2->get_coord().second << endl; 
	cout << "e2's first attack should be to the: " << random_dir() << endl;

	int sys_choice = 0;
	cout << "\033[32m";
	show_beta_banner();
	cout << "\033[0m";

	//tiles!
	//map<pair<int, int>, char> tiles; 
	state.tiles[{0, 0}] = 'X';
	//working here for a while...
	int e1_coord_first = e1->get_coord().first;
	int e1_coord_second = e1->get_coord().second;

	state.tiles[{e1_coord_first, e1_coord_second}] = info[e1->get_type()].symbol;
	//this is where the govt type is written, not the '1' or '2' of occcupied tiles. 

	int e2_coord_first = e2->get_coord().first;
	int e2_coord_second = e2->get_coord().second;

	state.tiles[{e2_coord_first, e2_coord_second}] = info[e2->get_type()].symbol;
	//same; govt type of the capital, not occupied tiles. 

	map<pair<int, int>, char> roads;
	
	/*tiles[{lake_seed.first, lake_seed.second}] = 'L';
	tiles[{(lake_seed.first) + 1, lake_seed.second}] = 'L';*/
	
	//basic_lake(state, lake_seed, 7);

	int bank_choice = 0;

	//GameState state;

	map<int, string> prod_menu = {
		{1, "Jets"},
		{2, "Tanks"},
		{3, "Troops"},
		{4, "Trucks"},
		{5, "Industrial Robots"},
		{6, "Cars" },
		{99, "Set current year production"}
	};

	while (sys_choice != 999) {
		auto bounds = normalize(find_bounds(state));
		//show_map(tiles, bounds);
		show_minimap(state, find_bounds_player(state));
		sys_choice = show_sys_menu(sys_menu);
		blank_lines(2);

		switch (sys_choice) {
		case 1:show_map(state, bounds);
			show_prod_menu(prod_menu, state);
			blank_lines(2);
			break;
		case 3:
		{
			int war_choice = 0;

			while (war_choice != 9) {
				//show_map(tiles, normalize(bounds));
				war_choice = show_war_menu();
				switch (war_choice) {
				case 1:
				{
					attack_neighbour(state, player, e1, e2);
					/*enemy_attack_once(tiles, e1);
					enemy_attack_once(tiles, e2);*/
					break;
				}
					/*case 2:
						break;
					case 3:
						break;*/
				case 4: 
				{auto bounds = find_bounds(state);
				show_map(state, bounds); }
					break;
					}
				}
			}
			break;
		
		case 4:
		{
			int bank_choice = 0;
			while (bank_choice != 9) {
				bank_choice = show_bank_menu();
				switch (bank_choice) {
				case 1:
					set_interest_rate(state);
					cout << "New interest rate: " << state.interest_rate << "%" << endl;
					break;
				case 2:
					take_a_loan(state);
					cout << state.new_debt << " $ have been added to your account. Annual payments will start at the end of the current year." << endl;
					break;
					//	case 3:
					//	{ 3, "Invest" }
					//	break;
					//	case 4:
					//	{ 4, "Print money" }
					//	break;
					//	case 5:
					//	{ 5, "Buy Food" }
					//	break;
					//	case 6:
					//	{ 6, "Sell Food" }
					//	break;
					//	case 9:
					//	{ 9, "Return to System Menu" }
					//	break;

				case 9:
					break;
					blank_lines(2);
				}
			}
			break;
		}
		case 6: { show_end_year_menu(state, prod_menu); }
			break;
		case 7:
			show_status(state);
			show_map(state, normalize(bounds));
			blank_lines(2);
			break;

		case 8: 
		{
			int test_choice = 0;

			while (test_choice != 9) {
				test_choice = show_test_menu(test_menu);  //working here. 
				
				switch (test_choice) {
				case 1:
					repeat_end_year(state, prod_menu);
					break;

				case 2:
					repeat_attack(state, player, e1, e2);
					break;

				case 3:
				{
					Bounds world = find_bounds(state);
					pair<int, int> start = { 0,0 };
					pair<int, int> target = { e1->get_coord().first, e1->get_coord().second };

					launch_missile(state, start, target, world);
					break;
				}
				case 4:
				{	
					Bounds world = find_bounds(state);
					pair<int, int> start = {state.current_x, state.current_y};
					pair<int, int> target = {lake_seed.first, lake_seed.second};
					access_water(start, target, state, e1, e2);
					break;
				}
				case 5:
				{
					Bounds b = find_bounds(state);
					flash_screen(state, b);
					break;
				}

				case 9:
					break;
				}
			}
			break;
		}
		case 999: break;
		}
	}
}
	
	


