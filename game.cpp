#include <vector>
#include <functional>

#ifndef GAME_H
#define GAME_H

/** *************************************************************************
 * Game Logic
 * ************************************************************************** */

struct State {
	virtual bool is_goal() = 0;
	virtual std::vector<State *> get_neighbors() = 0;
	virtual bool operator==(const State &other) const = 0;
	virtual size_t hash() const = 0;
};

/**
 * Two-dimensional coordinates, used for current player positions as well as 
 * actions (move left, right, up, down).
 */
struct Coord {
	int x;
	int y;
	Coord() {}
	Coord(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
	Coord operator+(const Coord &b) {
		Coord res;
		res.x = this->x + b.x;
		res.y = this->y + b.y;
		return res;
	}
	bool operator==(const Coord &b) const {
		return this->x == b.x && this->y == b.y;
	}
	bool operator<(const Coord &b) const {
		return x < b.x || (x == b.x && y < b.y);
	}
};


/**
 * The board is represented as a NxM (row-major) matrix of fields, each of which
 * can be an empty field, wall, box, box on goal or player.
 */
struct Board {
	enum Field {empty, wall, box, box_on_goal, goal};

	Coord dimensions;
	Field *fields;

	Board() {}

	Board(Coord dimensions, Field *fields) : 
		dimensions(dimensions), 
		fields(fields) {}

	/**
	 * Copy constructor.
	 */
	Board(const Board& obj) {
		this->dimensions = obj.dimensions;
		int n = this->dimensions.x*this->dimensions.y;
		this->fields = new Field[n];
		memcpy(this->fields, obj.fields, sizeof(Field)*n);
		assert(*this == obj);
	}

	/**
	 * Get the row-major index of a position on the given board.
	 */
	int get_index(Coord position) {
		return position.x + this->dimensions.x*position.y;
	}

	/**
	 * Return the board value at the given coordinates.
	 */
	Field get_field(Coord position) {
		return this->fields[this->get_index(position)];
	}

	/**
	 * Set field.
	 */
	void set_field(Coord position, Field value) {
		this->fields[this->get_index(position)] = value;
	}
	
	/**
	 * Compare whether two boards are equal.
	 */
	bool operator==(const Board &b) const {
		if(!(this->dimensions == b.dimensions)) {
			return false;
		}
		int n = this->dimensions.x * this->dimensions.y;
		for(int i = 0; i < n; i++) {
			if(this->fields[i] != b.fields[i]) {
				return false;
			}
		}
		return true;
	}

};

/**
 * The current game state is represented by the player position (X, Y) and the
 * current board state.
 */
struct Game : State {
	Coord player;
	Board board;

	Game() {}

	Game(Coord player, Board board) : player(player), board(board) {}

	/**
	 * Given the current board state, tell whether the desired action is legal.
	 */
	bool is_action_legal(Coord action) {
		Coord new_pos = this->player + action;
		if(new_pos.x >= this->board.dimensions.x || 
		   new_pos.y >= this->board.dimensions.y ||
		   0 > new_pos.x ||
		   0 > new_pos.y) {
			// Action would move player outside of board dimensions.
			return false;
		}
		Board::Field neighbor = this->board.get_field(new_pos);
		if(neighbor == Board::empty || neighbor == Board::goal) {
			// Player moves into empty or goal field.
			return true;
		}
		if(neighbor == Board::box || neighbor == Board::box_on_goal) {
			Board::Field neighbor_of_neighbor = this->board.get_field(new_pos + action);
			if(neighbor_of_neighbor == Board::empty || neighbor_of_neighbor == Board::goal) {
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
	bool is_goal() {
		for(int x = 0; x < this->board.dimensions.x; x++) {
			for(int y = 0; y < this->board.dimensions.y; y++) {
				Board::Field field = this->board.get_field((Coord){x, y});
				if(field != Board::empty 
				   && field != Board::wall
				   && field != Board::box_on_goal) {
					return false;
				}
			}
		}
		return true;
	}

	/**
	 * Apply the given action to the board.
	 *
	 * Returns 0 if no box was moved. Returns 1 if a box was moved into an empty
	 * field. Returns 2 if a box was moved into a goal.
	 */
	int take_action(Coord action) {
		int ret = 0;
		assert(this->is_action_legal(action));
		Coord neighbor = this->player + action;
		// Update player position (assuming action is legal).
		this->player.x = neighbor.x;
		this->player.y = neighbor.y;
		Board::Field field = this->board.get_field(neighbor);
		// If player is pushing box, remove box from that cell ...
		if(field == Board::box) {
			this->board.set_field(neighbor, Board::empty);
		} else if(field == Board::box_on_goal) {
			this->board.set_field(neighbor, Board::goal);
		}
		// ... and move it into the adjacent cell.
		if(field == Board::box || field == Board::box_on_goal) {
			Coord neighbor_of_neighbor = neighbor + action;
			Board::Field neighbor_field = this->board.get_field(neighbor_of_neighbor);
			if(neighbor_field == Board::goal) {
				this->board.set_field(neighbor_of_neighbor, Board::box_on_goal);
				ret = 2;
			} else if(neighbor_field == Board::empty) {
				this->board.set_field(neighbor_of_neighbor, Board::box);
				ret = 1;
			} else {
				// Illegal action.
				assert(false);
			}
		}
		return ret;
	}

	/**
	 * Returns true if the given board is obviously unsolvable. Deciding
	 * whether the game is unsolvable completely would require finding a
	 * solution. However, some obvious cases, such as non-goal boxes lodged
	 * against walls, can be determined more easily. Let's not waste 
	 * resources on those.
	 */
	bool is_obviously_unsolvable() {
		for(int x = 0; x < this->board.dimensions.x; x++) {
			for(int y = 0; y < this->board.dimensions.y; y++) {
				Coord pos = {x, y};
				Board::Field field = this->board.get_field(pos);
				if(field != Board::box) {
					continue;
				}
				// Box (not in goal) is lodged against corner of walls
				Board::Field left   = this->board.get_field(pos+(Coord){-1,  0});
				Board::Field right  = this->board.get_field(pos+(Coord){+1,  0});
				Board::Field top    = this->board.get_field(pos+(Coord){ 0, -1});
				Board::Field bottom = this->board.get_field(pos+(Coord){ 0, +1});
				if((left == Board::wall && top == Board::wall) ||
				   (left == Board::wall && bottom == Board::wall) ||
				   (right == Board::wall && top == Board::wall) ||
				   (right == Board::wall && bottom == Board::wall)) {
					return true;
				}
			}
		}
		return false;
	}

	/**
	 * Give all legal and not obviously unsolvable actions from current state.
	 */
	std::vector<State *> get_neighbors() {
		std::vector<State *> neighbors;
		Coord actions[4] = {(Coord){-1, 0},
				    (Coord){+1, 0},
				    (Coord){0, -1},
				    (Coord){0, +1}};
		for(int i = 0; i < 4; i++) {
			Coord action = actions[i];
			if(!this->is_action_legal(action)) {
				continue;
			}
			Game *neighbor = new Game(*this);
			neighbor->take_action(action);
			if(neighbor->is_obviously_unsolvable()) {
				continue;
			}
			neighbors.push_back(static_cast<State *>(neighbor));
		}
		return neighbors;
	}

	/**
	 * Compare whether two game objects represent the same state.
	 */
	bool operator==(const State& b) const {
		const Game &other = static_cast<const Game &>(b);
		return this->player == other.player && this->board == other.board;
	}

	/**
	 * Hash this state
	 */
	size_t hash() const {
		size_t hash = 0;
		std::hash<int> int_hasher;
		hash ^= int_hasher(this->player.x) ^ int_hasher(this->player.y);
		int n = this->board.dimensions.x * this->board.dimensions.y;
		for(int i = 0; i < n; i++) {
			hash ^= int_hasher(this->board.fields[i]);
		}
		return hash;
	}

};

#endif
