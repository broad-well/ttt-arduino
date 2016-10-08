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
 */

// variables that are volatile
string lcd_lines[4];

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
void replace_var(string& orig, const unordered_map<string, string>& keyval) {
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
}

// ready the LCD for further actions
void lcd_setup() {
	lcd.createChar(0, cursor_char);
	lcd.begin(20, 4);
}

// generate key -> value pairs like {cell00} -> 'O' and {cell20} -> '\0'
unordered_map<string, string> generate_row_cell_keyvals(char y) {
	unordered_map<string, string> keyval;
	for (char x = 0; x < 3; ++x) {
		keyval["cell" + to_string(x) + to_string(y)] = get_cell(x, y);
	}
}

// calls the previous function 3 times to get all cells returned
unordered_map<string, string> generate_all_cells_keyvals() {
	unordered_map<string, string> output;
	unordered_map<string, string> per_row;
	for (char y = 0; y < 3; ++y) {
		per_row = generate_row_cell_keyvals(y);
		output.insert(per_row.start(), per_row.end());
	}
	return output;
}

// update a line of information on the LCD. (argument should be in range of 0 to 3)
void lcd_update_line(char line) {
	unordered_map<string, string> all_keyval;
	
	// if first 3 lines, then generate keyvals
	if (line < 3) {
		unordered_map<string, string> cell_keyval = generate_row_cell_keyvals(line);
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

// update all lines
void lcd_render() {
	for (char i = 0; i < 4; ++i) {
		lcd_update_line(i);
	}
}
