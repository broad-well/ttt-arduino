/* Tic-Tac-Toe for Arduino header file
 *
 * Author: Michael Peng
 * Date: October 4, 2016
 *
 */

#include <vector>
#include <string>
#include <map>
#include <LiquidCrystal.h>

using namespace std;

enum TttCell {
	None = ' ',
	O = 'O',
	X = 'X',
}

struct TttPosition {
	char x;
	char y;
};

class TttBoard {
public:
	TttCell cells[3][3];
	string side_msg;
	TttGame* game;

	TttBoard(TttGame* game);

	TttCell* get_cell(TttPosition &pos);
	void set_cell(TttPosition &pos, TttCell &new_cond);
	void render(LcdManager &man);
	void fill(TttCell &condition);
};

class TttGame {
public:
	TttBoard board;
	LcdManager lcd;
	bool is_machine_first;
	bool is_machine_turn;

	TttGame(bool is_machine_first);
	~TttGame();
	vector<TttPosition> get_possible_moves();
	TttCell* cell_at(TttPosition &pos);
	char winner(); // -1 = None, 0 = O, 1 = X
	void report_data_to_serial();
	// algorithm
	char get_score(bool as_machine);
	char minimax(int depth);
};


class Strategizer {
public:
	TttGame* game;
	
	TttPosition decide(TttGame &game);
};

#define LCD_FORMAT "Tic-Tac-Toe    $s00$|$s01$|$s02$\n$msg$$s10$|$s11$|$s12$\nMinimax!       $s20$|$s21$|s22$\nYOU: $player_points$ ME: $machine_points$"
#define LCD_ROWS 4
#define LCD_COLUMNS 20

class LcdManager {
static:
	string replace(string &orig, map<string,string> &values);
public:
	LiquidCrystal lcd;
	LcdManager(TttGame* game);
	string[] format(map<string,string> &data);
	void update(map<string,string> &data);
}
