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
 * Main
 */
#define MAX_MOVES 4095
int main(int argc, char **argv) {

	// Print usage info.
	if(argc != 2) {
		fprintf(stderr, "Usage: %s LEVEL\n", argv[0]);
		fprintf(stderr, "    LEVEL: Path to Sokoban level text file.");
		return 1;
	}

	// Read in level to a new board.
	char *path = argv[1];
	Game board = board_from_file(path);

	// Keep track of moves made to show in the end.
	unsigned int n_moves = 0;
	char moves[MAX_MOVES+1];

	SimpleHeuristic heur;

	// Main loop: repeatedly show game board, ask user for a move, apply
	// the move to the board state, check if goal state reached, then 
	// visualize board again.
	while(n_moves < MAX_MOVES) {
		char *viz = board_to_string(board);
		double h = heur(board);
		printf("\nh(x) = %f\n%s\n", h, viz);
		if(board.is_goal()) {
			printf("Congratulations! You won after %d moves:\n%s\n",
				n_moves, moves);
			break;
		}
		int input;
		Coord action;
		do {
			// Repeatedly ask user for a move until they make a 
			// legal one or quit the game.
			std::vector<State *> children = board.get_neighbors();
			printf("Available moves: \n");
			for(int i = 0; i < children.size(); i++) {
				printf("%d: %d\n", i, children[i]->is_goal());
			}
			printf("[%d] Make a move (w=up, a=left, s=down, d=right, q=quit): ", n_moves);
			input = action_input(&action);
			printf("\n");
			if(input == 'q') {
				printf("You gave up after %d moves:\n%s\nGoodbye.\n",
				       n_moves, moves);
				return 1;
			}
			if(input == 'x') {
				std::vector<State *> solution = A_star(board, heur);
				int j = 0;
				for(std::vector<State *>::iterator it = solution.begin(); it != solution.end(); ++it) {
					Game *step = static_cast<Game *>(*it);
					char *viz = board_to_string(*step);
					printf("%d\n%s\n", j, viz);
					j++;
				}
				return 2;
			}
			if(!board.is_action_legal(action)) {
				printf("Illegal action. Please retry.\n");
			}
		} while(!board.is_action_legal(action));
		moves[n_moves] = (char)input;
		n_moves += 1;
		moves[n_moves] = '\0';
		board.take_action(action);
	};
	return 0;
}
