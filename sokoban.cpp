#include <cstdio>
#include <cstdlib>
#include <unistd.h>
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
 * Reads a sokoban level file as input and outputs steps (moves) to solve the
 * level. Alternatively, using the -p option, the level can be played 
 * interactively by the user.
 * 
 * The first drafted implementation was done in C, hence most of this does not
 * make use of C++ language features.
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
	fprintf(stderr, "Usage: %s LEVEL [-p] [-s] [-v] [-r] [-l]\n", name);
	fprintf(stderr, "    LEVEL: Path to Sokoban level text file.\n");
	fprintf(stderr, "    -p: Play in interactive mode.\n");
	fprintf(stderr, "    -s: Use simple heuristic (for performance comparison).\n");
	fprintf(stderr, "    -v, -vv: Print (very) verbose output to stderr.\n");
	fprintf(stderr, "    -r: Replay solution after it has been found\n");
	fprintf(stderr, "    -l: Use alternative visual input format.\n");
	return 1;
}

/**
 * Replay solution
 */
void replay_solution(std::vector<State *> solution) {
	for(std::vector<State *>::iterator it = solution.begin(); it != solution.end(); ++it) {
		fprintf(stderr, "%s\n\n", board_to_string(*static_cast<Game *>(*it)));
		fflush(stderr);
		usleep(150000);
	}
}

/**
 * Main
 */
int main(int argc, char **argv) {

	// Print usage info.
	if(argc < 2) {
		return print_usage(argv[0]);
	}

	bool interactive = false;
	bool simple_heuristic = false;
	bool replay = false;
	bool old_fmt = false;
	int verbosity = 0;

	// all args except for file are optional
	int opt;
	while((opt = getopt(argc, argv, "lpsvr")) != -1) {
		switch(opt) {
			case 'p':
				interactive = true;
				break;
			case 's':
				simple_heuristic = true;
				break;
			case 'r':
				replay = true;
				break;
			case 'v':
				verbosity++;
				break;
			case 'l':
				old_fmt = true;
				break;
		}
	}

	if(optind >= argc) {
		print_usage(argv[0]);
	}

	// Read in level to a new board.
	char *path = argv[optind];
	Game board = board_from_file(path, old_fmt);

	Heuristic *heuristic;
	if(simple_heuristic) {
		heuristic = new SimpleHeuristic();
	} else {
		heuristic = new MinCostHeuristic();
	}

	// Non-interactive: Read in file, run algorithm, return
	if(!interactive) {
		std::vector<State *> solution = A_star(board, *heuristic, verbosity > 1);
		if(verbosity > 0) {
			fprintf(stderr, "Solution found:\n");
		}
		printf("%lu ", solution.size());
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
		if(replay) {
			fprintf(stderr, "\nSolution replay:\n");
			usleep(2000000);
			replay_solution(solution);
		}
		return 0;
	}

	unsigned int n_moves = 0;

	// Interactive
	// Main loop: repeatedly show game board, ask user for a move, apply
	// the move to the board state, check if goal state reached, then 
	// visualize board again.
	while(true) {
		char *viz = board_to_string(board);
		double h = (*heuristic)(board);
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
				std::vector<State *> solution = A_star(board, *heuristic);
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
