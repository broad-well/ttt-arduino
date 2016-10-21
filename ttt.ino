/* 
 * Tic-Tac-Toe for Arduino
 *
 * Author: Michael Peng
 * Date: October 4, 2016
 *
 */

#include "ttt.h"
#include <algorithm>
#include <set>
#include <unordered_set>
#include <functional>
#include <unordered_map>

#define TTT_ARDUINO
#define TTT_DEBUG

using namespace std;

unsigned int machine_wins = 0;
unsigned int player_points = 0;
set<string> serial_buffer;
set<string> serial_levels;

void serial_debug(string &msg) {
#ifdef TTT_DEBUG
	if (Serial.available()) {
		for(string buffer_object: serial_buffer) {
			Serial.println(("buffer: "+buffer_object).c_str());
		}
		Serial.println(msg.c_str());
		return;
	}
	serial_buffer.insert(msg);
#endif
}

void serial_debug_enter(string &level) {
#ifdef TTT_DEBUG
	if (serial_levels.find(level) == serial_levels.end()) {
		serial_levels.insert(level);
		return;
	}		
	Serial.println("warn: serial_debug_enter existent level: " + level);
#endif
}

void serial_debug_exit() {
#ifdef TTT_DEBUG
	if (!serial_levels.empty()) {
		auto iter = serial_levels.end();
		serial_levels.erase(--iter);
	}
#endif
}

void warn(string& msg) {
	if (Serial.available()) {
		Serial.println("!WARN! " + msg);
	}
}

// new functional design

/* Fundamental codes (representations) for elements
 * 'O' for O selection
 * 'X' for X selection
 *
 * note: while accessing the array to place positions, x and y are inversed. That should be the only place the inversion takes place. i.e. "board[y][x]"
 *
 * Pins: LCD=>12,11,5,4,3,2 SEL1=>6 SEL2=>7 SEL=>8
 */
#define TTT_FIRST_CHAR 'X'

// variables that are volatile
string lcd_lines[4];

// should not be reset anywhere
char machine_score = 0;
char player_score = 0;
char current_turn;

// simplification of position representation
struct position {
	unsigned char x;
	unsigned char y;
};

// The following variables are per-game only and should be reset when a new game is initiated.
char board[3][3] = {}; 
position selected_position;
char machine_char;

// create positions easily
position get_pos(char x, char y) {
	position pos;
	pos.x = x;
	pos.y = y;
	return pos;
}

// set the cell of position on the board to a specified value
// ONLY places where x and y are inversed
// 4 upcoming functions
void set_cell(position& pos, char val) {
	position[pos.y][pos.x] = val;
}
void set_cell(char x, char y, char val) {
	position[y][x] = val;
}
char get_cell(position& pos) {
	return position[pos.y][pos.x];
}
char get_cell(char x, char y) {
	return position[y][x];
}

// ----- LCD START -----
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// replacements use unordered_map<string, string>, but that is too long.
typedef unordered_map<string, string> ReplaceMap;

// board cell selection increment; this uses an algorithm similar to converting numbers of different bases.
void selection_operation(bool add) {
	char uninum = selected_position.y * 3 + selected_position.x;
	uninum = (uninum + (add ? 1 : 8)) % 9;
	selected_position.y = floor(uninum / 3);
	selected_position.x = num - (selected_position.y * 3);
}
// Increment selection
void sel_incr() {
	selection_operation(true);
}
// Decrement selection
void sel_decr() {
	selection_operation(false);
}

// Cursor character for cell selection
const byte cursor_char[8] = {
	B11111,
	B11111,
	B11011,
	B10101,
	B11011,
	B11111,
	B11111
};

// LCD formatting backbone; x first, y second for cell expressions
const string lcd_backbone[4] = {
	"{line0}{cell00}|{cell10}|{cell20}",
	"{line1}{cell01}|{cell11}|{cell21}",
	"{line2}{cell02}|{cell12}|{cell22}",
	"{line3} TTT MP"
}

// Use this function to replace all {...}s with actual values
void replace_var(string& orig, const ReplaceMap& keyval) {
	size_t loc;
	for (auto keyval_pair: keyval) {
		loc = orig.find("{" + keyval_pair.first + "}");
		if (loc != string::npos) {
			orig.replace(loc, keyval_pair.first.length() + 2, keyval_pair.second);
			continue;
		}
		warn("replace_var: var not found: " + keyval_pair.first + " from " + orig);
	}
}

// fill line[0-3] to reach 15 (desired length) for correct board display.
// manipulates the powerful stream capabilities
string& fill_line_msg(string& line) {
	stringstream ss;
	ss << left << setfill(' ') << setw(15) << line;
	line = ss.str();
	return line;
	// does not get deleted because it is a parameter
}

// ready the LCD for further actions
void lcd_setup() {
	lcd.createChar(0, cursor_char);
	lcd.begin(20, 4);
}

// generate key -> value pairs like {cell00} -> 'O' and {cell20} -> '\0'
ReplaceMap generate_row_cell_keyvals(char y) {
	ReplaceMap keyval;
	for (char x = 0; x < 3; ++x) {
		keyval["cell" + to_string(x) + to_string(y)] = get_cell(x, y);
	}
}

// calls the previous function 3 times to get all cells returned
ReplaceMap generate_all_cells_keyvals() {
	ReplaceMap output;
	ReplaceMap per_row;
	for (char y = 0; y < 3; ++y) {
		per_row = generate_row_cell_keyvals(y);
		output.insert(per_row.start(), per_row.end());
	}
	return output;
}

// update a line of information on the LCD. (argument should be in range of 0 to 3)
void lcd_update_line(char line) {
	ReplaceMap all_keyval;
	
	// if first 3 lines, then generate keyvals
	if (line < 3) {
		ReplaceMap cell_keyval = generate_row_cell_keyvals(line);
		all_keyval.insert(cell_keyval.start(), cell_keyval.end());
	}
	
	all_keyval["line" + to_string(line)] = fill_line_msg(lcd_lines[line]);
	lcd.setCursor(0, line);
	string line_output = lcd_backbone[line];
	replace_var(line_output, all_keyval);
	if (line_output.length() != 20) 
		warn("lcd_update_line: line_output is not 20 chars long: " + line_output);
	lcd.print(line_output);	
}

void set_line(char linenum, string content) {
	lcd_lines[linenum] = content;
	lcd_update_line(linenum);
}

// update all lines
void lcd_render() {
	for (char i = 0; i < 4; ++i) {
		lcd_update_line(i);
	}
}

// sets the cursor to the specified position
void place_cursor(position& pos) {
	// x = 0 -> col = 15; x = 1 -> col = 17; x = 2 -> col = 19; y -> row
	lcd.setCursor(15 + (pos.x * 2), pos.y);
}

// sets a cell to a certain value; great for board manipulation
void render_cell(position& pos, char value) {
	place_cursor(pos);
	lcd.write(value);
}

// updates a row of cells on the board
void update_row(char row_num) {
	for (char x = 0; x < 3; ++x) {
		render_cell(get_pos(x, row_num), selected_position == get_pos(x, row_num) ?
			0 : get_cell(x, row_num));
	}
}

// enumerate calling update_row to update entire board
void update_board() {
	for (char y = 0; y < 3; ++y) {
		update_row(y);
	}
}

// updates score counter (last line)
void lcd_score() {
	set_line(3, "Player: " + to_string(player_score) + "; Self: " + to_string(machine_score));
}

void clear_lines() {
	for (short i = 0; i < 4; ++i) {
		set_line(i, "");
	}
}

// ----- LCD END -----

// ----- BOARD MANIPULATION START -----

// simplify Condition
typedef char Condition[3][3];

// only Condition handlers where y and x are inversed
char get_cell(Condition& cond, char x, char y) {
	return cond[y][x];
}
void set_cell(Condition& cond, char x, char y, char val) {
	cond[y][x] = val;
}

// returns all cells on the board that are not occupied (has ' ' value)
unordered_set<position> get_vacant_cells(Condition& cond) {
	unordered_set<position> positions;

	// iterate through all cells	
	for (char y = 0; y < 3; ++y) {
		for (char x = 0; x < 3; ++x) {
			if (get_cell(cond, x, y) == ' ')
				positions.insert(get_pos(x, y));
		}
	}

	return positions;
}

const array<array<pair<char, char>, 3>, 8> winning_patterns = {{
	{{0,0}, {1,0}, {2,0}},
	{{0,1}, {1,1}, {2,1}},
	{{0,2}, {1,2}, {2,2}},

	{{0,0}, {0,1}, {0,2}},
	{{1,0}, {1,1}, {1,2}},
	{{2,0}, {2,1}, {2,2}},

	{{0,0}, {1,1}, {2,2}},
	{{0,2}, {1,1}, {2,0}}
}};

// a shorthand for get_winner function
char cell_at(pair<char, char>& pos) {
	return get_cell(pos.first(), pos.second());
}

// will return ' ' for no winner yet
char get_winner(Condition& condition) {
	for (auto& pattern: winning_patterns) {
		char first_pos = cell_at(pattern[0]);
		
		if (cell_at(pattern[1]) == first_pos &&
			  cell_at(pattern[2]) == first_pos)
			return first_pos;
	}
	return ' ';
}

char player_char() {
	return other(machine_char);
}

char get_score(char winner, long depth) {
	return winner == machine_char ? 10 - depth: (winner == ' ' ? 0 : depth - 10);
}

void clear_board() {
	for (char y = 0; y < 3; ++y) {
		for (char x = 0; x < 3; ++x) {
			board[x][y] = ' ';
		}
	}
}

char other(char orig) {
	if (orig != 'O' && orig != 'X') {
		warn("Invalid parameter(orig): " + to_string(orig));
		return ' ';
	}
	return orig == 'O' ? 'X' : 'O';
}

// ----- BOARD MANIPULATION END -----
// ----- ALGORITHM START -----

// returns score of worst move
char min_move(Condition cond, long depth) {
	char winner = get_winner(cond);
	if (winner != ' ')
		return get_score(winner, depth);

	char this_score;
	char lowest_score = 127; // highest char possible
	for (position vacant_pos: get_vacant_cells(cond)) {
		set_cell(cond, vacant_pos.x, vacant_pos.y, machine_char);
		this_score = max_move(cond, depth+1);
		if (this_score < lowest_score)
			lowest_score = this_score;
		set_cell(cond, vacant_pos.x, vacant_pos.y, ' ');
	}

	return lowest_score;
}

// returns score of best move
char max_move(Condition cond, long depth) {
	char winner = get_winner(cond);
	if (winner != ' ')
		return get_score(winner, depth);

	char this_score;
	char highest_score = -127; // lowest signed char possible
	for (position vacant_pos: get_vacant_cells(cond)) {
		set_cell(cond, vacant_pos.x, vacant_pos.y, player_char());
		this_score = min_move(cond, depth+1);
		if (this_score > highest_score)
			highest_score = this_score;
		set_cell(cond, vacant_pos.x, vacant_pos.y, ' ');
	}

	return highest_score;
}

position minimax(Condition cond) {
	char best_score = -127;
	position best_move;
	
	char score;
	for (position vacant_pos: get_vacant_cells(cond)) {
		set_cell(cond, vacant_pos.x, vacant_pos.y, machine_char);
		score = max_move(cond, 1);
		if (score > best_score) {
			best_score = score;
			best_move = vacant_pos;
		}
	}

	return best_move;
}

// ----- ALGORITHM END -----

// ----- SIMPLE BUTTON MANIPULATION -----
// pin numbers
#define SEL1 6
#define SEL2 7
#define SEL 8
#define BTNs {SEL1,SEL2,SEL}

void setup_buttons() {
	pinMode(SEL1, INPUT);
	pinMode(SEL2, INPUT);
	pinMode(SEL, INPUT);
}

short waitbtn(short[] btn_pins) {
	while (true) {
		for (short btnpin: btn_pins) {
			if (digitalRead(btnpin) == HIGH)
				return btnpin;
		}
	}
}

short wait_button() {
	short btns[] = BTNs;
	return waitbtn(btns);
}

short wait_choice() {
	short btns[] = {SEL1, SEL2};
	return waitbtn(btns);
}

void wait_button(short btn_pin) {
	while (digitalRead(btn_pin) == LOW);
}

void user_cell_choice() {
	short btn;
	while (true) {
		btn = wait_button();
		if (btn == SEL)
			break;
		if (btn == SEL1)
			sel_decr();
		if (btn == SEL2)
			sel_incr();
		update_board();
	}
}

// ----- ARDUINO setup(), loop() -----

void setup() 
{
	Serial.begin(9600);
	lcd_setup();
	setup_buttons();
	// Lcd/button initialization done	
}

void loop() 
{
	// new game
	clear_board();
	set_line(0, "Welcome");
	set_line(1, "SEL1=mefirst");
	set_line(2, "SEL2=youfirst");
	lcd_score();
	{
		// selection of who starts first
		short choice = wait_choice();
		machine_char = (choice == SEL1 ?
			TTT_FIRST_CHAR :
			other(TTT_FIRST_CHAR));
		current_turn = (choice == SEL1 ? machine_char : player_char());
	}
	while (get_winner() == ' ') {	
		if (current_turn == machine_char) {
			set_line(0, "My turn");
			set_cell(minimax((Condition) board), current_turn);
		} else {
			set_line(0, "Your turn");
			user_cell_choice();
			set_cell(selected_position, current_turn);
		}
		if (get_vacant_cells.empty())
			break;
		current_turn = other(current_turn);
	}
	clear_lines();
	switch (get_winner()) {
	case ' ':
		// tie
		set_line(1, "Tie!");
		break;
	case machine_char:
		// machine wins
		set_line(1, "I win!");
		break;
	case player_char():
		// user wins
		set_line(1, "You win...");
		set_line(2, "Unbelievable.");
	default:
		set_line(3, "ERROR! winner");
		warn("Winner char unknown -> " + to_string(get_winner()));
	}
	set_line(0, "SELECT new game");
	wait_button(SEL);
}

// ----- ARDUINO setup(), loop() END -----
