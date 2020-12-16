CXXFLAGS=-Wall -g -std=c++11

sokoban: sokoban.cpp search.cpp heuristic.cpp game.cpp io.cpp mincostheuristic.cpp
	$(CXX) $(CXXFLAGS) sokoban.cpp $(LDFLAGS) -o $@
