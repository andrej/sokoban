#include <cstdio>
#include <cstdlib>
#include <cstdbool>
#include <cstring>
#include <cassert>
#include <vector>
#include <functional>
#include "game.cpp"
#include "search.cpp"
#include "heuristic.cpp"
#include "mincostheuristic.cpp"
#include "io.cpp"


/** 
 * Game of Sokoban
 *
 * (c) 2020 André Rösti
 */

/** *************************************************************************
 * Glue Code
 * ************************************************************************** */

/**
 * Get action from user input (w, a, s, d, q). 
 */
int action_input(Coord *action) {
	int input;
	do {
		input = getchar();
	} while(input == '\n');
	action->x = 0;
	action->y = 0;
	if(input == 'q') {
		return input;
	} else if(input == 'w') {
		action->y = -1;
	} else if(input == 'a') {
		action->x = -1;
	} else if(input == 's') {
		action->y = +1;
	} else if(input == 'd') {
		action->x = +1;
	}
	return input;
}

/**
 * Translate "difference" between two states into action taken: up, down, left
 * or right.
 */
char action_to_char(Game *from, Game *to) {
	Coord offs = to->player - from->player;
	if(offs.x == -1) {
		return 'L';
	} else if(offs.x == +1) {
		return 'R';
	} else if(offs.y == -1) {
		return 'U';
	} else if(offs.y == +1) {
		return 'D';
	}
	return '?';
}

/**
 * Usage information / help
 */
int print_usage(char *name) {
	fprintf(stderr, "Usage: %s LEVEL [--interactive]\n", name);
	fprintf(stderr, "    LEVEL: Path to Sokoban level text file.\n");
	fprintf(stderr, "    --interactive: Play in interactive mode.\n");
	return 1;
}

/**
 * Main
 */
int main(int argc, char **argv) {

	// Print usage info.
	if(argc < 2 || argc > 3) {
		return print_usage(argv[0]);
	}

	bool interactive = false;
	if(argc == 3) {
		if(strcmp(argv[2], "--interactive") == 0) {
			interactive = true;
		} else {
			return print_usage(argv[0]);
		}
	}

	// Read in level to a new board.
	char *path = argv[1];
	Game board = board_from_file(path);


	MinCostHeuristic heur;

	// Non-interactive: Read in file, run algorithm, return
	if(!interactive) {
		std::vector<State *> solution = A_star(board, heur);
		Game *prev = NULL;
		for(std::vector<State *>::iterator it = solution.begin(); it != solution.end(); ++it) {
			Game *current = static_cast<Game *>(*it);
			if(!prev) {
				prev = current;
				continue;
			}
			char action = action_to_char(prev, current);
			putchar(action);
			putchar(' ');
			prev = current;
		}
		putchar('\n');
		return 0;
	}

	unsigned int n_moves = 0;

	// Main loop: repeatedly show game board, ask user for a move, apply
	// the move to the board state, check if goal state reached, then 
	// visualize board again.
	while(true) {
		char *viz = board_to_string(board);
		double h = heur(board);
		printf("\n%s\nh(x) = %f\n\n", viz, h);
		if(board.is_goal()) {
			fprintf(stderr, "Congratulations! You won after %d moves.\n", n_moves);
			break;
		}
		int input;
		Coord action;
		do {
			// Repeatedly ask user for a move until they make a legal one or quit the game.
			fprintf(stderr, "[%d] Make a move (x=run solver, w=up, a=left, s=down, d=right, q=quit): ", n_moves);
			input = action_input(&action);
			fprintf(stderr, "\n");
			if(input == 'q') {
				fprintf(stderr, "You gave up after %d moves. Goodbye.\n", n_moves);
				return 1;
			}
			if(input == 'x') {
				std::vector<State *> solution = A_star(board, heur);
				int j = 0;
				for(std::vector<State *>::iterator it = solution.begin(); it != solution.end(); ++it) {
					Game *step = static_cast<Game *>(*it);
					char *viz = board_to_string(*step);
					fprintf(stderr, "Solution step %d:\n%s\n", j+1, viz);
					j++;
				}
				return 2;
			}
		} while(!board.is_action_legal(action));
		n_moves += 1;
		board.take_action(action);
	}
	return 0;
}
