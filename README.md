# Artificial Intelligence Agent for the Game of Sokoban

This program solves levels of the game of 
[Sokoban](https://en.wikipedia.org/wiki/Sokoban) using A* search with a
minimum cost heuristic. The program also allows interactively playing the game 
on the command line.

![Screen recording showing solving of Sokoban level](screengrab.gif?raw=true)

In the game of Sokoban, the player (represented as symbol `x`) is charged 
with moving crates (`O`) to their destinations (`.`). Once all crates are at
their destinations (`0`), the game is won. Crates can only be moved by pushing.
Once a crate is pushed against a wall (`#`), it cannot be moved further in that
direction, or away from it (since the player cannot get between the crate and
the wall to push).

## Input

Input can be given in one of two formats, toggled by the `-l` flag.

### List of Coordinates (default)

The files `sokobanNN.txt` in the `new_lvls` folder show files in this format.
The format is as follows:

    W H                                                 (two integers describing width and height, respectively, of board)
    N_walls X_1 Y_1 X_2 Y_2 ... X_N Y_N                 (walls, each with two coordinates X_i, Y_i)
    N_boxes X'_1 Y'_1 X'_2 Y'_2 ... X'_N Y'_N           (boxes/crates, each with two coordinates X'_i, Y'_i)
    N_goals X''_1 Y''_1 X''_2 Y''_2 ... X''_N Y''_N     (destinations, each with two coordinates X''_i, Y''_i)
    P_x P_Y                                             (initial player position)

In the same `new_lvls` folder, there are also `visualNN.txt` files; these files
are simply visual representations of the corresponding `sokobanNN.txt` files.
To use them as input, use the `-l` flag, described below.

### Visual Format (use `-l` flag)

This format is more convenient if one wishes to directly build levels in a text 
editor. When supplying input using this format, the `-l` flag must be given.

Files using the old format can be found in the `old_lvls` directory. They use
the same representation as our game output in replay/interactive modes,
i.e. `x` for the initial player position, `#` for a wall, `O` for a crate, and
`.` for a destination. A trailing newline must be present in the input files.

## Building the Executable

In order to build this program, the boost C++ libraries must be available. On
MacOS with Homebrew, run

    brew install boost

to satisfy this prerequisite. Then, make sure you are in the sokoban directory 
and run 

    make sokoban

## Usage

By default, the executable takes a Sokoban level input file (see `new_lvls`
directory for example format) and returns a solution to the level as number
of steps, and each individual step (up, down, left, right).

To run the program with the minimum cost heuristic run this command

    ./sokoban [path to imput file] 

To run the program with the simple heuristic use the `-s` flag

    ./sokoban -s [path to input file]

Further usage information can be obtained by running the program without any
options:

    Usage: ./sokoban LEVEL [-p] [-s] [-v] [-r] [-l]
        LEVEL: Path to Sokoban level text file.
        -p: Play in interactive mode.
        -s: Use simple heuristic (for performance comparison).
        -v, -vv: Print (very) verbose output to stderr.
        -r: Replay solution after it has been found
        -l: Use alternative visual input format.

## Credits

The game representation, logic, A* algorithm and simple heuristic was
implemented by [André Rösti](http://www.andreroesti.com).

[Danny Ibrahim](https://github.com/Didanny) contributed the minimum cost 
heuristic based on the Hungarian algorithm.

The Hungarian algorithm implementation used is by 
[Cong Ma](https://github.com/mcximing/hungarian-algorithm-cpp), see
`hugarian-algorithm-cpp-master/LICENSE`.

The example levels in `new_lvls` are adapted from Dr. Kalev Kask's CS271
Introduction to Artificial Intelligence course at the University of California,
Irvine.