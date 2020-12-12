#include <cstdio>
#include <cmath>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <boost/heap/fibonacci_heap.hpp>
#include "io.cpp"

#ifndef SEARCH_H
#define SEARCH_H


struct Action {};

template<>
struct std::hash<State> {
	size_t operator()(const State& s) const {
		return s.hash();
	}
};

template<>
struct std::hash<Game> {
	size_t operator()(const State& s) const {
		return s.hash();
	}
};

struct Heuristic {
	virtual double operator()(State &state) = 0;
};

struct PrioritizedState {
	double priority;
	State *state;
	PrioritizedState(double priority, State *state) :
	priority(priority), state(state) {
	}
	bool operator<(const PrioritizedState &other) const {
		// We flip the sign since fibonacci_heap is a max-heap, but
		// we need the minimum value to have highest priority.
		return this->priority > other.priority;
	}
};

/**
 * The following pointer set class provides basic set functionality with a
 * twist; we store pointers to objects, but equality is checked on the
 * dereferenced pointers.
 */
template<typename T>
struct PointerSet {
	std::unordered_map<size_t, std::vector<T *> > data;
	PointerSet() : data() {}
	size_t count(T obj) {
		return (this->find(obj) == NULL ? 0 : 1);
	}

	void insert(T *obj) {
		std::hash<T> hasher;
		size_t h = hasher(*obj); // Hash is calculated on dereferenced value
		if(this->count(*static_cast<Game *>(obj))) {
			return;
		}
		if(!this->data.count(h)) {
			//std::vector<T *> objs;
			//objs.push_back(obj);
			this->data[h].push_back(obj);
		} else {
			this->data[h].push_back(obj);
		}
	}

	T *find(T obj) {
		std::hash<T> hasher;
		size_t h = hasher(obj); // Hash is calculatedon  dereferenced value
		if(!this->data.count(h)) {
			return NULL;
		}
		for(typename std::vector<T *>::iterator it = this->data[h].begin(); it != this->data[h].end(); ++it) {
			if(**it == obj) {
				return *it;
			}
		}
		return NULL;
	}
};

/**
 * A* search. Returns an array of actions to take, starting from initial state
 * to reach a goal state.
 */
std::vector<State *> A_star(State &start, Heuristic &heuristic) {

	boost::heap::fibonacci_heap<PrioritizedState> todo;  // Nodes to be visited
	PointerSet<Game> visited; // Set of all visited nodes
	std::unordered_map<State *, State *> predecessor;  // Predecessor on shortest path to given state
	std::unordered_map<State *, double> g; // g: Cost of shortest path to State
	State *goal = NULL;

	g[&start] = 0;
	todo.push(PrioritizedState(heuristic(start), &start));
	double best = +INFINITY;

	while(!todo.empty()) {
		PrioritizedState prio_current = todo.top();
		todo.pop();
		State *current = prio_current.state;
		if(current->is_goal()) {
			goal = current;
			break;
		}
		std::vector<State *> neighbors = current->get_neighbors();
		for(std::vector<State *>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
			visited.insert(static_cast<Game *>(*it));
			State *neighbor = visited.find(*static_cast<Game *>(*it));
			double old_g = (g.count(neighbor) ? g[neighbor] : INFINITY);
			double tentative_g = g[current] + 1;
			if(tentative_g < old_g) {
				double h = heuristic(*neighbor);
				if(h <= best) {
					best = h;
					char *viz = board_to_string(*static_cast<Game *>(neighbor));
					printf("%f\n%s\n", best, viz);
				}
				predecessor[neighbor] = current;
				g[neighbor] = tentative_g;
				double f = tentative_g + h;
				todo.push(PrioritizedState(f, neighbor));
			}
		}
		//delete current;
	}

	std::vector<State *> out;
	if(goal) {
		do {
			out.push_back(goal);
			goal = predecessor[goal];
		} while(predecessor.count(goal));
		std::reverse(out.begin(), out.end());
	}
	return out;

}

#endif
