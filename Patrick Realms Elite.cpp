
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
#include <windows.h>
#include <cmath>

#include <conio.h>

//#include <function>
using namespace std;
using namespace std::chrono;

namespace colors {
	const string green =	"\033[32m";
	const string red =		"\033[31m";
	const string red_bg =	"\033[41m";

	const string blue =		"\033[34m";
	const string light_blue = "\033[94m";
	const string blue_bg =		"\033[44m";

	const string grey =			"\033[90m";
	const string none =			"";
	const string white_bg =		"\033[47m";

	const string purple_bg =	"\033[45m";
	const string bright_purple_bg = "\033[105m";
	const string violet_bg = "\033[48;5;54m";

	const string green_bg = "\033[48;5;58m";
	const string orange = "\033[38;5;208m9"; 
	const string orange_bg = "\033[48;5;208m";

	const string color_reset = "\033[0m";
}

namespace terminal {
	const string clear = "\x1b[2J";
	const string home =	 "\x1b[H";
	const string clear_and_home = "\x1b[2J\x1b[H";
}

//besser waere:
//namespace terminal {
//	inline constexpr const char* clear = "\x1b[2J";
//	inline constexpr const char* home = "\x1b[H";
//	inline constexpr const char* clear_and_home = "\x1b[2J\x1b[H";
//}

struct Glyph {
	string symbol;
	string color;
	string background = "";
	bool use_color = true;
};

const Glyph cap_player{ "X", colors::color_reset, colors::none };
const Glyph cap_dict{ "D", colors::color_reset, colors::none };
const Glyph cap_negan{ "N", colors::color_reset, colors::none };
const Glyph cap_repub{ "R", colors::color_reset, colors::none };
const Glyph cap_blackadder{"B", colors::color_reset, colors::none};
const Glyph occ_player{ "C", colors::blue, colors::none };

const Glyph empty_tile{ ".", colors::green, colors::none };
//const Glyph cap_enemy_1{ 'R' || 'N' || 'D', colors::red };
const Glyph occ_enemy_1{ " ", colors::none, colors::red_bg };
const Glyph occ_enemy_texture{ "3", colors::grey, colors::red_bg};
const Glyph occ_enemy_2{ "#", colors::grey, colors::green_bg };
const Glyph occ_enemy_texture2{ "3", colors::none, colors::green_bg };

const Glyph road{"-", colors::grey, colors::none };
const Glyph spacer{ " ",colors::none, colors::none, false };
const Glyph water_tile{ " ","", colors::blue_bg };
const Glyph water_wave{ "~", colors::light_blue, colors::blue_bg};

const Glyph player_tile{" ", colors::none, colors::violet_bg};
const Glyph player_texture{".", colors::grey, colors::violet_bg};
const Glyph explosion_tile{" ", colors::none, colors::white_bg};
const Glyph explosion_tile_orange{" ", colors::none, colors::orange_bg};
const Glyph bomb{ "*", colors::grey, colors::none};

vector<string> tank_art = {
	".||= ",
	"OOOO"
};

vector<string> jet_art = {
	"--o-(_)-o--"
};

string frame;

bool flashing = false;

void draw(const Glyph& g) {
	if (flashing) {
		cout << g.background
			 << colors::red 
			 << g.symbol 
			 << colors::color_reset; 
	}
	else if (g.use_color){
		cout << g.background
		     << g.color
			 << g.symbol
			 << colors::color_reset;
	}
	else { cout << g.symbol; }
}

void append_draw(string& frame, const Glyph& g) {
	if (flashing) {
		frame += g.background;
		frame += colors::red;
		frame += g.symbol;
		frame += colors::color_reset;
	}
	else if (g.use_color) {
		frame += g.background;
		frame += g.color;
		frame += g.symbol;
		frame += colors::color_reset;
	}
	else { frame += g.symbol; }
}

//void draw_tank(int cx, int cy) {
//	for (int row = 0; row < tank_art.size(); ++row) {
//		for (int col = 0; col < tank_art[row].size(); ++col) {
//
//			char c = tank_art[row][col];
//			if (c == ' ') continue;
//
//			int x = cx + col;
//			int y = cy - row;
//		
//		}
//	}
//}

char tank_char_at(int x, int y, int cx, int cy) {
	int local_col = x - cx;
	int local_row = cy - y;

	if (local_row < 0 || local_row >= tank_art.size()) return ' ';
	if (local_col < 0 || local_col >= tank_art[local_row].size()) return ' ';

	return tank_art[local_row][local_col];

}

char unit_char_at(int x, int y, int cx, int cy, const vector<string>& art) {		//generalized version. 
	int local_col = x - cx;
	int local_row = cy - y;

	if (local_row < 0 || local_row >= art.size()) return ' ';
	if (local_col < 0 || local_col >= art[local_row].size()) return ' ';

	return art[local_row][local_col];

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
		{"Republic",		{'R', 1, 0}},			//attack,defense bonuses...
		{"Dictatorship",	{'D', 3, 0}},
		{"Negan",			{'N', 2, 1}},
		{"Cmd Blackadder",	{'B', 0, 3}}
};

string random_dir() {
	int randomDir = rand() % 4;
	return attack_direction[randomDir];
};

vector<string> messages = {
	"Victory!",
	"Game Over!",
	"We are under attack!",
	"Crush'em!",
	"Easy peasy, lemon squeezy!",
	"Year 3"
};

vector <string> box_lines = {
	"===============",
	"=             =",
	"=             =",
	"=             =",
	"=             =",
	"=             =",
	"==============="
};



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

struct Bounds {
	int max_x;
	int max_y;
	int min_x;
	int min_y;
};

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

void update_msg_box(const GameState& state){
	box_lines[0] = "===============";

	string state_year = to_string(state.year);
	string show_year = "= Year:" + state_year + "  =";
	box_lines[4] = show_year;

	string state_day = to_string(state.day);
	string show_day = "= Day:" + state_day + "  =";
	box_lines[5] = show_day;

	if (state.victory == 1) {
		box_lines[1] =	"= Victory!    =";
		box_lines[2] =	"=             ="; 
	}
	else if (state.gameover == 1) {
		box_lines[1] = "= Game Over!  =";
		box_lines[2] = "=             =";
	}
	else {
		box_lines[1] = "=             =";
		box_lines[2] = "=             =";
	}
}

string msg_box_row(int row) {
	if (int i = row - 10; i >= 0 && i < box_lines.size()) {
		return box_lines[i];
	}
	return string(15, ' ');
};

const vector < pair<int, int>> dirs = { {1,0}, {-1,0}, {0,1}, {0,-1} };
void make_explosions(GameState& state, string frame, int number);

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
	
bool enemy_attack_once(GameState& state, Enemy* enemy, char sym) {			// DEPRECATED 

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


Bounds find_bounds_player(const GameState& state);

bool enemy_single_attack(GameState& state, string dir, Enemy* e1, int e1x, int e1y) {  //new version of enemy_attack_once(); based on functional (92% acquisition) attack_neighbour_dir(renamed)
																					   //basis for enemy2; avoid hardcoded stuff in next version!
	bool success = false;
	auto b_before = find_bounds_player(state);
	char old_tile = 'w';
	int sx, sy;
	dir = random_dir();			//avoids mimicking Player's direction.
	
	if (state.frontier_e1.empty()) {
		sx = e1->get_coord().first;
		sy = e1->get_coord().second;
	}
	else {
		int i = rand() % state.frontier_e1.size();
		sx = state.frontier_e1[i].first;
		sy = state.frontier_e1[i].second;
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

		if (nx == 0 && ny == 0) {				//if Enemy1 takes Player.
			set_tile(state, { nx, ny }, '1');
			state.gameover = 1;
				for (auto& [pos, tile] : state.tiles) {
					if (tile == 'C') {
						tile = '1';
					}
				}
			success = true;
			return true;
		}
		else if ((old_tile == 'C' || old_tile == '2') && !(nx == state.tank_x && ny == state.tank_y)) {
			//make_explosions(state, 5);
			//flash_screen(state, b_before);
			
			set_tile(state, { nx,ny }, '1');
			success = true;
		}

		else if (state.occupied.contains({ nx,ny }) &&  // if already occupied
			!(old_tile == 'C' || old_tile == '2') ||	// and not an enemy tile
			(nx == state.tank_x && ny == state.tank_y)) // and not Tank.
			{
			//dir = random_dir();	
			if (state.frontier_e1.empty()) {
				sx = e1->get_coord().first;
				sy = e1->get_coord().second;
			}
			else {
				int i = rand() % state.frontier_e1.size();
				sx = state.frontier_e1[i].first;
				sy = state.frontier_e1[i].second;
			}
			continue;							//restart loop.
			}

	}

	if (!(old_tile == 'C' || old_tile == '2') && !(old_tile == '1') && !(old_tile == info[e1->get_type()].symbol) &&
		!(nx == state.tank_x && ny == state.tank_y)
		) {
		set_tile(state, { nx,ny }, '1');
	}

	state.frontier_e1.clear();

	//char t = info[e1->get_type()].symbol;
	for (auto& [pos, tile] : state.tiles) {  //in all tiles
		bool is_capital = (pos.first == e1x && pos.second == e1y);
		
		if (tile != '1' && !is_capital) continue; // if not this enemy's tile

		int x = pos.first;
		int y = pos.second;

		for (const auto&[dx, dy]: dirs) {
			int nx = x + dx;
			int ny = y + dy;

			if (!state.occupied.contains({nx, ny})) {
				state.frontier_e1.push_back(pos);
				break;
			}

			char neighbour_tile = state.tiles.at({nx, ny});

			{
				if (neighbour_tile != '1' && neighbour_tile != is_capital) {
				}
				state.frontier_e1.push_back(pos);
				break;
			}
		}
	}
	return success;
}

bool enemy2_single_attack(GameState& state, string dir, Enemy* e2, int e2x, int e2y) {  //new version of enemy_attack_once(); based on functional (92% acquisition) attack_neighbour_dir

	bool success = false;
	auto b_before = find_bounds_player(state);
	char old_tile = 'w';
	int sx, sy;
	dir = random_dir();

	if (state.frontier_e2.empty()) {
		sx = e2->get_coord().first;
		sy = e2->get_coord().second;
	}
	else {
		int i = rand() % state.frontier_e2.size();
		sx = state.frontier_e2[i].first;
		sy = state.frontier_e2[i].second;
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

		if (nx == 0 && ny == 0) {				//if Enemy1 takes Player.
			set_tile(state, { nx, ny }, '2');
			state.gameover = 1;
			for (auto& [pos, tile] : state.tiles) {
				if (tile == 'C') {
					tile = '2';
				}
			}
			success = true;
			return true;
		}
		else if ((old_tile == 'C' || old_tile == '1') && !(nx == state.tank_x && ny == state.tank_y)) {
			//make_explosions(state, 5);
			//flash_screen(state, b_before);

			set_tile(state, { nx,ny }, '2');
			success = true;
		}

		else if (state.occupied.contains({ nx,ny }) &&  // if already occupied
			!(old_tile == 'C' || old_tile == '1') ||	// and not an enemy tile
			(nx == state.tank_x && ny == state.tank_y)) // and not Tank.
		{
			//dir = random_dir();	
			if (state.frontier_e2.empty()) {
				sx = e2->get_coord().first;
				sy = e2->get_coord().second;
			}
			else {
				int i = rand() % state.frontier_e2.size();
				sx = state.frontier_e2[i].first;
				sy = state.frontier_e2[i].second;
			}
			continue;							//restart loop.
		}

	}

	if (!(old_tile == 'C' || old_tile == '1') && !(old_tile == '2') && !(old_tile == info[e2->get_type()].symbol) &&
		!(nx == state.tank_x && ny == state.tank_y)
		) {
		set_tile(state, { nx,ny }, '2');
	}

	state.frontier_e2.clear();

	//char t = info[e1->get_type()].symbol;
	for (auto& [pos, tile] : state.tiles) {  //in all tiles
		bool is_capital = (pos.first == e2x && pos.second == e2y);

		if (tile != '2' && !is_capital) continue; // if not this enemy's tile

		int x = pos.first;
		int y = pos.second;

		for (const auto& [dx, dy] : dirs) {
			int nx = x + dx;
			int ny = y + dy;

			if (!state.occupied.contains({ nx, ny })) {
				state.frontier_e2.push_back(pos);
				break;
			}

			char neighbour_tile = state.tiles.at({ nx, ny });

			{
				if (neighbour_tile != '2' && neighbour_tile != is_capital) {
				}
				state.frontier_e2.push_back(pos);
				break;
			}
		}
	}
	return success;
}


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
		{2, "Move Tanks"},
		{3, "Buy Turrets"}, 
		{4, "Show current map"},
		{5, "Pause/Resume: press P to stop during attacks."},
		{9, "Return to System Menu"}
		
};

map<int, string> test_menu = {
		{1, "End of Year"},
		//{2, "Diplomacy"},
		{2, "War"},
		{3, "Missile"},
		{4, "Gain Access to Water"},
		{5, "Flash Screen aka BOOM"},
		{6, "Explosions"},
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

	//Bounds world = find_bounds(state); //Goal: making the rendered screen a stable rectangle with a fixed box instead. 
	Bounds world;
	world.min_x = -40;
	world.max_x = 40;
	world.min_y = -20;
	world.max_y = 20;

	for (int y = world.max_y; y >= world.min_y; --y) {
		for (int x = world.min_x; x <= world.max_x; ++x) {
			pair<int, int> pos{ x,y };

			char tc = tank_char_at(x, y, 5, 10);
			
			if (tc != ' ') {
				cout << tc;
			}
			
			else {
				if (state.tiles.contains(pos)) {
					char t = state.tiles.at(pos);

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
						draw(player_tile);
						draw(player_texture);
					}
					else if (t == '1') {
						draw(occ_enemy_1);
						draw(occ_enemy_texture);
					}
					else if (t == '2') {
						draw(occ_enemy_texture);
						draw(occ_enemy_2);
					}
					else if (t == 'L') {
						draw(water_wave);
						draw(water_tile);
					}
					else if (t == '9') {
						draw(explosion_tile);
						draw(explosion_tile);
					}
					else { cout << t; }
				}
				else {
					draw(empty_tile);
					draw(spacer);
				}
			}
		}
		cout << '\n';
	} 
	blank_lines(2);
}

void show_minimap_to_frame(GameState& state, Bounds& b, string& frame) {			//Rendering.
	Bounds world;
	world.min_x = -40;
	world.max_x = 40;
	world.min_y = -20;
	world.max_y = 20;
	int row = 0;
	state.tank_active = true;
	state.jet_active = true;
	Glyph g;
	g.use_color = false;

	for (int y = world.max_y; y >= world.min_y; --y) {
		
		int gap = 5;

		for (int x = world.min_x; x <= world.max_x; ++x) {
			pair<int, int> pos{ x,y };

			char tc = ' ';
			char jc = ' ';
			char bc = ' ';
			if (state.tank_active /*&& x == state.tank_x && y == state.tank_y*/) {
				tc = tank_char_at(x, y, state.tank_x, state.tank_y);
			}
			if (state.jet_active) {
				jc = unit_char_at(x, y, state.jet_x, state.jet_y, jet_art);
			}
			
			if (tc != ' ') {
				/*frame += tc;
				frame += " ";*/
				g.symbol = string(1, tc);
				append_draw(frame, g);
				append_draw(frame, spacer);
			}

			else if (jc != ' ') {
				frame += jc;
				frame += " ";
			}
			else if (state.bomb_is_flying == true && x == state.bomb_x && y == state.bomb_y) {
				append_draw(frame, bomb);
				append_draw(frame, spacer);
			}
			
				else if (state.tiles.contains(pos)) {
					char t = state.tiles.at(pos);

					if (t == 'X') {
						append_draw(frame, cap_player);
						append_draw(frame, spacer);
					}
					else if (t == 'R') {
						append_draw(frame, cap_repub);
						append_draw(frame, spacer);
					}
					else if (t == 'D') {
						append_draw(frame, cap_dict);
						append_draw(frame, spacer);
					}
					else if (t == 'N') {
						append_draw(frame, cap_negan);
						append_draw(frame, spacer);
					}
					else if (t == 'B') {
						append_draw(frame, cap_blackadder);
						append_draw(frame, spacer);
					}
					else if (t == 'C') {
						append_draw(frame, player_tile);
						append_draw(frame, player_texture);
					}
					else if (t == '1') {
						append_draw(frame, occ_enemy_1);
						append_draw(frame, occ_enemy_texture);
					}
					else if (t == '2') {
						append_draw(frame, occ_enemy_texture2);
						append_draw(frame, occ_enemy_2);
					}
					else if (t == 'L') {
						append_draw(frame, water_wave);
						append_draw(frame, water_tile);
					}
					else if (t == '9') {
						append_draw(frame, explosion_tile);
						append_draw(frame, explosion_tile);
					}
					else if (t == '8') {
						append_draw(frame, explosion_tile_orange);
						append_draw(frame, explosion_tile_orange);
					}


					else { frame+= t; }
				}
				else {
					append_draw(frame, empty_tile);
					append_draw(frame, spacer);
				}
			
		}
		
		frame += string(gap, ' '); 
		frame += msg_box_row(row);
		frame += '\n';
		row++;
	}
	//blank_lines(2);

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

bool single_attack(GameState& state, Player* player, string dir, Enemy* e1, Enemy* e2, int e1x, int e1y, int e2x, int e2y);

//int count_controlled_tiles(const GameState& state);
struct EnemyCounts {
	int e1;
	int e2;
};
EnemyCounts count_enemy_tiles(const GameState& state);

void update_bomb(GameState& state);

void repeat_attack(GameState& state, Player* player, Enemy* e1, Enemy* e2, int e1x, int e1y, int e2x, int e2y) {
	int days = 0;	
	
	cout << "How many days/turns?" << endl;
	cin >> days;
	cout << "Direction? north/south/east/west/random/access to water" << endl;
	cin >> state.direction;
	
	for (int i = 0; i < days; ++i) {

		if (_kbhit()) {
			char ch = _getch();
			if (ch == 'p' || ch == 'P') {
				state.pause = !state.pause;
				cout << "PAUSED\n";
				cout << "To drop a bomb, press 'b'." << endl;
				cout << "To resume, press 'p'." << endl;
			}
		}
		while (state.pause) {
			if (_kbhit()) {
				char ch = _getch();
				if (ch == 'p' || ch == 'P') {
					state.pause = false;
				}
				else if (ch == 'b' || ch == 'B') {
					state.bomb_is_flying = true;
					
					state.bomb_start_x = state.jet_x;
					state.bomb_start_y = state.jet_y;
					state.bomb_x = state.bomb_start_x;
					state.bomb_y = state.bomb_start_y; 
					state.bomb_dir = state.jet_dir;
					
					state.bomb_target_y = state.world.min_y;

					int dy = state.bomb_start_y - state.world.min_y;
					int dx = 2*dy;

					state.bomb_target_x = state.bomb_start_x + (state.bomb_dir * dx);
					

					state.pause = false;
				}
			}
		}
		update_bomb(state);
		state.day++;
		if (state.day % 365 == 0) {
			state.year++;
		}
		string dir = state.direction;
		if (dir == "random") {
			dir = random_dir();
		}
		else {
			if (i % 2) {
				dir = random_dir();
			}
		}
		
		single_attack(state, player, dir, e1, e2, e1x, e1y, e2x, e2y);

		if (state.victory || state.gameover) {
			break;
		}

	}
	// add a "Press enter..." to make it show. Goal; stabilizing screen, avoid jumpy rendering...
	/*cout << "You now control " << count_controlled_tiles(state) << " territories/ km^2." << endl;
	cout << "Enemy 1 now controls " << count_enemy_tiles(state).e1 << " territories/ km^2." << endl;
	cout << "Enemy 2 now controls " << count_enemy_tiles(state).e2 << " territories/ km^2." << endl;
	cout << "The distance to Enemy 1 is: " << shooting_distance(e1) << endl;*/
};


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
	int max_y = 39;
	int min_x = 0;
	int min_y = 19;
	
	for (auto [pos,tile]: state.tiles) {
		
		max_x = max(max_x, pos.first);
		max_y = max(max_y, pos.second);
		min_x = min(min_x, pos.first);
		min_y = min(min_y, pos.second);
	}
	return Bounds{max_x, max_y, min_x, min_y};
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
void flash_screen( GameState& state, string& frame, Bounds b);

void move_jet(GameState& state);

bool single_attack(GameState& state, Player* player,string dir,Enemy* e1, Enemy* e2, int e1x, int e1y, int e2x, int e2y) {

	bool success = false;
	auto b_before = find_bounds_player(state);
	char old_tile = 'w';
	int sx, sy;

		if (state.frontier.empty()) {
			sx = player->x;
			sy = player->y;
		}
		else {
			int i = rand() % state.frontier.size(); //**************
			sx = state.frontier[i].first;
			sy = state.frontier[i].second;
		}

		int nx = sx;					//initialization.
		int ny = sy;

		int attempts = 0;

		while (!success && attempts < 50) {

			attempts++;
			old_tile = 'w';			// old_tile reset to avoid that a value stays stale across iterations. 
			nx = sx;				//	same thinking.
			ny = sy;

			if (dir == "east")			nx = sx + 1;
			else if (dir == "west")		nx = sx - 1;
			else if (dir == "north")	ny = sy + 1;
			else if (dir == "south")	ny = sy - 1;

			if (state.tiles.contains({ nx,ny })) {
				old_tile = state.tiles.at({ nx,ny });
			}

			if ((nx == e1x && ny == e1y) || (nx == e2x && ny == e2y)) { // if Player takes an Enemy capital; Victory.
				set_tile(state, { nx,ny }, 'C');
				state.victory = 1;
					for (auto& [pos, tile] : state.tiles) {
						if (tile == '1' || tile == '2') {
							tile = 'C';
						}
					}
					auto b_after = find_bounds_player(state);

					frame.clear();
					update_msg_box(state);
					move_jet(state);
					show_minimap_to_frame(state, b_after, frame);
					cout << terminal::clear_and_home;
					cout << frame;

				success = true;
				return true;
			}
			else if (old_tile == '1' || old_tile == '2') {			//if enemy
					//make_explosions(state, 5);
					//flash_screen(state,frame, b_before);
				
				set_tile(state, { nx,ny }, 'C');
				success = true;
			}
			
			else if (state.occupied.contains({ nx,ny }) && // if already occupied
				!(old_tile == '1' || old_tile == '2') // and not an enemy tile ...meaning if belonging to the Player.
				) {
				//dir = random_dir();	
				// 				//roll the dice again.  *****************************************
				if (state.frontier.empty()) {
					sx = player->x;
					sy = player->y;
				}
				else {
					int i = rand() % state.frontier.size(); //**************
					sx = state.frontier[i].first;
					sy = state.frontier[i].second;
				}
				continue;							//restart loop.
			}

		}
		if (!(old_tile == '1' || old_tile == '2') && !(old_tile == 'C') && !(old_tile == 'X')) {
			set_tile(state, { nx,ny }, 'C');
		}
		move_jet(state);

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
		
	//enemy_attack_once(state, e1, '1');

		if (state.day % 2 == 0) {
			enemy_single_attack(state, dir, e1, e1x, e1y);
			enemy2_single_attack(state, dir, e2, e2x, e2y);
		}
		else {
			enemy2_single_attack(state, dir, e2, e2x, e2y);
			enemy_single_attack(state, dir, e1, e1x, e1y);
		}
	
	//enemy_attack_once(state, e2, '2');

	auto b_after = find_bounds_player(state);

	//show_minimap(state, b_after);
	frame.clear();
	update_msg_box(state);
	show_minimap_to_frame(state, b_after, frame);
	cout << terminal::clear_and_home;
	cout << frame;
	
	/*cout << "sx: " << sx << " sy: " << sy << endl;
	cout << "nx: " << nx << " ny: " << ny << endl;*/

	//cout << "You now control " << count_controlled_tiles(state) << " territories/ km^2." << endl;
	/*cout << "Enemy 1 now controls " << count_enemy_tiles(state).e1 << " territories/ km^2." << endl;
	cout << "Enemy 2 now controls " << count_enemy_tiles(state).e2 << " territories/ km^2." << endl;
	cout << "The distance to Enemy 1 is: " << shooting_distance(e1) << endl;*/
	this_thread::sleep_for(milliseconds(20));
	//cout << flush;

	return success;
}

bool move_tanks(GameState& state, Bounds& b, int e1x, int e1y, int e2x, int e2y) {
	//auto b_before = find_bounds_player(state);
	//state.day += 1;
	int days = 0;
	Bounds world;
	world.min_x = -40;
	world.max_x = 40;
	world.min_y = -20;
	world.max_y = 20;
	
	cout << "In what direction should the Tank Divisions advance? north/south/east/west" << endl;
	cin >> state.tank_dir;
	string dir = state.tank_dir;

	cout << "How many days in a row? (1 day = 1 tile)" << endl;
	cin >> days;

	//cout << "For how many turns? (max 10)" << endl;
	for (int i = 0; i < days; i++) {
		int sx, sy;

		sx = state.tank_x;
		sy = state.tank_y;

		int nx, ny;

		char old_tile = 'w';			// old_tile reset to avoid that a value stays stale across iterations. 
		nx = sx;				//	same thinking.
		ny = sy;

		if (dir == "east")			nx = sx + 1;
		else if (dir == "west")		nx = sx - 1;
		else if (dir == "north")	ny = sy + 1;
		else if (dir == "south")	ny = sy - 1;

		if (( nx > world.max_x || nx < world.min_x) || (ny > world.max_y || ny < world.min_y)) {
			cout << "Invalid coordinates!" << endl;
			break;
		}

		if (state.tiles.contains({ nx,ny })) {
			old_tile = state.tiles.at({ nx,ny });
		}

		state.day++;

		if ((nx == e1x && ny == e1y) || (nx == e2x && ny == e2y)) { // if Player/Tank takes an Enemy capital; Victory.
			set_tile(state, { nx,ny }, 'C');
			state.victory = 1;
			for (auto& [pos, tile] : state.tiles) {
				if (tile == '1' || tile == '2') {
					tile = 'C';
				}
			}
			state.tank_x = nx;
			state.tank_y = ny;
			auto b_after = find_bounds_player(state);

			frame.clear();
			update_msg_box(state);
			show_minimap_to_frame(state, b_after, frame);
			cout << terminal::clear_and_home;
			cout << frame;
			//success = true;
			return true;
		}
		else if (old_tile == '1' || old_tile == '2') {			//if enemy
			//make_explosions(state, 5);
			//flash_screen(state,frame, b_before);

			set_tile(state, { nx,ny }, 'C');
			state.tank_x = nx;
			state.tank_y = ny;
			//success = true;
		}


		if (!(old_tile == '1' || old_tile == '2') && !(old_tile == 'C') && !(old_tile == 'X')) {
			set_tile(state, { nx,ny }, 'C');
			state.tank_x = nx;
			state.tank_y = ny;
		}

		if (old_tile == 'C' || old_tile == 'X') {
			state.tank_x = nx;
			state.tank_y = ny;
		}

		auto b_after = find_bounds_player(state);

		frame.clear();
		update_msg_box(state);
		show_minimap_to_frame(state, b_after, frame);
		cout << terminal::clear_and_home;
		cout << frame;

		state.tank_dir.clear();

		
	}
	return true;
}

void move_jet(GameState& state) {
	state.jet_x += state.jet_dir;

	if (state.jet_x <= state.world.min_x || state.jet_x >= state.world.max_x) {
		state.jet_dir *= -1;
	}

	state.jet_target = state.jet_x;
}

bool launch_bomb(GameState& state) {
	state.bomb_is_flying = true;
	return state.bomb_is_flying;
};

void apply_explosion(GameState& state, int cx, int cy);

void update_bomb(GameState& state) {
	if (!state.bomb_is_flying) return;

	int& y = state.bomb_y;
	int& x = state.bomb_x;

	if (x != state.bomb_target_x || y != state.bomb_target_y) {  //each turn, just once
		y -= 1;
		x = x + 2 * (state.bomb_dir);
	}

	char old_tile = 'w';
	if (state.tiles.contains({ x,y })) {
		old_tile = state.tiles.at({ x,y });
	}

	if (old_tile == '1' || old_tile == '2' || old_tile == 'C' ||
		(x == state.bomb_target_x && y == state.bomb_target_y)){

		apply_explosion(state, x, y);
		state.bomb_is_flying = false;
	}
}

bool attack_neighbour(GameState& state,
	Player* player,
	Enemy* e1, Enemy* e2, int e1x, int e1y, int e2x, int e2y) {
	cout << /*"Do you wish to attack a neighbor? ('yes' or 'no') If yes,*/ "in which direction? 'north/south/west/east' ?" << endl;
	cin /*>> state.state_of_war*/ >> state.direction;

	//if (state.state_of_war != "yes") return false; 

	return single_attack(state, player, state.direction, e1, e2, e1x, e1y, e2x, e2y);
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
void flash_screen(GameState& state, string& frame, Bounds b);

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
		flash_screen(state, frame, b);
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
pair<int, int> lake_seed = random_coord();

//map<pair<int, int>, char> tiles;
//pair<int, int> lake_seed = random_coord();

vector<pair<int,int>> make_full_circle(int cx, int cy, int radius) {
	vector<pair<int, int>> tiles;

	for (int y = cy - radius; y <= cy + radius; ++y) {
		for (int x = cx - radius; x <= cx + radius; ++x) {
			double dx = (x - cx); 
			double dy = (y - cy); //*1.5; //compensating for the rectangular shape...terminal characters are twice as tall as they are wide. 

			if (dx * dx + dy * dy <= radius * radius) {//Pythagore: sum of the squared sides of a right triangle equals the length of the hypotenuse squared.
				tiles.push_back({x,y});
			}
		}
	}
	return tiles;
}

void basic_lake(GameState& state, pair<int, int>lake_seed, int number) {
	pair<int,int> l = lake_seed;
	for (int dx = 0; dx < number; dx++) {
		for (int dy = 0; dy < number; dy++) {
			//state.tiles[{l.first + dx, l.second - dy }] = 'L';
			set_tile(state, { l.first + dx, l.second - dy }, 'L');
		}
	}
}

void better_lake(GameState& state, pair<int,int> lake_seed, int radius) {
	auto circle_tiles = make_full_circle(lake_seed.first, lake_seed.second, radius);
	for (auto pos:circle_tiles) {
		set_tile(state, { pos.first, pos.second }, 'L');
	}
}

pair<int, int> target = { -40,-20 };

vector<pair<int, int>> make_river_path(pair<int, int> lake_seed, pair<int, int> target) {
	vector<pair<int, int>> tiles;
	int x = lake_seed.first;
	int y = lake_seed.second;

	/*while (x != target.first || y != target.second) {
		if (x < target.first) ++x;
		else if (x > target.first) --x;
		else if (y < target.second) ++y;
		else if (y > target.second) --y;*/

	int distance = abs(target.first - lake_seed.first) + abs(target.second - lake_seed.second);

	for (int i = 0; i < distance; i++){
		if (i % 2 == 0) {
			if (x != target.first) {
				if (x < target.first) ++x;
				else  --x;
			}
			else {
				if (y < target.second) ++y;
				else if (y > target.second) --y;
			}

		} else {
			if (y != target.second) {
				if (y < target.second) ++y;
				else --y;
			}
			else {
				if (x < target.first) ++x;
				else if (x > target.first)--x;
			}
		}

		tiles.push_back({ x,y });
	}
	return tiles;
}

void make_river(GameState& state, pair<int, int> lake_seed, pair<int, int> target) {
	auto river_tiles = make_river_path(lake_seed, target);

	for (auto pos : river_tiles) {
		set_tile(state, { pos.first, pos.second }, 'L');
	}
}



//void append_msg_box(string& frame, const vector<string>& lines) {
//};

void make_explosions(GameState& state, string frame, int number) {
	for (int i = 0; i < number; i++) {
		int radius = rand_int(2,5);
		auto p = random_coord();
		int cx = p.first;
		int cy = p.second;
		Bounds b = find_bounds_player(state);
	
		auto explosion_tiles = make_full_circle(cx, cy, radius); //I need to repeat/loop this one to get many circles. 
		auto explosion_tiles_orange = make_full_circle(cx+1, cy+1, radius  +1);
	
		for (auto tile : explosion_tiles) {
			set_tile(state, tile, '9');
		}

		frame.clear();
		show_minimap_to_frame(state, b, frame);
		cout << terminal::clear_and_home << frame;
		this_thread::sleep_for(milliseconds(70));
		
		for (auto tile : explosion_tiles_orange) {
			set_tile(state, tile, '8');
		}

		frame.clear();
		show_minimap_to_frame(state, b, frame);
		cout << terminal::clear_and_home << frame;
		this_thread::sleep_for(milliseconds(90));

		for (auto tile : explosion_tiles) {
			erase_tile(state, tile);
		}
		for (auto tile: explosion_tiles_orange) {
			erase_tile(state, tile);
		}

		if (i > 0 && i % 4 == 0) {
			this_thread::sleep_for(milliseconds(10));
		}
		
	}
}

void apply_explosion(GameState& state, int cx, int cy) {
	int radius = 4;
	//auto p = random_coord();
	//int cx = p.first;
	//int cy = p.second;
	Bounds b = find_bounds_player(state);
	
	auto explosion_tiles = make_full_circle(cx, cy, radius); //I need to repeat/loop this one to get many circles. 

	for (auto tile : explosion_tiles) {
		set_tile(state, tile, '9');
	}

	for (auto tile : explosion_tiles) {
		erase_tile(state, tile);
	}

}


void flash_screen(GameState& state, string& frame, Bounds b ) {
	
	for (int tries = 0; tries < 6; tries++) {
		
		frame.clear();
		flashing = true;
		//make_explosions(state, 5);
		//show_minimap(state, b);
		show_minimap_to_frame(state, b, frame);
		cout << terminal::clear_and_home << frame;
		this_thread::sleep_for(milliseconds(50));
		
		frame.clear();
		flashing = false;
		//show_minimap(state, b);
		show_minimap_to_frame(state, b, frame);
		cout << terminal::clear_and_home << frame;
		this_thread::sleep_for(milliseconds(50));
	}
}

void reset_game(GameState& state) {
	//it resets major stuff like year and day to 0, and calls the stuff necessary at the start, remaking random coord capitals.
	state.year = 0;
	state.day = 0;
	state.victory = 0;
	state.gameover = 0;
	state.tiles.clear();
	state.frontier.clear();
	state.frontier_e1.clear();
	state.frontier_e2.clear();
	state.occupied.clear();
	state.bomb_is_flying = false;

}

void initialize_game(GameState& state, Player* player, Enemy* e1, Enemy* e2, string frame, Bounds b) {
	
	cout << terminal::clear_and_home;
	b = find_bounds_player(state);
	flash_screen(state, frame, b);
	make_explosions(state, frame, 15);

	//tiles!
	//map<pair<int, int>, char> tiles; 
	state.tiles[{0, 0}] = 'X';

	pair<int, int> lake_seed = random_coord();
	better_lake(state, lake_seed, 4);
	make_river(state, lake_seed, target);

	auto p = random_coord();
	int e1x = p.first;
	int e1y = p.second;

	while /*(e1x == 0 && e1y == 0) || */(abs(e1x) + abs(e1y) < 10){ // Puts distance between enemy respawn and player and avoid player and enemy spawning at same spot.
		auto p = random_coord();
		e1x = p.first;
		e1y = p.second;
	}
	state.tiles[{e1x, e1y}] = info[e1->get_type()].symbol;
	e1->set_coord({ e1x , e1y });
	
	auto q = random_coord();
	int e2x = q.first;
	int e2y = q.second;

	while ((e2x == 0 && e2y == 0) ||
		(e2x == e1x && e2y == e1y)) {
		auto p = random_coord();
		e2x = p.first;
		e2y = p.second;
	}
	state.tiles[{e2x, e2y}] = info[e2->get_type()].symbol;
	e2->set_coord({ e2x, e2y });
}

int main()
{
	srand(time(nullptr));  //avoids making the same "random" path every time.

	GameState state;
	state.world.min_x = -40;
	state.world.max_x = 40;
	state.world.min_y = -20;
	state.world.max_y = 20;

	//Player* player = new Player();  // "()" means constructor 

	////Enemy e1;
	//Enemy* e1 = new Enemy();

	////Enemy e2;
	//Enemy* e2 = new Enemy();

	Player* player = nullptr;
	Enemy* e1 = nullptr;
	Enemy* e2 = nullptr;

	Bounds b = find_bounds_player(state);
	//initialize_game(state, e1, e2, frame, b); 

	/*int e1x = e1->get_coord().first;
	int e1y = e1->get_coord().second;

	int e2x = e2->get_coord().first;
	int e2y = e2->get_coord().second;*/

	int sys_choice = 0;

	cout << "\033[?25l";  // hides cursor
	//ShowWindow(GetConsoleWindow(), SW_MAXIMIZE); //maximizing window; buggy. 

	cout << "\033[32m";
	show_beta_banner();
	cout << "\033[0m";
	cout << "Maximize this window for a better gaming experience." << endl;

	cout << colors::light_blue << "Some features are shown in blue because they are still under construction." << colors::color_reset << endl;;
	cout << "Press Enter to continue...";
	cin.get();

	cout << terminal::clear_and_home;

	//flash_screen(state, frame, b);

	//map<pair<int, int>, char> roads;

	/*tiles[{lake_seed.first, lake_seed.second}] = 'L';
	tiles[{(lake_seed.first) + 1, lake_seed.second}] = 'L';*/

	//basic_lake(state, lake_seed, 5);
	//better_lake(state, lake_seed, 4);
	//make_river(state, lake_seed, target);

	int bank_choice = 0;

	map<int, string> prod_menu = {
		{1, "Jets"},
		{2, "Tanks"},
		{3, "Troops"},
		{4, "Trucks"},
		{5, "Industrial Robots"},
		{6, "Cars" },
		{99, "Set current year production"}
	};

	while (state.reset_answer == 'y') {
		sys_choice = 0;

		delete player;
		delete e1;
		delete e2;

		player = new Player();
		e1 = new Enemy();
		e2 = new Enemy();
		initialize_game(state, player, e1, e2, frame, b);
		
		auto [e1x, e1y] = e1->get_coord();
		auto [e2x, e2y] = e2->get_coord();
		while (sys_choice != 999 && !state.victory && !state.gameover) {						// Gameloop.
			auto bounds = normalize(find_bounds(state));
			//show_map(tiles, bounds);
			frame.clear();
			show_minimap_to_frame(state, b, frame);
			cout << terminal::clear_and_home << frame;
			//show_minimap(state, find_bounds_player(state));
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

				while (war_choice != 9 && !state.victory && !state.gameover) {
					war_choice = show_war_menu();

					switch (war_choice) {
					case 1:
					{
						repeat_attack(state, player, e1, e2, e1x, e1y, e2x, e2y);
						break;
					}
					case 2:
						if (state.year >= 1) {
							move_tanks(state, b, e1x, e1y, e2x, e2y);
						}
						else {
							cout << "Tank divisions will be available at the end of this Production Year." << endl;
						}
						break;


					case 4:
					{
						auto bounds = find_bounds(state);
						show_map(state, bounds);
						break;
					}

					case 5: 
					
					{
						state.pause = !state.pause;
						break;
					}
					}
				}
				break;
			}


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

				while (test_choice != 9 && !state.victory && !state.gameover) {
					test_choice = show_test_menu(test_menu);   

					switch (test_choice) {
					case 1:
						repeat_end_year(state, prod_menu);
						break;

					case 2:
						repeat_attack(state, player, e1, e2, e1x, e1y, e2x, e2y);
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
						pair<int, int> start = { state.current_x, state.current_y };
						pair<int, int> target = { lake_seed.first, lake_seed.second };
						access_water(start, target, state, e1, e2);
						break;
					}
					case 5:
					{
						Bounds b = find_bounds(state);
						flash_screen(state, frame, b);
						break;
					}
					case 6:
					{
						int number = 0;
						cout << "How many?" << endl;
						cin >> number;
						make_explosions(state, frame, number);
						//show_minimap(state, b);
						break;
					}

					case 9:
						break;

					}
					break;
				}
			//case 999: break;
			}
			}
			cout << "Do you want to play again?" << endl;
			cin >> state.reset_answer;
			if (state.reset_answer == 'y') {
				reset_game(state);
				sys_choice = 999;
			}


		}
		cout << "\033[?25h";  // show cursor again
	}
}
	
	


