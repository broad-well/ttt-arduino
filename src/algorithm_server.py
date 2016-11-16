""" TTT-Arduino Host (algorithm and other memory-intensive tasks) """
import copy
import random
import time

machine_char = ''

# Board manipulation subsection

WIN_PATTERNS = [
    [0, 1, 2],
    [3, 4, 5],
    [6, 7, 8],

    [0, 3, 6],
    [1, 4, 7],
    [2, 5, 8],

    [0, 4, 8],
    [2, 4, 6]
]

def board_winner(board):
    """ Returns the winner of the given board,
        ' ' if not determinable. """
    pset = None
    for pattern in WIN_PATTERNS:
        pset = set([board[x] for x in pattern])
        if len(pset) == 1 and list(pset)[0] != ' ':
            return list(pset)[0]
    return ' '

def board_gets():
    """ Returns a board from the user via input() interaction. """
    print("Input the board below")
    input_ = [input(), input(), input()]
    output = []
    for i in input_:
        output += list(i)
    return output

def board_other(i):
    """ Returns 'x' for 'o' and 'o' for 'x'. """
    return ('o', 'x')[i == 'o']

def board_score(board, ismachine):
    """ Returns a score for the given player with the board as the
        querying condition. win=1, tie=0, loss=-1 """
    board_result = board_winner(board)
    return 1 if (board_result ==
            (machine_char if ismachine else board_other(machine_char))) \
            else (0 if board_result in [' ', None] else -1)

def board_vacant_cells(board):
    """ Returns a list of indexes of the board parameter that point to ' ',
        or "vacant". """
    output = []
    for i in range(9):
        if board[i] == ' ':
            output.append(i)
    return output

def board_filled(board):
    """ Returns whether the board argument has no more empty cells to play. """
    return ' ' not in board

def board_print(board):
    """ Prints out the given board fancily. """
    for i in range(0, 9, 3):
        print("{}|{}|{}".format(*board[i:i+3]))

def board_clean():
    """ Returns a fresh board with all cells ' '. """
    return [' '] * 9

def mm_sel_rand(array, selector):
    """ Return an index of the array that is randomly chosen from all cells
        with the 'selector' value of the array. """
    maxes = []
    for i, v in enumerate(array):
        if v == selector(array):
            maxes.append(i)
    return random.choice(maxes)

def mm(board, depth, ismachine):
    """ The internal function of Minimax. Returns the score of the given board
        for the given perspective and the move that depicts that score."""
    winner = board_winner(board)
    if winner != ' ':
        ret_score = board_score(board, ismachine)
        if ismachine:
            ret_score *= 10 - depth
        else:
            ret_score *= depth - 10
        return (ret_score, None)
    if board_filled(board):
        return (0, None)

    scores = []
    moves = []
    temp_board = []
    mm_results = ()
    for move in board_vacant_cells(board):
        temp_board = copy.copy(board)
        if ismachine:
            temp_board[move] = machine_char
        else:
            temp_board[move] = board_other(machine_char)
        mm_results = mm(temp_board, depth + 1, not ismachine)
        scores.append(mm_results[0])
        moves.append(move)
    best_score_index = mm_sel_rand(scores, max if ismachine else min)
    return (scores[best_score_index], moves[best_score_index])

def interactive(machine_first):
    """ The main interactive program for testing minimax. """
    global machine_char
    machine_char = 'x' if machine_first else 'o'
    machine_turn = machine_first
    board = board_clean()
    while board_winner(board) == ' ' and not board_filled(board):
        if machine_turn:
            begin_time = time.time()
            board[mm(board, 0, True)[1]] = machine_char
            print("Solution obtained in {} milliseconds".format(
                (time.time()-begin_time)*1000))
        else:
            board[int(input("Your choice index ->"))] = \
                    board_other(machine_char)
        board_print(board)
        machine_turn = not machine_turn

# run this for interactive
if __name__ == '__main__':
    mf = input("Machine first? [0,1]")
    interactive(mf == '1')
