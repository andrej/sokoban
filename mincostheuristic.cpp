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

//2147483647

using namespace boost::numeric::ublas;

struct MinCostHeuristic: Heuristic
{
    bool start;
	matrix<int> reverse_directed_graph;
	std::map<int, Coord> key_to_coord;
	std::map<Coord, int> coord_to_key;

    MinCostHeuristic() : Heuristic()
	{
		start = true;
	}

	void display_key_to_coord(State &state)
	{
		Game &game = static_cast<Game &>(state);
		std::cout << "KEY TO COORD\n";
		for (auto& e: key_to_coord)
		{
			std::cout << e.first << "=>(" << e.second.x << ',' << e.second.y << ")=>" << game.board.get_field(e.second) << '\n';
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
				coord_to_key.insert({Coord(i, j), game.board.get_index(Coord(i, j))});
			}
		}
		//display_key_to_coord(game);
	}

	void display_reverse_directed_graph()
	{
		for (unsigned i = 0; i < reverse_directed_graph.size1(); ++i)
		{
			for (unsigned j = 0; j < reverse_directed_graph.size2(); ++j)
			{
				if (reverse_directed_graph(i, j) != 1) 
				{
					//std::cout << '-';
				}
				else 
				{
					Coord c1 = key_to_coord.at(j);
					Coord c2 = key_to_coord.at(i);
					std::cout << '(' << c1.x << ',' << c1.y << ")=>(" << c2.x << ',' << c2.y << ")\n";
					//std::cout << reverse_directed_graph(i, j);
				}				
			}
			//std::cout << '\n';
		}
	}

	void build_reverse_directed_graph(State &state)
	{
		Game &game = static_cast<Game &>(state);
		int d = key_to_coord.size();
		reverse_directed_graph = matrix<int>(d, d);
		for (unsigned i = 0; i < reverse_directed_graph.size1(); ++i)
		{
			for (unsigned j = 0; j < reverse_directed_graph.size2(); ++j)
			{
				reverse_directed_graph(i, j) = +INFINITY;
			}
		}
		for (auto& e: key_to_coord)
		{
			int x = e.second.x;
			int y = e.second.y;

			if (game.board.get_field(e.second) != 1)
			{
				//Coord above = Coord(x, y-1);
				//Coord below = Coord(x, y+1);
				//Coord left = Coord(x-1, y);
				//Coord right = Coord(x+1, y);	

				if (y > 0 && y < game.board.dimensions.y && (game.board.get_field(Coord(x, y+1)) == 0 || game.board.get_field(Coord(x, y+1)) == 2 || game.board.get_field(Coord(x, y+1)) == 4) && (game.board.get_field(Coord(x, y-1)) == 0 || game.board.get_field(Coord(x, y-1)) == 2 || game.board.get_field(Coord(x, y-1)) == 4))
				{
					reverse_directed_graph(coord_to_key.at(Coord(x, y+1)), e.first) = 1;
					reverse_directed_graph(coord_to_key.at(Coord(x, y-1)), e.first) = 1;
				}
				if (x > 0 && x < game.board.dimensions.x && (game.board.get_field(Coord(x+1, y)) == 0 || game.board.get_field(Coord(x+1, y)) == 2 || game.board.get_field(Coord(x+1, y)) == 4) && (game.board.get_field(Coord(x-1, y)) == 0 || game.board.get_field(Coord(x-1, y)) == 2 || game.board.get_field(Coord(x-1, y)) == 4))
				{
					reverse_directed_graph(coord_to_key.at(Coord(x+1, y)), e.first) = 1;
					reverse_directed_graph(coord_to_key.at(Coord(x-1, y)), e.first) = 1;
				}				
			}
		}
		//display_reverse_directed_graph();
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
			build_reverse_directed_graph(game);
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