#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#include <ncurses.h>
#include <panel.h>
#include <boost/thread/thread.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

class Canvas {
    static int maxx, maxy, miny, minx;
    static WINDOW *window;
public:
    static void initialize(int y, int x) {
        minx = 50;
        miny = 10;
        maxx = x - 50;
        maxy = y - 10;
        window = newwin(maxy - miny, maxx - minx, miny, minx);
        box(window, 0, 0);
        new_panel(window);
        update_panels();
        doupdate();
        wrefresh(window);
        refresh();
    }
    static int get_maxx() {
        return maxx;
    };
    static int get_maxy() {
        return maxy;
    };
    static int get_minx() {
        return minx;
    };
    static int get_miny() {
        return miny;
    };
};
int Canvas::maxx = 0;
int Canvas::maxy = 0;
int Canvas::miny = 10;
int Canvas::minx = 10;
WINDOW *Canvas::window = 0;


class Coordinates {
public:
    int x, y;
    Coordinates(int x_, int y_) : x(x_), y(y_) {}

    static Coordinates random() {
        boost::random::mt19937 generator;
        generator.seed(static_cast<unsigned int>(std::time(0)));
        boost::random::uniform_int_distribution<> random_x(
            Canvas::get_minx(),
            Canvas::get_maxx());
        generator.seed(static_cast<unsigned int>(std::time(0)));
        boost::random::uniform_int_distribution<> random_y(
            Canvas::get_miny(),
            Canvas::get_maxy());
        Coordinates c(random_x(generator), random_y(generator));

        return c;
    }

    static std::vector<Coordinates> all() {
        std::vector<Coordinates> coordinates;
        for (int j = Canvas::get_miny() + 1; j < Canvas::get_maxy() - 1; j ++) {
            for (int i = Canvas::get_minx() + 1; i < Canvas::get_maxx() - 1; i ++) {
                Coordinates c(j, i);
                coordinates.push_back(c);
            }
        }

        return coordinates;
    }

    void print() {
        std::cout << "(" << this->y << ", " << this->x << ")\n";
    }
};

class Snake {
    std::vector<Coordinates> trace;
    bool _is_alive;
    bool _is_green;

    void blacken_current_position() {
        attron(COLOR_PAIR(1));
        mvprintw(trace[0].x, trace[0].y, "%s", " ");
        attroff(COLOR_PAIR(1));
    }

    void green_current_position(Coordinates c) {
        init_pair(5, COLOR_BLACK, COLOR_GREEN);
        attron(COLOR_PAIR(5));
        mvprintw(c.x, c.y, "%s", " ");
        attroff(COLOR_PAIR(5));
    }

    void green_current_position() {
        green_current_position(trace[0]);
    }

    bool can_advance_to_position(int x, int y) {
        if (!is_alive()) {
            return false;
        }

        const int next_position_color =
            mvinch(x, y) & A_COLOR;

        if (next_position_color == COLOR_PAIR(2)) {
            this->_is_alive = false;
            return false;
        }

        return true;
    }

public:
    Snake(Coordinates c) {
        this->trace.push_back(c);
        this->_is_alive = true;
        green_current_position(c);
    }

    void move_up() {
        if (!can_advance_to_position(trace[0].x - 1, trace[0].y)) {
            return;
        }
        blacken_current_position();
        trace[0].x --;
        green_current_position();
    }

    void move_down() {        
        if (!can_advance_to_position(trace[0].x + 1, trace[0].y)) {
            return;
        }
        blacken_current_position();
        trace[0].x ++;
        green_current_position();
    }

    void move_left() {
        if (!can_advance_to_position(trace[0].x, trace[0].y - 1)) {
            return;
        }
        blacken_current_position();
        trace[0].y --;
        green_current_position();
    }

    void move_right() {
        if (!can_advance_to_position(trace[0].x, trace[0].y + 1)) {
            return;
        }
        blacken_current_position();
        trace[0].y ++;
        green_current_position();
    }

    void blink() {
        const int this_position_color =
            mvinch(trace[0].x, trace[0].y) & A_COLOR;

        if (this->_is_green) {
            blacken_current_position();
            this->_is_green = false;
        } else {
            green_current_position();
            this->_is_green = true;
        }
        refresh();
    }

    bool is_alive() {
        return _is_alive;
    }
};

void exit_unless_colors_are_supported() {
    if (has_colors() == false) {
        endwin();
        printf("This Canvas does not support colors\n");
        std::exit(1);
    }
}

void exit_if_cannot_change_color() {
    if (!can_change_color()) {
        std::cout << "This Canvas does not support colors\n";
        std::exit(1);
    }
}

void init_curses() {
    initscr();
    curs_set(0);
    nonl();
    intrflush(stdscr, false);
    keypad(stdscr, true);
    exit_unless_colors_are_supported();
    exit_if_cannot_change_color();
    start_color();
    cbreak();
    mousemask(ALL_MOUSE_EVENTS, NULL);
    keypad(stdscr, true);
    noecho();
    int row, col;
    getmaxyx(stdscr, row, col);
    Canvas::initialize(row, col);
}

int myrandom (int i) { return std::rand()%i;}

int main(int argc, char* argv[]) {
    init_curses();
    std::vector<Coordinates> coordinates = Coordinates::all();
    int ch;
    std::srand ( unsigned ( std::time(0) ) );
    std::random_shuffle(coordinates.begin(), coordinates.end(), myrandom);
    init_pair(1, COLOR_BLACK, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    for (int i = 11; i < coordinates.size(); i ++) {
        mvprintw(coordinates[i].x, coordinates[i].y, "%s", " ");
    }
    attroff(COLOR_PAIR(1));

    init_pair(2, COLOR_BLACK, COLOR_RED);
    attron(COLOR_PAIR(2));
    for (int i = 0; i < 5; i ++) {
        mvprintw(coordinates[i].x, coordinates[i].y, "%s", " ");
    }
    attroff(COLOR_PAIR(2));
    refresh();

    init_pair(4, COLOR_BLACK, COLOR_BLUE);
    attron(COLOR_PAIR(4));
    for (int i = 5; i < 10; i ++) {
        mvprintw(coordinates[i].x, coordinates[i].y, "%s", " ");
    }
    init_color(COLOR_BLUE, 0, 0, 1000);
    attroff(COLOR_PAIR(4));

    Snake s(coordinates[11]);

    refresh();
    int count = 0;
    while (s.is_alive() && (ch = getch()) != 27) {
        if (ch == KEY_UP) {
            s.move_up();
            refresh();
        } else if (ch == KEY_DOWN) {
            s.move_down();
            refresh();
        } else if (ch == KEY_LEFT) {
            s.move_left();
            refresh();
        } else if (ch == KEY_RIGHT) {
            s.move_right();
            refresh();
        }
    }

    while (!s.is_alive()) {
        s.blink();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        refresh();
    }
    endwin();
    return 0;
}