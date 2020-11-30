#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/** 
 * Game of Sokoban
 *
 * (c) 2020 André Rösti
 */


/** *************************************************************************
 * Game Data Types
 * ************************************************************************** */

/**
 * Two-dimensional coordinates, used for current player positions as well as 
 * actions (move left, right, up, down).
 */
typedef struct {
	int x;
	int y;
} coord_t;

/**
 * The board is represented as a NxM (row-major) matrix of fields, each of which
 * can be an empty field, wall, box, box on goal or player.
 */
typedef enum {empty, wall, box, box_on_goal, goal} field_t;
typedef struct {
	coord_t dimensions;
	field_t *fields;
} board_t;

/**
 * The current game state is represented by the player position (X, Y) and the
 * current board state.
 */
typedef struct {
	coord_t player;
	board_t board;
} state_t;


/** *************************************************************************
 * Game Logic
 * ************************************************************************** */

/**
 * Add two positions.
 */
coord_t add(coord_t a, coord_t b) {
	return (coord_t){a.x + b.x, a.y + b.y};
}

/**
 * Get the row-major index of a position on the given board.
 */
int get_index(board_t board, coord_t position) {
	return position.x + board.dimensions.x*position.y;
}

/**
 * Return the board value at the given coordinates.
 */
field_t get_field(board_t board, coord_t position) {
	return board.fields[get_index(board, position)];
}

/**
 * Set field.
 */
void set_field(board_t board, coord_t position, field_t value) {
	board.fields[get_index(board, position)] = value;
}

/**
 * Given the current board state, tell whether the desired action is legal.
 */
bool is_action_legal(state_t state, coord_t action) {
	coord_t new_pos = add(state.player, action);
	if(new_pos.x >= state.board.dimensions.x || 
	   new_pos.y >= state.board.dimensions.y ||
	   0 > new_pos.x ||
	   0 > new_pos.y) {
		// Action would move player outside of board dimensions.
		return false;
	}
	field_t neighbor = get_field(state.board, new_pos);
	if(neighbor == empty || neighbor == goal) {
		// Player moves into empty or goal field.
		return true;
	}
	if(neighbor == box || neighbor == box_on_goal) {
		field_t neighbor_of_neighbor = get_field(state.board, add(new_pos, action));
		if(neighbor_of_neighbor == empty || neighbor_of_neighbor == goal) {
			// Player moves box into empty or goal field.
			return true;
		}
	}
	// Action would move player into wall, a box into another box, or a box
	// into a wall, all of which is not allowed.
	return false;
}

/**
 * Return true if current state is goal state, i.e. all boxes are in goals.
 */
bool is_goal_state(state_t state) {
	for(int x = 0; x < state.board.dimensions.x; x++) {
		for(int y = 0; y < state.board.dimensions.y; y++) {
			field_t field = get_field(state.board, (coord_t){x, y});
			if(field != empty 
			   && field != wall
			   && field != box_on_goal) {
				return false;
			}
		}
	}
	return true;
}

/**
 * Apply the given action to the board.
 */
void take_action(state_t *state, coord_t action) {
	assert(is_action_legal(*state, action));
	coord_t neighbor = add(state->player, action);
	// Update player position (assuming action is legal).
	state->player.x = neighbor.x;
	state->player.y = neighbor.y;
	field_t field = get_field(state->board, neighbor);
	// If player is pushing box, remove box from that cell ...
	if(field == box) {
		set_field(state->board, neighbor, empty);
	} else if(field == box_on_goal) {
		set_field(state->board, neighbor, goal);
	}
	// ... and move it into the adjacent cell.
	if(field == box || field == box_on_goal) {
		coord_t neighbor_of_neighbor = add(neighbor, action);
		field_t neighbor_field = get_field(state->board, neighbor_of_neighbor);
		if(neighbor_field == goal) {
			set_field(state->board, neighbor_of_neighbor, box_on_goal);
		} else if(neighbor_field == empty) {
			set_field(state->board, neighbor_of_neighbor, box);
		} else {
			// Illegal action.
			assert(false);
		}
	}
}


/** *************************************************************************
 * Input/Output
 * ************************************************************************** */

typedef struct {
	char empty;
	char wall;
	char box;
	char box_on_goal;
	char goal;
	char player;
} field_chars_t;

/**
 * Default board characters used to encode different fields.
 */
field_chars_t field_chars = {' ', '#', 'O', '0', '.', 'x'};

/**
 * Return string visualization of the board with textual characters.
 */
char *board_to_string(state_t state) {
	int row_len = state.board.dimensions.x + 1; // additional char for newline
	int len = row_len * state.board.dimensions.y;
	char *out = malloc(sizeof(char) * len + 1); // additional char for terminating null
	for(int i = 0; i < len; i++) {
		if(i % row_len == state.board.dimensions.x) {
			out[i] = '\n';
			continue;
		}
		coord_t pos = {i % row_len,
				  i / row_len};
		field_t field = get_field(state.board, pos);
		if(pos.x == state.player.x && pos.y == state.player.y) {
			out[i] = field_chars.player;
		} else if(field == empty) {
			out[i] = field_chars.empty;
		} else if(field == wall) {
			out[i] = field_chars.wall;
		} else if(field == box) {
			out[i] = field_chars.box;
		} else if(field == box_on_goal) {
			out[i] = field_chars.box_on_goal;
		} else if(field == goal) {
			out[i] = field_chars.goal;
		}
	}
	return out;
}

/**
 * Create board object from string.
 */
state_t board_from_string(char *str) {
	int len = strlen(str);
	int width = strchr(str, '\n') - str;
	assert(width > 0);
	assert(len % (width+1) == 0);
	int height = len/width;
	field_t *fields = malloc(sizeof(field_t) * width * height);
	assert(fields != NULL);
	state_t state = {
		.player = (coord_t){0, 0},
		.board = (board_t){
			.dimensions = (coord_t){width, height},
			.fields = fields
		}
	};
	for(int i = 0; i < len; i++) {
		coord_t pos = {i % (width + 1),
			       i / (width + 1)};
		assert(pos.x < width || str[i] == '\n');
		if(pos.x >= width) {
			continue;
		}
		char field = str[i];
		if(field == field_chars.player) {
			state.player.x = pos.x;
			state.player.y = pos.y;
		} else if(field == field_chars.empty) {
			set_field(state.board, pos, empty);
		} else if(field == field_chars.wall) {
			set_field(state.board, pos, wall);
		} else if(field == field_chars.box) {
			set_field(state.board, pos, box);
		} else if(field == field_chars.box_on_goal) {
			set_field(state.board, pos, box_on_goal);
		} else if(field == field_chars.goal) {
			set_field(state.board, pos, goal);
		} else {
			// Encountered unknown character.
			assert(false);
		}
	}
	return state;
}

/**
 * Read a file and parse it as a board.
 */
#define MAX_FILE_SIZE 4095
state_t board_from_file(char *path) {
	FILE *fp = fopen(path, "r");
	assert(fp != NULL);
	char *str = malloc(sizeof(char) * (MAX_FILE_SIZE + 1));
	int len = fread(str, sizeof(char), MAX_FILE_SIZE, fp);
	fclose(fp);
	str[len] = '\0';
	state_t board = board_from_string(str);
	free(str);
	return board;
}


/** *************************************************************************
 * Glue Code
 * ************************************************************************** */

/**
 * Get action from user input (w, a, s, d, q). 
 */
int action_input(coord_t *action) {
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
	state_t board = board_from_file(path);
	int input;

	// Keep track of moves made to show in the end.
	unsigned int n_moves = 0;
	char moves[MAX_MOVES+1];

	// Main loop: repeatedly show game board, ask user for a move, apply
	// the move to the board state, check if goal state reached, then 
	// visualize board again.
	while(true) {
		char *viz = board_to_string(board);
		printf("\n%s\n", viz);
		if(is_goal_state(board)) {
			printf("Congratulations! You won after %d moves:\n%s\n",
				n_moves, moves);
			break;
		}
		int input;
		coord_t action;
		do {
			// Repeatedly ask user for a move until they make a 
			// legal one or quit the game.
			printf("[%d] Make a move (w=up, a=left, s=down, d=right, q=quit): ", n_moves);
			input = action_input(&action);
			printf("\n");
			if(input == 'q') {
				printf("You gave up after %d moves:\n%s\nGoodbye.\n",
				       n_moves, moves);
				exit(1);
			}
			if(!is_action_legal(board, action)) {
				printf("Illegal action. Please retry.\n");
			}
		} while(!is_action_legal(board, action));
		moves[n_moves] = (char)input;
		n_moves += 1;
		moves[n_moves] = '\0';
		take_action(&board, action);
	};
	return 0;
}
