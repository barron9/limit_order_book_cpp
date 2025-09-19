#include <ncurses.h>
#include <deque>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <unistd.h>  // sleep
#include <cstdlib>   // rand

const int GRAPH_HEIGHT = 20;
const int GRAPH_WIDTH = 40;
const int MAX_POINTS = GRAPH_WIDTH;

std::deque<double> price_history;

double get_fake_price() {
    static double price = 100.0;
    double change = ((std::rand() % 200) - 100) / 100.0;  // -1.0 to +1.0
    price += change;
    return price;
}

void draw_order_book(WINDOW* win, const MatchingEngine& engine) {
    wclear(win);

    mvwprintw(win, 0, 1, " Buy Orders ");
    int row = 1;
    for (const auto& [price, queue] : engine.get_bids()) {
        mvwprintw(win, row++, 1, "Price: %.2f Qty: %d", price, queue.front().quantity);
    }

    mvwprintw(win, row++, 1, " Sell Orders ");
    for (const auto& [price, queue] : engine.get_asks()) {
        mvwprintw(win, row++, 1, "Price: %.2f Qty: %d", price, queue.front().quantity);
    }

    box(win, 0, 0);
    wrefresh(win);
}
void draw_price_graph(WINDOW* win, const std::vector<double>& prices) {
    int height, width;
    getmaxyx(win, height, width);
    wclear(win);

    double max_price = *std::max_element(prices.begin(), prices.end());
    double min_price = *std::min_element(prices.begin(), prices.end());

    for (size_t i = 0; i < prices.size(); ++i) {
        int x = i;
        double price = prices[i];
        int y = (int)((price - min_price) / (max_price - min_price + 1e-5) * (height - 2));
        y = height - 2 - y;
        if (x < width - 2 && y >= 0)
            mvwaddch(win, y, x, '*');
    }

    box(win, 0, 0);
    wrefresh(win);
}



int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    int screen_rows, screen_cols;
    getmaxyx(stdscr, screen_rows, screen_cols);

    int order_win_width = screen_cols - GRAPH_WIDTH - 4;

    WINDOW* order_win = newwin(GRAPH_HEIGHT, order_win_width, 1, 1);
    WINDOW* graph_win = newwin(GRAPH_HEIGHT, GRAPH_WIDTH, 1, order_win_width + 2);

    box(order_win, 0, 0);
    mvwprintw(order_win, 0, 2, " Order Book ");
    mvwprintw(order_win, 1, 1, "Buy: ID:1 Q:10 P:100");
    mvwprintw(order_win, 2, 1, "Sell: ID:3 Q:5 P:101");
    wrefresh(order_win);

 while (true) {
    // Accept new orders via Boost.Asio or input
    // For example:
    // engine.add_order(price, qty, side);
    // price_history.push_back(price);

    draw_order_book(order_book_win, engine);
    draw_price_graph(graph_win, price_history);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

    delwin(order_win);
    delwin(graph_win);
    endwin();
    return 0;
}
