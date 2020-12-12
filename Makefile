CXXFLAGS=-Wall -g

sokoban: sokoban.cpp search.cpp heuristic.cpp game.cpp
	$(CXX) $(CXXFLAGS) sokoban.cpp $(LDFLAGS) -o $@
