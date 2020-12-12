#include <cstdio>
#include "game.cpp"

#ifndef IO_H
#define IO_H

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
char *board_to_string(Game &state) {
	int row_len = state.board.dimensions.x + 1; // additional char for newline
	int len = row_len * state.board.dimensions.y;
	char *out = new char[len+1]; // additional char for terminating null
	for(int i = 0; i < len; i++) {
		if(i % row_len == state.board.dimensions.x) {
			out[i] = '\n';
			continue;
		}
		Coord pos = {i % row_len,
				  i / row_len};
		Board::Field field = state.board.get_field(pos);
		if(pos.x == state.player.x && pos.y == state.player.y) {
			out[i] = field_chars.player;
		} else if(field == Board::empty) {
			out[i] = field_chars.empty;
		} else if(field == Board::wall) {
			out[i] = field_chars.wall;
		} else if(field == Board::box) {
			out[i] = field_chars.box;
		} else if(field == Board::box_on_goal) {
			out[i] = field_chars.box_on_goal;
		} else if(field == Board::goal) {
			out[i] = field_chars.goal;
		}
	}
	out[len] = '\0';
	return out;
}

/**
 * Create board object from string.
 */
Game board_from_string(char *str) {
	int len = strlen(str);
	int width = strchr(str, '\n') - str;
	assert(width > 0);
	assert(len % (width+1) == 0);
	int height = len/width;
	Board::Field *fields = new Board::Field[width*height];
	assert(fields != NULL);
	Game state((Coord){0, 0},
		   Board((Coord){width, height}, fields));
	for(int i = 0; i < len; i++) {
		Coord pos = {i % (width + 1),
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
			state.board.set_field(pos, Board::empty);
		} else if(field == field_chars.wall) {
			state.board.set_field(pos, Board::wall);
		} else if(field == field_chars.box) {
			state.board.set_field(pos, Board::box);
		} else if(field == field_chars.box_on_goal) {
			state.board.set_field(pos, Board::box_on_goal);
		} else if(field == field_chars.goal) {
			state.board.set_field(pos, Board::goal);
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
Game board_from_file(char *path) {
	FILE *fp = fopen(path, "r");
	assert(fp != NULL);
	char *str = new char[MAX_FILE_SIZE+1];
	int len = fread(str, sizeof(char), MAX_FILE_SIZE, fp);
	fclose(fp);
	str[len] = '\0';
	Game board = board_from_string(str);
	//free(str);
	return board;
}

#endif
