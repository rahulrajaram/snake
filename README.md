## 1. Description

`snake.cpp` is a C++ program of the Snake game in C++ built using the [Ncurses library](https://www.gnu.org/software/ncurses/). It works on Linux/GNU and OS X and requires ncurses installed.

## 2. Execution

```bash
git clone https://github.com/rahulrajaram/snake.git
cd snake
g++ snake.cpp -std=c++11 -lncurses -lpanel -pthread -o snake
./snake
```

To exit from the game, perform `Ctrl+C`.