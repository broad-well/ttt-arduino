/*
 * =====================================================================================
 *
 *       Filename:  ttt-alg.cpp
 *
 *    Description:  Tic Tac Toe algorithm
 *
 *        Version:  0.3-alpha
 *        Created:  10/28/2016 07:07:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Michael Peng
 *   Organization:  A.E. Kent Middle School
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <array>
#include <algorithm>
#include <vector>
#include <limits>
#include <fstream>
#include <chrono>
#include "termcolor.hpp"
// sleep is used later
#if defined(__linux__) || defined(__APPLE__)

#include <unistd.h>
#define unisleep(x) sleep(x)

#endif
#ifdef _WIN32

#include <cstdlib>
#define unisleep(x) _sleep(x*1000)

#endif

// debug setup
//#define TTT_DEBUG
#define DEBUG_FNAME "/tmp/ttt-debug.log"

using namespace std;

/* ========== Debug Routines ========== */

#ifdef TTT_DEBUG

#pragma message "Tic Tac Toe Debug is enabled"
fstream debug_file;

void debug_init()
{
	debug_file.open(DEBUG_FNAME, iostream::out);
}

void debug_write(const string& msg)
{
	debug_file << msg << endl;
}

void debug_exit()
{
	debug_file.close();
}

#else

#define debug_init()
#define debug_write(m)
#define debug_exit()

class NullBuffer : public streambuf {
	public:
		int overflow(int c) {
			return c;
		}
};

NullBuffer debug_nbuff;
ostream debug_file(&debug_nbuff);

#endif

/* ========== Global Variables, Typedefs ========== */

// the role of the machine
char machine = ' ';

// simplified board
typedef array<char, 9> Board;

/* ========== Board Manipulation ========== */

// deduces the winner of the board.
// return values: 'o', 'x', or ' ' for no winner
// following function is declared later
string board_to_string(const Board& board);
char board_winner(const Board& board)
{
	// winning patterns
	const short WIN_PTNS[8][3] = {
		// horizontals
		{0, 1, 2},
		{3, 4, 5},
		{6, 7, 8},

		// verticals
		{0, 3, 6},
		{1, 4, 7},
		{2, 5, 8},

		// diagonals
		{0, 4, 8},
		{2, 4, 6}
	};

	for (auto& win_ptn: WIN_PTNS) {
		// check if all cells referenced from this array are equal in value
		// previous bug: prematurely returned ' ' from loop
		char first_cell = board[win_ptn[0]];
		if (first_cell != ' ' &&
				board[win_ptn[1]] == first_cell &&
				board[win_ptn[2]] == first_cell) {
			return first_cell;
		}
	}

	// No winners found - could be completely full, could be partly occupied
	return ' ';
}

// retrieves the cell of given index from the board and adds appropriate prefix
// to the char, specified by the DEFINEs below.
#define MACHINE_CELL termcolor::magenta
#define PLAYER_CELL termcolor::cyan
const string paint_choice(const Board& board, unsigned short index)
{
	debug_file << "paint_choice: board=" << board_to_string(board)
		<< ",index=" << index << endl;
	stringstream sstr;
	sstr << (board[index] == machine ? MACHINE_CELL : PLAYER_CELL) << board[index];
	sstr << termcolor::reset;
	debug_file << "paint_choice: result=" << sstr.str() << endl;
	return sstr.str();
}

// shorthand function for print_board_row
const string paint_choice_if(const Board& b, unsigned short i, bool do_)
{
	return (do_ ?
			paint_choice(b, i) :
			to_string(b[i]));
}

// returns a row of the board for use by print_board ONLY
const string print_board_row(const Board& board, bool with_color,
		unsigned short sindex)
{
	debug_file << "print_board_row: " << board_to_string(board) << ","
		<< with_color << "," << sindex << endl;
	stringstream sstr;
	sstr << "│ " <<
		paint_choice_if(board, sindex, with_color)
		<< " │ " <<
		paint_choice_if(board, sindex+1, with_color)
		<< " │ " <<
		paint_choice_if(board, sindex+2, with_color)
		<< " │" << endl;
	debug_file << "print_board_row: result=" << sstr.str() << endl;
	return sstr.str();
}

// prints the board parameter in a way that humans understand.
void print_board(const Board& board, bool with_color)
{
		cout  << "┌───┬───┬───┐" << endl
					<< print_board_row(board, with_color, 0)
					<< "├───┼───┼───┤" << endl
					<< print_board_row(board, with_color, 3)
					<< "├───┼───┼───┤" << endl
					<< print_board_row(board, with_color, 6)
					<< "└───┴───┴───┘" << endl << termcolor::reset;
}

// constructs a board object from string.
Board board_from_string(string str_)
{
	Board output;
	copy(str_.begin(), str_.end(), output.data());
	return output;
}

// constructs a board through user input.
Board board_from_input()
{
	cout << "Please type the board below" << endl;
	stringstream boardstr;

	// used in the loop, declared outside
	string segment;
	for (size_t i = 0; i < 3; ++i) {
		cout << "Row " << i+1 << " >>";
		getline(cin, segment);
		boardstr << segment;
	}

	return board_from_string(boardstr.str());
}

// returns a vector of board indexes that point to empty cells.
vector<short> empty_cells(const Board& board)
{
	vector<short> output;
	for (size_t i = 0; i < 9; ++i) {
		if (board[i] == ' ')
			output.push_back(i);
	}
	return output;
}

// returns a completely sanitary board for new games.
Board clean_board()
{
	Board output;
	output.fill(' ');
	return output;
}

// returns whether the given board is full and needs to be disposed.
// could've used empty_cells().empty(), but this is simpler
bool is_full(const Board& board)
{
	return find(board.begin(), board.end(), ' ') == board.end();
}

// returns a string that represents a certain move.
string fmt_move(char player, short index)
{
	stringstream output;
	output << 'm' << player << index;
	return output.str();
}

// returns a string that represents a certain board.
string board_to_string(const Board& brd)
{
	stringstream output;
	for (size_t i = 0; i < 9; ++i) {
		output << brd[i];
	}
	return output.str();
}

/* ========== Algorithms ========== */

// returns 'x' for 'o' and 'o' for 'x', '!' otherwise
char inverse(char input)
{
	switch (input) {
		case 'x':
			return 'o';
		case 'o':
			return 'x';
		default:
			return '!';
	}
}

// returns the preferability score for the given board in the perspective of
// the given player (ismachine)
short score(const Board& board, bool is_machine)
{
	char winner = board_winner(board);
	if (winner == ' ')
		return 0;

	return is_machine ? (winner == machine ? 1 : -1) :
		         (winner == inverse(machine) ? 1 : -1);
}

// returns a random index of the given vector that points to (one of) the largest
// numbers in it
unsigned int rand_max_index(const vector<int>& vect)
{
	vector<unsigned int> occurrences;
	int largest_occurred = numeric_limits<int>::min();

	for (unsigned int i = 0; i < vect.size(); ++i) {
		if (vect[i] > largest_occurred) {
			largest_occurred = vect[i];
			occurrences = {i};
		} else if (vect[i] == largest_occurred) {
			occurrences.push_back(i);
		}
	}

	// suffers from distribution problem, but no uniform distribution required
	return occurrences[rand() % occurrences.size()];
}

// returns a random index of the given vector that points to (one of) the
// smallest numbers in it
unsigned int rand_min_index(const vector<int>& varr)
{
	vector<unsigned int> occurrences;
	int smallest_occurred = numeric_limits<int>::max();

	for (unsigned int i = 0; i < varr.size(); ++i) {
		if (varr[i] < smallest_occurred) {
			smallest_occurred = varr[i];
			occurrences = {i};
		} else if (varr[i] == smallest_occurred) {
			occurrences.push_back(i);
		}
	}

	return occurrences[rand() % occurrences.size()];
}

// the internal function of MiniMax, called recursively.
int minimax_internal(Board board, unsigned short depth, bool ismachine)
{
	short initial_score = score(board, ismachine);
	if (initial_score != 0) {
		return ismachine ?
			initial_score * (10 - depth) :
			initial_score * (depth - 10);
	}
	if (is_full(board))
		return 0;

	// insertion-ordered mapping from move to score, plus individual vector manipulation
	// should be at same size at any time except between insertions in loop
	vector<short> moves;
	vector<int> scores;
	Board hypo_board;
	for (short& _move: empty_cells(board)) {
		hypo_board = board;
		hypo_board[_move] = ismachine ? machine : inverse(machine);

		moves.push_back(_move);
		scores.push_back(minimax_internal(hypo_board, depth+1, !ismachine));
	}

	unsigned int chosen_index = ismachine ?
		rand_max_index(scores) : rand_min_index(scores);
	return scores[chosen_index];
}

// the external function of minimax, returns desired move, or -1 if game over
// simplified version of minimax_internal
short minimax(const Board& board)
{
	if (score(board, true) != 0 ||
			is_full(board))
		return -1;

	vector<short> moves;
	vector<int> scores;
	Board hypo_board;
	for (short& _move: empty_cells(board)) {
		hypo_board = board;
		hypo_board[_move] = machine;

		moves.push_back(_move);
		scores.push_back(minimax_internal(hypo_board, 1, false));
	}

	return moves[rand_max_index(scores)];
}

// a dumb strategizer, only gets random index from available cells
short dumb_strategy(const Board& board)
{
	vector<short> empties = empty_cells(board);
	return empties[ rand() % empties.size() ];
}

/* ========== Interactive ========== */

short get_short_range(const string& prompt, short low, short high)
{
	short input;
	cout << prompt;
	while (true) {
		cin >> input;
		debug_file << "get_short_range: input = " << input << endl;
		if (input > low && input < high) {
			break;
		} else {
			cout << "\e[1A\e[2KOut of range! " << prompt;
		}
	}
	debug_file << "get_short_range: about to return" << endl;
	return input;
}

void play_game(bool machine_first, short difficulty)
{
	// prepare game by defining turn variables and obtaining clean board
	machine = machine_first ? 'x' : 'o';
	char whose_turn = 'x';
	cout << "You are " << inverse(machine) << endl;
	Board brd = clean_board();

	// main game loop
	while (score(brd, true) == 0 &&
	       !is_full(brd)) {

		// present the game to the player
		print_board(brd, true);
		// obtain decisions
		if (whose_turn == machine) {

			cout << "Thinking..." << endl;
			unsigned short machine_decision;
			switch (difficulty) {
				case 0:
					machine_decision = dumb_strategy(brd);
					break;
				case 2:
					machine_decision = minimax(brd);
					break;
				default:
					cerr << "Bad difficulty!" << endl;
			}
			brd[machine_decision] = machine;
			debug_write("move: " + fmt_move(machine, machine_decision));

		} else {
			// Goto is bad for readability, but it's in the proximity, so bear with it.
wait_user_choice:

			cout << termcolor::yellow;
			short user_choice;
			user_choice = get_short_range("input choice here =>", -1, 9);
			cout << termcolor::reset;

			// we don't trust the player
			if (brd[user_choice] != ' ') {
				cout << termcolor::red <<
					"That cell is occupied!" << termcolor::reset << endl;
				unisleep(2);
				cout << "\e[2K\e[1A\e[2K\e[1A";
				goto wait_user_choice;
			}
			brd[user_choice] = inverse(machine);
			debug_write("move: " + fmt_move(inverse(machine), user_choice));
		}
		// clear the message and relocate the cursor
		cout << "\e[1A\e[2K\e[7A\r";
		whose_turn = inverse(whose_turn);
		debug_write("board: " + board_to_string(brd));
	}
	// intended to clear the 1st line of the board
	cout << "\e[2K";
	char winner = board_winner(brd);
	if (winner == machine) {
		cout << "I win. Ha :D" << endl;
	} else if (winner == ' ') {
		cout << "Tie" << endl;
	} else {
		cout << "You win. ;(" << endl;
	}
	cout << "Board for reference: " << endl;
	print_board(brd, true);
}

/* ========== Main Routine ========== */

// all prompts should be yellow
int main(int argc, const char** argv)
{
	debug_init();
	cout << termcolor::green << "Hello player!" << termcolor::reset << endl;
	bool machine_first;
	cout << termcolor::yellow;
	machine_first =
		get_short_range("1 for me first, 0 for you first >", -1, 2) == 1;
	short difficulty =
		get_short_range("0 for Easy, 2 for Impossible >", -1, 3);

	srand(chrono::system_clock::now().time_since_epoch().count());
	// helper
	cout << termcolor::cyan << termcolor::bold <<
		"When inputting choice, follow this chart for desired cell:" << endl
		<< "┌───┬───┬───┐" << endl
		<< "│ 0 │ 1 │ 2 │" << endl
		<< "├───┼───┼───┤" << endl
		<< "│ 3 │ 4 │ 5 │" << endl
		<< "├───┼───┼───┤" << endl
		<< "│ 6 │ 7 │ 8 │" << endl
		<< "└───┴───┴───┘" << endl << termcolor::reset;
	play_game(machine_first, difficulty);
	debug_exit();
}
