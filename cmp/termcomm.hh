/*
 * ==========================================================================
 *
 *       Filename:  termcomm.hh
 *
 *    Description:  Terminal communication header for Tic Tac Toe
 *
 *        Version:  0.1
 *        Created:  11/07/2016 09:51:48 AM
 *       Revision:  0
 *       Compiler:  gcc
 *
 *         Author:  Michael Peng
 *   Organization:  Adaline E. Kent Middle School
 *
 * ==========================================================================
 */

#ifndef TTT_TERMCOMM

#include "termcolor.hpp"
#define TTT_TERMCOMM

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

short proto_out(short proto)
{
	switch (proto) {
	case PROTO_IMTHINKING:
		cout << termcolor::blink << "I'm thinking..." << termcolor::reset << endl;
		break;
	case PROTO_BADCHOICE:
		cout << termcolor::red <<
			"That cell is occupied!" << termcolor::reset << endl;
		unisleep(2);
		cout << "\e[2K\e[1A\e[2K\e[1A";
		break;
	case PROTO_FINECHOICE:
		// clear the message and relocate the cursor
		cout << "\e[1A\e[2K\e[7A\r";
		break;
	case PROTO_GAMEDONE:
		cout << "\e[2K";
		break;
	case PROTO_IWIN:
		cout << termcolor::green << "I win. Ha!" << termcolor::reset << endl;
		break;
	case PROTO_UWIN:
		cout << termcolor::red << "You win. Sad." << termcolor::reset << endl;
		break;
	case PROTO_TIE:
		cout << termcolor::underline << "Draw!" << termcolor::reset << endl;
		break;
	default:
		cout << termcolor::on_red << termcolor::white <<
			"Bad protocol: " << proto << termcolor::reset << endl;
		return 1;
	}
	return 0;
}

short proto_query(short proto_out)
{
	switch (proto_out) {
	case PROTO_WHOFIRST:
		cout << termcolor::yellow << "1 for machine first, 0 for you first >";
		bool machfirst;
		cin >> machfirst;
		cout << "Difficulty? (1,2,3) >";
		short diff;
		cin >> diff;
		return (machfirst ? 1 : -1) * diff;

	case PROTO_WHATCELL: {
		cout << termcolor::yellow;
		short ch = get_short_range("Which cell do you choose? >", -1, 9);
		cout << termcolor::reset;
		return ch;
	}

	case PROTO_AGAIN:
		cout << termcolor::yellow;
		return get_short_range("Again? >", -1, 2);

	default:
		cout << termcolor::on_red << termcolor::white <<
			"Bad protocol: " << proto_out << termcolor::reset << endl;
		return -1;
	}
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

Board board_in()
{
	return board_from_input();
}

short board_out(const Board& brd)
{
	print_board(brd, true);
	return 0;
}

#endif
