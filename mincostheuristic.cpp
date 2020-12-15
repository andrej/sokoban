#ifndef MINCOSTHEURISTIC_H
#define MINCOSTHEURISTIC_H

#include <cmath>
#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>
#include <map>
#include <vector>
#include <queue>
#include <iostream>
#include <algorithm>
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
	std::vector<int> goal_keys;
	std::vector<int> box_keys;
	std::vector<std::map<int, int>> distances_to_goals;
	matrix<int> box_goal_adjacency;
	std::map<int, int> goal_graph_to_adj_key;
	std::map<int, int> box_graph_to_adj_key;

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

	void display_coord_to_key(State &state)
	{
		Game &game = static_cast<Game &>(state);
		std::cout << "COORD TO KEY\n";
		for (auto& e: coord_to_key)
		{
			std::cout << e.second << "=>(" << e.first.x << ',' << e.first.y << ")=>" << game.board.get_field(e.first) << '\n';
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
				if (game.board.get_field(Coord(i, j)) == 4)
				{
					goal_keys.push_back(coord_to_key.at(Coord(i, j)));
				}
				if (game.board.get_field(Coord(i, j)) == 2)
				{
					box_keys.push_back(coord_to_key.at(Coord(i, j)));
				}
			}
		}
		//display_key_to_coord(game);
	}

	bool update_box_keys(State& state)
	{
		std::vector<int> new_box_keys;
		bool box_moved = false;
		Game &game = static_cast<Game &>(state);
		for (int i = 0; i < game.board.dimensions.x; ++i)
		{
			for (int j = 0; j < game.board.dimensions.y; ++j)
			{
				if (game.board.get_field(Coord(i, j)) == 2)
				{
					new_box_keys.push_back(coord_to_key.at(Coord(i, j)));
					if (std::find(box_keys.begin(), box_keys.end(), coord_to_key.at(Coord(i, j))) == box_keys.end())
					{
						box_moved = true;
					}
				}
			}
		}
		if (box_moved)
		{
			box_keys = new_box_keys;
			return true;
		}
		else
		{
			return false;
		}
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
		//std::cout << "Entered build_reverse_directed_graph\n";
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
		//std::cout << "DIMS: " << game.board.dimensions.x << ',' << game.board.dimensions.y << '\n';
		//std::cout << "Initializes matrix\n";
		for (auto& e: key_to_coord)
		{
			int x = e.second.x;
			int y = e.second.y;
			//std::cout << "for (" << x << ',' << y << ")\n";

			if (game.board.get_field(e.second) != 1)
			{
				//Coord above = Coord(x, y-1);
				//Coord below = Coord(x, y+1);
				//Coord left = Coord(x-1, y);
				//Coord right = Coord(x+1, y);	

				if (y > 0 && y < game.board.dimensions.y - 1 && (game.board.get_field(Coord(x, y+1)) == 0 || game.board.get_field(Coord(x, y+1)) == 2 || game.board.get_field(Coord(x, y+1)) == 4) && (game.board.get_field(Coord(x, y-1)) == 0 || game.board.get_field(Coord(x, y-1)) == 2 || game.board.get_field(Coord(x, y-1)) == 4))
				{
					//std::cout << "First\n";
					reverse_directed_graph(coord_to_key.at(Coord(x, y+1)), e.first) = 1;
					//std::cout << "Second\n";
					reverse_directed_graph(coord_to_key.at(Coord(x, y-1)), e.first) = 1;
				}
				if (x > 0 && x < game.board.dimensions.x - 1 && (game.board.get_field(Coord(x+1, y)) == 0 || game.board.get_field(Coord(x+1, y)) == 2 || game.board.get_field(Coord(x+1, y)) == 4) && (game.board.get_field(Coord(x-1, y)) == 0 || game.board.get_field(Coord(x-1, y)) == 2 || game.board.get_field(Coord(x-1, y)) == 4))
				{
					//std::cout << "Third\n";
					reverse_directed_graph(coord_to_key.at(Coord(x+1, y)), e.first) = 1;
					//std::cout << "Fourth\n";
					reverse_directed_graph(coord_to_key.at(Coord(x-1, y)), e.first) = 1;
				}				
			}
		}
		//display_reverse_directed_graph();
	}

	// void initialize_distances_matrix(int key)
	// {
	// 	int d = key_to_coord.size();
	// 	distances_to_goals.at(key) = matrix<int>(d, d);
	// 	for (unsigned i = 0; i < distances_to_goals.at(key).size1(); ++i)
	// 	{
	// 		for (unsigned j = 0; j < distances_to_goals.at(key).size2(); ++j)
	// 		{
	// 			distances_to_goals.at(key)(i, j) = +INFINITY;
	// 		}
	// 	}
	// }

	std::vector<int> get_children(int key)
	{
		std::vector<int> children;
		for (unsigned i = 0; i < reverse_directed_graph.size2(); ++i)
		{
			if (reverse_directed_graph(key, i) == 1)
			{
				children.push_back(i);
			}
		}
		return children;
	}

	void display_distances_to_goals()
	{
		for (auto& e: distances_to_goals.at(0))
		{
			Coord c = key_to_coord.at(e.first);
			std::cout << '(' << c.x << ',' << c.y << ")=>" << e.second;
		}		
	}

	void build_distances_to_goals()
	{
		//std::cout << "Begin distances to goals\n";
		//Game &game = static_cast<Game &>(state);
		for (std::vector<int>::iterator it = goal_keys.begin(); it < goal_keys.end(); it++)
		{
			//std::cout << "Entered goals loop\n";
			//int x = key_to_coord.at(*it).x;
			//int y = key_to_coord.at(*it).y;

			// initialize_distances_matrix(*it);

			std::queue<int> frontier;
			frontier.push(*it);
			//std::cout << "After init queue\n";

			std::map<int, bool> visited;
			for (auto& e: key_to_coord)
			{
				visited.insert({e.first, false});
			}
			//std::cout << "After init visited map\n";

			std::map<int, int> level;
			level.insert({*it, 0});

			//std::cout << "Right after first node\n";
			while (!frontier.empty())
			{
				int tile = frontier.front();
				frontier.pop();

				if (!visited.at(tile))
				{
					visited.at(tile) = true;
					//std::cout << "Is the problem here?\n";
					std::vector<int> children = get_children(tile);
					for (std::vector<int>::iterator itr = children.begin(); itr < children.end(); itr++)
					{
						level.insert({*itr, level.at(tile)+1});
						frontier.push(*itr);
					}
				}
			}
			distances_to_goals.push_back(level);
		}
		//display_distances_to_goals();
	}

	void display_box_goal_adjacency()
	{
		std::cout << "Box-Goal Adjacency Matrix\n";
		for (unsigned i = 0; i < box_goal_adjacency.size1(); ++i)
		{
			for (unsigned j = 0; j < box_goal_adjacency.size2(); ++j)
			{
				if (box_goal_adjacency(i, j) == 2147483647)
				{
					std::cout << "_inf";
				}	
				else 
				{
					std::cout << '_' << box_goal_adjacency(i, j);
				}
			}
			std::cout << '\n';
		}
	}

	void build_box_goal_adjacency(State& state)
	{
		//std::cout << "Entered build_box_goal_adjacency\n";
		Game &game = static_cast<Game &>(state);
		box_goal_adjacency = matrix<int>(box_keys.size(), goal_keys.size());
		//std::cout << "Created box-goal matrix\n";
		for (unsigned i = 0; i < goal_keys.size(); i++)
		{
			goal_graph_to_adj_key.insert({goal_keys.at(i), i});
		}
		for (unsigned i = 0; i < box_keys.size(); i++)
		{
			box_graph_to_adj_key.insert({box_keys.at(i), i});
		}
		//std::cout << "Initialized graph_to_adj maps\n";
		
		for (unsigned g = 0; g < goal_keys.size(); g++)
		{
			//std::cout << "For goal:" << g << '\n';
			for (unsigned b = 0; b < box_keys.size(); b++)
			{
				//std::cout << "For box:" << b << '\n';
				Coord box_coord = key_to_coord.at(box_keys.at(b));
				Coord goal_coord = key_to_coord.at(goal_keys.at(g));

				int box_graph_key = game.board.get_index(box_coord);
				int goal_graph_key = game.board.get_index(goal_coord);

				if (box_graph_to_adj_key.find(box_graph_key) != box_graph_to_adj_key.end())
				{
					box_graph_to_adj_key.at(box_graph_key) = b;
				}
				else
				{
					box_graph_to_adj_key.insert({box_graph_key, b});
				}
				if (goal_graph_to_adj_key.find(goal_graph_key) != goal_graph_to_adj_key.end())
				{
					goal_graph_to_adj_key.at(goal_graph_key) = g;
				}
				else
				{
					goal_graph_to_adj_key.insert({goal_graph_key, g});
				}

				if (distances_to_goals.at(g).find(box_graph_key) != distances_to_goals.at(g).end())
				{
					int dist = distances_to_goals.at(g).at(box_graph_key);
					box_goal_adjacency(b, g) = dist;
				}
				else
				{
					box_goal_adjacency(b, g) = 2147483647;
				}				
			}			
		}
		display_box_goal_adjacency();
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
			//std::cout << "Built key_to_coord\n";
			//display_coord_to_key(game);
			build_reverse_directed_graph(game);
			//std::cout << "Built reverse_directed_graph\n";
			build_distances_to_goals();
			//std::cout << "Built distances_to_goals\n";
			build_box_goal_adjacency(game);
			//std::cout << "Built box_goal_adjacency\n";
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