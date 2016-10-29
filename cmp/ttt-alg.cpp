/*
 * =====================================================================================
 *
 *       Filename:  ttt-alg.cpp
 *
 *    Description:  Tic Tac Toe algorithm
 *
 *        Version:  0.2-alpha
 *        Created:  10/28/2016 07:07:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Michael Peng, 
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

using namespace std;

/* ========== Global Variables, Typedefs ========== */

// the role of the machine
char machine = ' ';

// simplified board
typedef array<char, 9> Board;

/* ========== Board Manipulation ========== */

// deduces the winner of the board.
// return values: 'o', 'x', or ' ' for no winner
char board_winner(Board& board)
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

	char first_cell = ' ';
	for (auto& win_ptn: WIN_PTNS) {
		// check if all cells referenced from this array are equal in value
		first_cell = board[win_ptn[0]];
		if (board[win_ptn[1]] == first_cell &&
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
	cout << board[0] << '|' << board[1] << '|' << board[2] << endl <<
	        board[3] << '|' << board[4] << '|' << board[5] << endl <<
	        board[6] << '|' << board[7] << '|' << board[8] << endl;
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

// TODO: Algorithm section

/* ========== Main Routine ========== */

int main(int argc, const char** argv)
{
	Board b = board_from_input();
	cout << "winner: " << board_winner(b) << endl;
	cout << "emptycells: ";
	for (short cindex: empty_cells(b)) {
		cout << cindex << ' ';
	};
	cout << endl << "is full?: " << is_full(b) << endl;
}
