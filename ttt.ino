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

TttPosition get_position(char x, char y) {
	TttPosition pos;
	pos.x = x;
	pos.y = y;
	return pos;
}

// define items in ttt.h

TttBoard::TttBoard(TttGame* game) {
	serial_debug_enter("TttBoard_init");
	this->game = game;
	this->fill(None);
	serial_debug("Initialized new TttBoard object");
	serial_debug_exit();
}

TttCell* TttBoard::get_cell(TttPosition &pos) {
	return &(this->cells[pos.y][pos.x]);
}

void TttBoard::set_cell(TttPosition &pos, TttCell &new_cond) {
	this->cells[pos.y][pos.x] = new_cond;
}

void TttBoard::render(LcdManager &man) {
	serial_debug_enter("TttBoard::render");
	map<string,string> vars;
	for(char i0 = 0; i0 < 3; ++i0) {
		for(char i1 = 0; i1 < 3; ++i1) {
			vars["s"+to_string(i0)+to_string(i1)] = this->cells[i0][i1];
			serial_debug("rendered cell "+to_string(i0)+" "+to_string(i1)+" -> "+to_string(this->cells[i0][i1]));
		}
	}
	stringstream sstream;
	sstream << left << setw(15) << this->side_msg;
	vars["msg"] = sstream.str();
	vars["player_points"] = to_string(player_points);
	vars["machine_points"] = to_string(machine_points);
	man.update(vars);
	serial_debug_exit();
}

void TttBoard::fill(TttCell &condition) {
	for(char y = 0; y < 3; ++y) {
		for(char x = 0; x < 3; ++x) {
			this->set_cell(get_position(x, y), condition);
		}
	}
}

TttGame::TttGame(bool is_machine_first) {
	serial_debug_enter("TttGame_init");
	this->is_machine_first = is_machine_first;
	this->is_machine_turn = is_machine_first;
	this->board = new TttBoard(this);
	this->lcd = new LcdManager(this);
	serial_debug("Initialized new TttGame object");
	serial_debug_exit();
}

~TttGame::TttGame() {
	delete this->board;
	delete this->lcd;
}

vector<TttPosition> TttGame::get_possible_moves() {
	vector<TttPosition> moves;
	for(char y = 0; y < 3; ++y) {
		for(char x = 0; x < 3; ++x) {
			if(this->board.get_cell(get_position(x, y)) == None) {
				moves.push_back(get_position(x, y));
			}
		}
	}
	return moves;
}

TttCell* TttGame::cell_at(TttPosition &pos) {
	return this->board.get_cell(pos);
}

char TttBoard::winner() {
	serial_debug_enter("TttBoard::winner");
	TttCell first_cell;
	
	serial_debug("Now checking rows");	
	// check rows
	for(char y = 0; y < 3; ++y) {
		first_cell = *(this->get_cell(get_position(0, y)));
		if( *(this->get_cell(get_position(1, y))) == first_cell &&
				*(this->get_cell(get_position(2, y))) == first_cell) {
			serial_debug("Row winner found: " + to_string(first_cell));
			serial_debug_exit();
			return static_cast<char>(first_cell);
		}
	}
	
	serial_debug("Now checking columns");
	// check columns
	for(char x = 0; x < 3; ++x) {
		first_cell = *(this->get_cell(get_position(x, 0)));
		if( *(this->get_cell(get_position(x, 1))) == first_cell &&
				*(this->get_cell(get_position(x, 2))) == first_cell) {
			serial_debug("Column winner found: " + to_string(first_cell));
			serial_debug_exit();
			return static_cast<char>(first_cell);
		}
	}
	
	serial_debug("Now checking diagonals");
	// check diagonals
	first_cell = *(this->get_cell(get_position(1, 1))); // middle cell

	if( ( *(this->get_cell(get_position(0, 0))) == first_cell &&
				*(this->get_cell(get_position(2, 2))) == first_cell ) ||
			( *(this->get_cell(get_position(0, 2))) == first_cell &&
				*(this->get_cell(get_position(2, 0))) == first_cell ) {
			serial_debug("Diagonal winner found: " + to_string(first_cell));
			serial_debug_exit();
			return static_cast<char>(first_cell);
	}

	return None;
}

void TttGame::report_data_to_serial() {
	stringstream ss;
	ss << "TttGameReport => is_machine_turn: " << this->is_machine_turn << "; current winner: " << this->winner() << "; board side_msg: " << this->board.side_msg;
	Serial.println(ss.str());
}

// algorithm functions

#define SCORE_UNIT 10

char get_cell_char(TttCell& cell) {
		default:
			warn("get_cell_char: invalid cell: " + string(static_cast<char>(cell)));
	switch(static_cast<char>(cell)) {
		case ' ':
			return 0;
		case 'O':
			return 1;
		case 'X':
			return -1;
		default:
			warn("get_cell_char: invalid cell: " + string(static_cast<char>(cell)));
}

char TttGame::get_score(bool as_machine, TttBoard* condition) {
	return condition->winner() * (is_machine_first ? -1 : 1) * SCORE_UNIT;

}
