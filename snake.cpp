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

class ColorCode {
public:
    static int BLACK_ON_GREEN;
    static int BLACK_ON_RED;
    static int BLACK_ON_BLUE;
    static int BLACK_ON_BLACK;

    static void initialize_all() {
        init_pair(BLACK_ON_GREEN, COLOR_BLACK, COLOR_GREEN);
        init_pair(BLACK_ON_RED, COLOR_BLACK, COLOR_RED);
        init_pair(BLACK_ON_BLUE, COLOR_BLACK, COLOR_BLUE);
        init_pair(BLACK_ON_BLACK, COLOR_BLACK, COLOR_BLACK);
    }
};
int ColorCode::BLACK_ON_BLACK = 1;
int ColorCode::BLACK_ON_RED = 2;
int ColorCode::BLACK_ON_BLUE = 4;
int ColorCode::BLACK_ON_GREEN = 5;

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
        blacken_coordinate(trace[0]);
    }

    void green_current_position(Coordinates c) {
attron(COLOR_PAIR(ColorCode::BLACK_ON_GREEN));
        mvprintw(c.x, c.y, "%s", " ");
attroff(COLOR_PAIR(ColorCode::BLACK_ON_GREEN));
    }

    void green_current_position() {
        green_current_position(trace[0]);
    }

    bool can_advance_to_position(int x, int y) {
        if (!is_alive()) {
            return false;
        }

        if (
            (y == Canvas::get_minx())
                || (y == Canvas::get_maxx() - 1)
                || (x == Canvas::get_miny())
                || (x == Canvas::get_maxy() - 1)
        ){
            this->_is_alive = false;
            return false;
        }
        const int next_position_color =
            mvinch(x, y) & A_COLOR;

        if (next_position_color == COLOR_PAIR(ColorCode::BLACK_ON_RED)) {
            this->_is_alive = false;
            return false;
        }

        return true;
    }

    bool can_advance_to_position(Coordinates c) {
        return can_advance_to_position(c.x, c.y);
    }

    bool should_grow(Coordinates c) {
        const int next_position_color =
            mvinch(c.x, c.y) & A_COLOR;

        return next_position_color == COLOR_PAIR(ColorCode::BLACK_ON_BLUE);
    }

    void blacken_coordinate(Coordinates c) {
attron(COLOR_PAIR(ColorCode::BLACK_ON_BLACK));
        mvprintw(c.x, c.y, "%s", " ");
attroff(COLOR_PAIR(ColorCode::BLACK_ON_BLACK));
    }

    void redraw(Coordinates next_coordinate) {
        if (!can_advance_to_position(next_coordinate)) {
            return;
        }
        if (!should_grow(next_coordinate)) {
            Coordinates last = trace[0];
            blacken_coordinate(last);
            trace.erase(trace.begin());
        }

        trace.push_back(next_coordinate);

        for (int i = 0; i <trace.size(); i ++) {
            green_current_position(trace[i]);
        }
    }

public:
    Snake(Coordinates c) {
        this->trace.push_back(c);
        this->_is_alive = true;
        green_current_position(c);
    }

    void move_up() {
        Coordinates next_coordinate(trace[trace.size()-1].x - 1, trace[trace.size()-1].y);
        redraw(next_coordinate);
    }

    void move_down() {
        Coordinates next_coordinate(trace[trace.size()-1].x + 1, trace[trace.size()-1].y);
        redraw(next_coordinate);
    }

    void move_left() {
        Coordinates next_coordinate(trace[trace.size()-1].x, trace[trace.size()-1].y - 1);
        redraw(next_coordinate);
    }

    void move_right() {
        Coordinates next_coordinate(trace[trace.size()-1].x, trace[trace.size()-1].y + 1);
        redraw(next_coordinate);
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

class KeyContext {
    bool _is_alive;
public:
    int current_key;
    KeyContext() : _is_alive(true), current_key(999999) {}
    void manage() {
        current_key = getch();
        while (this->_is_alive) {
            this->current_key = getch();

            if (this->current_key == 27) {
                kill();
                return;
            }
        }
    }

    void kill() {
        this->_is_alive = false;
    }
};

int main(int argc, char* argv[]) {
    init_curses();
    std::vector<Coordinates> coordinates = Coordinates::all();
    std::srand ( unsigned ( std::time(0) ) );
    std::random_shuffle(coordinates.begin(), coordinates.end(), myrandom);
    ColorCode::initialize_all();
attron(COLOR_PAIR(ColorCode::BLACK_ON_BLACK));
    for (int i = 11; i < coordinates.size(); i ++) {
        mvprintw(coordinates[i].x, coordinates[i].y, "%s", " ");
    }
attroff(COLOR_PAIR(ColorCode::BLACK_ON_BLACK));

attron(COLOR_PAIR(ColorCode::BLACK_ON_RED));
    for (int i = 0; i < 5; i ++) {
        mvprintw(coordinates[i].x, coordinates[i].y, "%s", " ");
    }
attroff(COLOR_PAIR(ColorCode::BLACK_ON_RED));

attron(COLOR_PAIR(ColorCode::BLACK_ON_BLUE));
    for (int i = 5; i < 10; i ++) {
        mvprintw(coordinates[i].x, coordinates[i].y, "%s", " ");
    }
attroff(COLOR_PAIR(ColorCode::BLACK_ON_BLUE));

    Snake s(coordinates[11]);
    refresh();
    KeyContext kc;
    std::thread key_context_thread(&KeyContext::manage, &kc);
    while (s.is_alive()) {
        if (kc.current_key == KEY_UP) {
            s.move_up();
        } else if (kc.current_key == KEY_DOWN) {
            s.move_down();
        } else if (kc.current_key == KEY_LEFT) {
            s.move_left();
        } else if (kc.current_key == KEY_RIGHT) {
            s.move_right();
        } else if (kc.current_key == 27) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        refresh();
    }
    kc.kill();
    while (!s.is_alive()) {
        s.blink();
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    key_context_thread.join();
    endwin();
    return 0;
}