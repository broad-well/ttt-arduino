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
// sleep is used later
#ifdef __GNUC__

#include <unistd.h>
#define unisleep(x) sleep(x)

#endif
#ifdef _WIN32

#include <cstdlib>
#define unisleep(x) _sleep(x*1000)

#endif

// debug setup
#define TTT_DEBUG
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

// prints the board parameter in a way that humans understand.
void print_board(const Board& board)
{
		cout  << "┌───┬───┬───┐" << endl
					<< "│ " << board[0] << " │ " << board[1] << " │ " << board[2] << " │" << endl
					<< "├───┼───┼───┤" << endl
					<< "│ " << board[3] << " │ " << board[4] << " │ " << board[5] << " │" << endl
					<< "├───┼───┼───┤" << endl
					<< "│ " << board[6] << " │ " << board[7] << " │ " << board[8] << " │" << endl
					<< "└───┴───┴───┘" << endl;
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

/* ========== Interactive ========== */

void play_game(bool machine_first)
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
		print_board(brd);
		// obtain decisions
		if (whose_turn == machine) {

			cout << "Thinking..." << endl;
			unsigned short machine_decision = minimax(brd);
			brd[machine_decision] = machine;
			debug_write("move: " + fmt_move(machine, machine_decision));

		} else {
			// Goto is bad for readability, but it's in the proximity, so bear with it.
wait_user_choice:

			cout << "input choice here =>";
			short user_choice;
			cin >> user_choice;

			// we don't trust the player
			if (brd[user_choice] != ' ') {
				cout << "That cell is occupied!" << endl;
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
		cout << "You win. Unbelievable. ;(" << endl;
	}
	cout << "Board for reference: " << endl;
	print_board(brd);
}

/* ========== Main Routine ========== */

int main(int argc, const char** argv)
{
	debug_init();
	bool machine_first;
	cout << "1 for me first, 0 for you first >";
	cin >> machine_first;

	srand(time(NULL));
	// helper
	cout << "When inputting choice, follow this chart for desired cell:" << endl
		<< "┌───┬───┬───┐" << endl
		<< "│ 0 │ 1 │ 2 │" << endl
		<< "├───┼───┼───┤" << endl
		<< "│ 3 │ 4 │ 5 │" << endl
		<< "├───┼───┼───┤" << endl
		<< "│ 6 │ 7 │ 8 │" << endl
		<< "└───┴───┴───┘" << endl;
	play_game(machine_first);
	debug_exit();
}
