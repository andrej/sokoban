#ifndef MINCOSTHEURISTIC_H
#define MINCOSTHEURISTIC_H

#include <cmath>
#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>
#include <map>
#include <iostream>
#include "game.cpp"
#include "search.cpp"
#include "io.cpp"

using namespace boost::numeric::ublas;

struct MinCostHeuristic: Heuristic
{
    bool start;
	matrix<int> reverse_directed_graph;
	std::map<int, Coord> key_to_coord;

    MinCostHeuristic() : Heuristic()
	{
		start = true;
	}

	void display_key_to_coord()
	{
		std::cout << "KEY TO COORD\n";
		for (auto& e: key_to_coord)
		{
			std::cout << e.first << "=>(" << e.second.x << ',' << e.second.y << ")\n";
		}
	}

	void build_key_to_coord(State &state)
	{	
		Game &game = static_cast<Game &>(state);
		for (int i = 0; i < game.board.dimensions.x; ++i)
		{
			for (int j = 0; j < game.board.dimensions.y; ++j)
			{
				key_to_coord.insert({game.board.get_index(Coord(i, j)), Coord(i, j)});
			}
		}
		//display_key_to_coord();
	}

	double operator()(State &state) {
		Game &game = static_cast<Game &>(state);
		if(game.is_goal()) {
			return 0;
		}

		if(start)
		{
			start = false;
			build_key_to_coord(game);
			std::cout << board_to_string(game);
			return +INFINITY;
		}
		throw "STOP";
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