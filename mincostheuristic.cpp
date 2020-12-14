#ifndef MINCOSTHEURISTIC_H
#define MINCOSTHEURISTIC_H

#include <cmath>
#include <algorithm>
#include "game.cpp"
#include "search.cpp"

struct MinCostHeuristic: Heuristic
{
	double operator()(State &state) {
		Game &game = static_cast<Game &>(state);
		if(game.is_goal()) {
			return 0;
		}
		double player_to_box = +INFINITY;
		double box_to_goal = +INFINITY;
		for(int x0 = 0; x0 < game.board.dimensions.x; x0++) {
			for(int y0 = 0; y0 < game.board.dimensions.y; y0++) {
				Coord pos1 = {x0, y0};
				Board::Field field1 = game.board.get_field(pos1);
				if(field1 != Board::box) {
					continue;
				}
				for(int x1 = 0; x1 < game.board.dimensions.x; x1++) {
					for(int y1 = 0; y1 < game.board.dimensions.y; y1++) {
						Coord pos2 = {x1, y1};
						if(pos1 == pos2) {
							continue;
						}
						Board::Field field2 = game.board.get_field(pos2);
						double d = std::abs(pos1.x-pos2.x)
							   + std::abs(pos1.y-pos2.y);
						if(pos2 == game.player) {
							player_to_box = std::min(player_to_box, d);
						}
						if(field2 == Board::goal) {
							box_to_goal = std::min(box_to_goal, d);
						}
					}
				}
			}
		}
		return player_to_box + box_to_goal;
	}
};

#endif