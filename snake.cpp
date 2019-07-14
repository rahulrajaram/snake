#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#include <ncurses.h>
#include <panel.h>

int myrandom (int i) { return std::rand()%i;}

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

class Coordinates {
public:
    int x, y;
    Coordinates(int x_, int y_) : x(x_), y(y_) {}
};

class Canvas {
    WINDOW *window;
public:
    unsigned char available_blue_coordinates;
    unsigned int score;
    int maxx, maxy, miny, minx;
    std::vector<Coordinates> potential_blue_coordinates;
    Canvas() {}
    Canvas(int y, int x) {
        this->available_blue_coordinates = 5;
        this->minx = 50;
        this->miny = 10;
        this->maxx = x - 50;
        this->maxy = y - 10;
        window = newwin(
            this->maxy - this->miny,
            this->maxx - this->minx,
            this->miny,
            this->minx);
        box(window, 0, 0);
        new_panel(window);
        update_panels();
        doupdate();
        wrefresh(window);
        refresh();
    }

    void print_score() {
        mvprintw(2, 2, "%s", "Score: ");
        mvprintw(2, 8, "%d", Canvas::score);
    }

    std::vector<Coordinates> generate_coordinates() {
        std::vector<Coordinates> coordinates;
        for (int j = this->miny + 1; j < this->maxy - 1; j ++) {
            for (int i = this->minx + 1; i < this->maxx - 1; i ++) {
                Coordinates c(j, i);
                coordinates.push_back(c);
            }
        }

        return coordinates;
    }
};

class Snake {
    std::vector<Coordinates> trace;
    bool _is_alive;
    bool _is_green;
    Canvas canvas;

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
            (y == this->canvas.minx)
                || (y == this->canvas.maxx - 1)
                || (x == this->canvas.miny)
                || (x == this->canvas.maxy - 1)
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

        bool is_blue_dot = next_position_color == COLOR_PAIR(ColorCode::BLACK_ON_BLUE);
        if (is_blue_dot) {
            this->canvas.available_blue_coordinates --;
            this->canvas.score ++;
        }

        if (this->canvas.available_blue_coordinates == 0) {
            this->canvas.available_blue_coordinates = 5;
attron(COLOR_PAIR(ColorCode::BLACK_ON_BLUE));
            std::random_shuffle(
                this->canvas.potential_blue_coordinates.begin(),
                this->canvas.potential_blue_coordinates.end(),
                myrandom);
            for (int i = 0; i < 5; i ++) {
                mvprintw(
                    this->canvas.potential_blue_coordinates[i].x,
                    this->canvas.potential_blue_coordinates[i].y,
                    "%s",
                    " ");
            }
attroff(COLOR_PAIR(ColorCode::BLACK_ON_BLUE));
        }
        return is_blue_dot;
    }

    void blacken_coordinate(Coordinates c) {
attron(COLOR_PAIR(ColorCode::BLACK_ON_BLACK));
        mvprintw(c.x, c.y, "%s", " ");
attroff(COLOR_PAIR(ColorCode::BLACK_ON_BLACK));
    }

    void blacken_snake() {
attron(COLOR_PAIR(ColorCode::BLACK_ON_BLACK));
        for (int i = 0; i < trace.size(); i ++) {
            mvprintw(trace[i].x, trace[i].y, "%s", " ");
        }
attroff(COLOR_PAIR(ColorCode::BLACK_ON_BLACK));
    }

    void green_snake() {
attron(COLOR_PAIR(ColorCode::BLACK_ON_GREEN));
        for (int i = 0; i < trace.size(); i ++) {
            mvprintw(trace[i].x, trace[i].y, "%s", " ");
        }
attroff(COLOR_PAIR(ColorCode::BLACK_ON_GREEN));
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
        this->canvas.print_score();
    }

public:
    Snake(Coordinates c, Canvas canvas) {
        this->trace.push_back(c);
        this->_is_alive = true;
        this->canvas = canvas;
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
            blacken_snake();
            this->_is_green = false;
        } else {
            green_snake();
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

Canvas init_curses() {
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
    Canvas c(row, col);

    return c;
}

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
    Canvas canvas = init_curses();
    std::vector<Coordinates> coordinates = canvas.generate_coordinates();
    std::srand ( unsigned ( std::time(0) ) );
    std::random_shuffle(coordinates.begin(), coordinates.end(), myrandom);
    ColorCode::initialize_all();
    std::vector<Coordinates> potential_blue_coordinates(
        coordinates.begin() + 11,
        coordinates.end() - 1);
    canvas.potential_blue_coordinates = potential_blue_coordinates;
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
    std::random_shuffle(
        canvas.potential_blue_coordinates.begin(),
        canvas.potential_blue_coordinates.end(),
        myrandom);
    for (int i = 0; i < 5; i ++) {
        mvprintw(
            canvas.potential_blue_coordinates[i].x,
            canvas.potential_blue_coordinates[i].y,
            "%s",
            " ");
    }
attroff(COLOR_PAIR(ColorCode::BLACK_ON_BLUE));

    Snake s(coordinates[coordinates.size() - 1], canvas);
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