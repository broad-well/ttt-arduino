/* 
 * Tic-Tac-Toe for Arduino
 *
 * Author: Michael Peng
 * Date: October 4, 2016
 *
 */

#include "Arduino.h"
#include "LiquidCrystal.h"

#include <set>
#include <string>
#include <cmath>
#include <sstream>

#define TTT_ARDUINO
#define TTT_DEBUG

unsigned int machine_wins = 0;
unsigned int player_points = 0;

std::set<std::string> serial_buffer;
std::set<std::string> serial_levels;

void serial_debug(std::string& msg) {
#ifdef TTT_DEBUG
	if (Serial.available()) {
		for (std::string& buffer_object: serial_buffer) {
			Serial.println(("buffer: "+buffer_object).c_str());
		}
		Serial.println(msg.c_str());
		return;
	}
	serial_buffer.insert(msg);
#endif
}

void serial_debug_enter(std::string& level) {
#ifdef TTT_DEBUG
	if (serial_levels.find(level) == serial_levels.end()) {
		serial_levels.insert(level);
		return;
	}		
	Serial.println(("warn: serial_debug_enter existent level: " + level).c_str());
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

void warn(std::string& msg) {
	if (Serial.available()) {
		Serial.println(("!WARN! " + msg).c_str());
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
#define DoThreeTimes for (size_t q = 0; q < 3; ++q)

// variables that are volatile
std::string lcd_lines[4];

// should not be reset anywhere
char machine_score = 0;
char player_score = 0;
char current_turn;

// simplification of position representation
typedef struct Position {
	unsigned char x;
	unsigned char y;
};

// The following variables are per-game only and should be reset when a new game is initiated.
char board[3][3] = {}; 
Position selected_position;
char machine_char;

// create positions easily
Position get_pos(char x, char y) {
	Position pos;
	pos.x = x;
	pos.y = y;
	return pos;
}

// set the cell of position on the board to a specified value
// ONLY places where x and y are inversed
// 4 upcoming functions
void set_cell(Position& pos, char val) {
	board[pos.y][pos.x] = val;
}
void set_cell(char x, char y, char val) {
	board[y][x] = val;
}
char get_cell(Position& pos) {
	return board[pos.y][pos.x];
}
char get_cell(char x, char y) {
	return board[y][x];
}

template<typename T>
std::string to_string(T& i)
{
	std::stringstream sstr("");
	sstr << i;
	return sstr.str();
}

// std::to_string alternative
template<typename T>
std::string format(std::string orig, std::set<T> slist) {
	size_t startpos = 0;
	for (size_t i = 0; i < slist.size(); ++i) {
		while ((startpos = orig.find(
						"{" + to_string<T>(i) + "}")) != std::string::npos) {
			orig.replace(startpos, std::floor(std::log10(std::abs(i))) + 1, slist[i]);
		}
	}
}

// returns whether a == b
bool equals(Condition& a, Condition& b) {
	for (size_t c = 0; c < 3; ++c) {
		for (size_t d = 0; d < 3; ++d) {
			if (a[c][d] != b[c][d])
				return false;
		}
	}
	return true;
}

// ----- LCD START -----
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// board cell selection increment; this uses an algorithm similar to converting numbers of different bases.
void selection_operation(bool add) {
	char uninum = selected_position.y * 3 + selected_position.x;
	uninum = (uninum + (add ? 1 : 8)) % 9;
	selected_position.y = floor(uninum / 3);
	selected_position.x = uninum - (selected_position.y * 3);
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
byte cursor_char[8] = {
	B11111,
	B11111,
	B11111,
	B10001,
	B11111,
	B11111,
	B11111
};

// ready the LCD for further actions
void lcd_setup() {
	lcd.createChar(0, cursor_char);
	lcd.begin(20, 4);
}

// update a line of information on the LCD. (argument should be in range of 0 to 3)
void lcd_update_line(unsigned char line)
{
	lcd.setCursor(0, line);
	lcd.print(lcd_lines[line].c_str());
	if (line < 3) {
		lcd.setCursor(14, line);
		char board_row[5] = {};
		
		DoThreeTimes {
			board_row[2*q] = get_cell(2*q, line);
		}
		
		board_row[1] = '|';
		board_row[3] = '|';
		
		lcd.print(board_row);
	}
}

void set_line(unsigned char linenum, std::string content)
{
	if (linenum < 3 && content.size() > 15)
		warn(format("While setting line for linenum {0}, content size > 15", linenum));
	lcd_lines[linenum] = content;
	lcd_update_line(linenum);
}

// update all lines
void lcd_render()
{
	for (char i = 0; i < 4; ++i) {
		lcd_update_line(i);
	}
}

// sets the cursor to the specified position
void place_cursor(Position& pos)
{
	// x = 0 -> col = 15; x = 1 -> col = 17; x = 2 -> col = 19; y -> row
	lcd.setCursor(15 + (pos.x * 2), pos.y);
}

// sets a cell to a certain value; great for board manipulation
void render_cell(Position& pos, char value)
{
	place_cursor(pos);
	lcd.write(value);
}

// updates a row of cells on the board
void update_row(char row_num)
{
	DoThreeTimes {
		render_cell(get_pos(q, row_num), equals(selected_position, get_pos(q, row_num)) ?
			0 : get_cell(q, row_num));
	}
}

// enumerate calling update_row to update entire board
void update_board() {
	DoThreeTimes {
		update_row(q);
	}
}

// updates score counter (last line)
void lcd_score() {
	set_line(3, format(std::string("Player: {0}; Self: {1}"), {player_score, machine_score}));
}

void clear_lines() {
	for (size_t i = 0; i < 4; ++i) {
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
std::set<Position> get_vacant_cells(Condition& cond) {
	std::set<Position> positions;

	// iterate through all cells	
	for (char y = 0; y < 3; ++y) {
		for (char x = 0; x < 3; ++x) {
			if (get_cell(cond, x, y) == ' ')
				positions.insert(get_pos(x, y));
		}
	}

	return positions;
}

const std::array<std::array<std::pair<char, char>, 3>, 8> winning_patterns = {{
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
char cell_at(std::pair<char, char>& pos) {
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

const char player_char() {
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

const char other(char orig) {
	if (orig != 'O' && orig != 'X') {
		warn("Invalid parameter(orig): {0}" + orig);
		return ' ';
	}
	return orig == 'O' ? 'X' : 'O';
}

constexpr char const_machine()
{
	return static_cast<const char>(machine_char);
}

// ----- BOARD MANIPULATION END -----

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

short waitbtn(short btn_pins[]) {
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
	while (get_winner(board) == ' ') {	
		if (current_turn == machine_char) {
			set_line(0, "My turn");
			set_cell(minimax(static_cast<Condition>(board)), current_turn);
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
	case const_machine():
		// machine wins
		set_line(1, "I win!");
		break;
	case player_char():
		// user wins
		set_line(1, "You win...");
		set_line(2, "Unbelievable.");
	default:
		set_line(3, "ERROR! winner");
		warn("Winner char unknown -> " + std::to_string(get_winner()));
	}
	set_line(0, "SEL for new game");
	wait_button(SEL);
}

// ----- ARDUINO setup(), loop() END -----
