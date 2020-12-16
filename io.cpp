#include <cstdio>
#include <set>
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
		Coord pos(i % row_len,
			  i / row_len);
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
int board_from_string(char *str, Game *state) {
	int len = strlen(str);
	int width = strchr(str, '\n') - str;
	assert(width > 0);
	assert(len % (width+1) == 0);
	int height = len/width;
	Board::Field *fields = new Board::Field[width*height];
	assert(fields != NULL);
	state->player = Coord(0, 0);
	state->board.dimensions = Coord(width, height);
	state->board.fields = fields;
	for(int i = 0; i < len; i++) {
		Coord pos(i % (width + 1),
			  i / (width + 1));
		assert(pos.x < width || str[i] == '\n');
		if(pos.x >= width) {
			continue;
		}
		char field = str[i];
		if(field == field_chars.player) {
			state->player.x = pos.x;
			state->player.y = pos.y;
		} else if(field == field_chars.empty) {
			state->board.set_field(pos, Board::empty);
		} else if(field == field_chars.wall) {
			state->board.set_field(pos, Board::wall);
		} else if(field == field_chars.box) {
			state->board.set_field(pos, Board::box);
		} else if(field == field_chars.box_on_goal) {
			state->board.set_field(pos, Board::box_on_goal);
		} else if(field == field_chars.goal) {
			state->board.set_field(pos, Board::goal);
		} else {
			// Encountered unknown character.
			return 1;
		}
	}
	return 0;
}

/**
 * Helper function that reads list of coordinates in the format
 * N X_1 Y_1 X_2 Y_2 ...
 * e.g.
 * 2 1 2 2 3 -> {Coord(2, 1), Corod(2, 3)}
 * Returns number of characters in str consumed.
 */
int read_coords(char *str, std::set<Coord> *coords) {
	int pos = 0;
	int n = 0;
	if(1 != sscanf(str+pos, "%d %n", &n, &pos)) {
		return 0;
	}
	for(int i = 0; i < n; i++) {
		int x = 0;
		int y = 0;
		int read = 0;
		if(2 != sscanf(str+pos, "%d %d%n", &y, &x, &read)) {
			// for some reason, in this input format, y and x coordinates are flipped
			return 0;
		}
		pos += read + 1; // for trailing space or newline character
		coords->insert(Coord(x-1, y-1));
	}
	return pos;
}

/**
 * Initialize board object from a file in the format as described in the project
 * manual.
 */
int board_from_new_fmt_string(char *str, Game *state) {
	int pos = 0;
	int read = 0;
	int width = 0;
	int height = 0;
	if(2 != sscanf(str, "%d %d\n%n", &width, &height, &read)) {
		return 1;
	}
	pos += read;
	Board::Field *fields = new Board::Field[width*height]();
	state->board.fields = fields;
	state->board.dimensions = Coord(width, height);
	std::set<Coord> walls;
	read = read_coords(str+pos, &walls);
	pos += read;
	if(!read) {
		return 1;
	}
	for(std::set<Coord>::iterator it = walls.begin(); it != walls.end(); ++it) {
		state->board.set_field(*it, Board::wall);
	}
	if(0 != sscanf(str+pos, "\n%n", &read)) {
		return 1;
	}
	pos += read;
	std::set<Coord> boxes;
	read = read_coords(str+pos, &boxes);
	pos += read;
	if(!read) {
		return 1;
	}
	if(0 != sscanf(str+pos, "\n%n", &read)) {
		return 1;
	}
	pos += read;
	std::set<Coord> goals;
	read = read_coords(str+pos, &goals);
	pos += read;
	if(!read) {
		return 1;
	}
	for(std::set<Coord>::iterator it = boxes.begin(); it != boxes.end(); ++it) {
		if(goals.count(*it)) {
			state->board.set_field(*it, Board::box_on_goal);
		} else {
			state->board.set_field(*it, Board::box);
		}
	}
	for(std::set<Coord>::iterator it = goals.begin(); it != goals.end(); ++it) {
		if(boxes.count(*it)) {
			continue;
		}
		state->board.set_field(*it, Board::goal);
	}
	if(0 != sscanf(str+pos, "\n%n", &read)) {
		return 1;
	}
	pos += read;
	if(2 != sscanf(str+pos, "%d %d", &state->player.y, &state->player.x)) {
		return 1;
	}
	state->player.x -= 1;
	state->player.y -= 1;
	return 0;
}

/**
 * Read a file and parse it as a board.
 */
#define MAX_FILE_SIZE 4095
Game board_from_file(char *path, bool old_fmt = false) {
	FILE *fp = fopen(path, "r");
	assert(fp != NULL);
	char *str = new char[MAX_FILE_SIZE+1];
	int len = fread(str, sizeof(char), MAX_FILE_SIZE, fp);
	fclose(fp);
	str[len] = '\0';
	Game board;
	int success;
	if(old_fmt) {
		success = board_from_string(str, &board);
	} else {
		success = board_from_new_fmt_string(str, &board);
	}
	assert(success == 0); // TODO better error handling
	//free(str);
	return board;
}

#endif
