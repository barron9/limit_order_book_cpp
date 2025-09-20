#include <deque>
#include <vector>
#include <chrono>
#include <thread>
#include "matching_engine.hpp"
#include <ncurses.h>
#undef timeout
#undef stdscr

#include <iomanip>
#include <sstream>
#include "Server.hpp"
// You may need these includes for sorting:
#include <map>
#include <vector>
#include <algorithm>
// Constants for layout
const int ORDER_BOOK_HEIGHT = 30;
const int ORDER_BOOK_WIDTH = 50;
const int GRAPH_HEIGHT = 20;
const int GRAPH_WIDTH = 50;
const int TRADES_HEIGHT = 30;
const int TRADES_WIDTH = 40;


// void draw_price_graph(WINDOW* win, const std::vector<double>& prices) {
//     werase(win);
//     box(win, 0, 0);
//     mvwprintw(win, 0, 2, " Price Chart (6h Window) ");

//     if (prices.empty()) {
//         wrefresh(win);
//         return;
//     }

//     int height, width;
//     getmaxyx(win, height, width);

//     int graph_height = height - 2;
//     int graph_width = width - 2;

//     // Total points for 6 hours (3 sec interval)
//     const int full_window_points = 6 * 60 * 60 / 3; // = 7200
//     int total_points = std::min((int)prices.size(), full_window_points);

//     // Calculate bucket size
//     int bucket_count = graph_width;
//     int bucket_size = std::max(total_points / bucket_count, 1);

//     // Determine start index to show latest 6 hours
//     int start_index = prices.size() > full_window_points
//                       ? prices.size() - full_window_points
//                       : 0;

//     // Prepare aggregated data
//     std::vector<double> aggregated_prices;

//     for (int i = 0; i < bucket_count; ++i) {
//         int bucket_start = start_index + i * bucket_size;
//         int bucket_end = std::min(bucket_start + bucket_size, (int)prices.size());

//         if (bucket_start >= bucket_end) break;

//         // Aggregate method: average
//         double sum = 0;
//         for (int j = bucket_start; j < bucket_end; ++j) {
//             sum += prices[j];
//         }
//         double avg = sum / (bucket_end - bucket_start);
//         aggregated_prices.push_back(avg);
//     }

//     // Normalize aggregated prices
//     double min_price = *std::min_element(aggregated_prices.begin(), aggregated_prices.end());
//     double max_price = *std::max_element(aggregated_prices.begin(), aggregated_prices.end());
//     double range = std::max(max_price - min_price, 1.0);

//     // Draw graph
//     for (int i = 0; i < aggregated_prices.size(); ++i) {
//         int x = 1 + i;

//         double normalized = (aggregated_prices[i] - min_price) / range;
//         int y = graph_height - static_cast<int>(normalized * graph_height); // Invert y

//         for (int fy = graph_height; fy >= y; --fy) {
//             mvwaddch(win, fy, x, ACS_CKBOARD); // or '█'
//         }
//     }

//     wrefresh(win);
// }

void draw_price_graph(WINDOW* win, const std::vector<double>& prices, const std::vector<time_t>& timestamps) {
    werase(win);

    box(win, 0, 0);
    mvwprintw(win, 0, 2, " Price Chart (6h Window) ");

    if (prices.empty()) {
        wrefresh(win);
        return;
    }

    int height, width;
    getmaxyx(win, height, width);

    const int left_margin = 8;  // for price labels
    const int bottom_margin = 2; // for time labels
    int graph_height = height - 2 - bottom_margin;
    int graph_width = width - 2 - left_margin;

    const int full_window_points = 6 * 60 * 60 / 3; // 7200
    int total_points = std::min((int)prices.size(), full_window_points);
    int bucket_count = graph_width;
    int bucket_size = std::max(total_points / bucket_count, 1);

    int start_index = prices.size() > full_window_points
                      ? prices.size() - full_window_points
                      : 0;

    std::vector<double> aggregated_prices;
    std::vector<time_t> aggregated_times;

    for (int i = 0; i < bucket_count; ++i) {
        int bucket_start = start_index + i * bucket_size;
        int bucket_end = std::min(bucket_start + bucket_size, (int)prices.size());

        if (bucket_start >= bucket_end) break;

        double sum = 0;
        for (int j = bucket_start; j < bucket_end; ++j) {
            sum += prices[j];
        }
        double avg = sum / (bucket_end - bucket_start);
        aggregated_prices.push_back(avg);

        // Take middle timestamp of bucket
        time_t t = timestamps[bucket_start + (bucket_end - bucket_start) / 2];
        aggregated_times.push_back(t);
    }

    double min_price = *std::min_element(aggregated_prices.begin(), aggregated_prices.end());
    double max_price = *std::max_element(aggregated_prices.begin(), aggregated_prices.end());
    double range = std::max(max_price - min_price, 1.0);

    // Draw Y-axis price labels
    int label_steps = 5;
    for (int i = 0; i <= label_steps; ++i) {
        int y = 1 + i * (graph_height) / label_steps;
        double price = max_price - i * (range) / label_steps;

        // Format and print price on left margin
        mvwprintw(win, y, 1, "%6.2f", price);
    }

    // Draw bars
    for (int i = 0; i < aggregated_prices.size(); ++i) {
        int x = left_margin + i;

        double normalized = (aggregated_prices[i] - min_price) / range;
        int y = graph_height - static_cast<int>(normalized * graph_height);

        for (int fy = graph_height; fy >= y; --fy) {
            mvwaddch(win, fy + 1, x, ACS_CKBOARD); // +1 to skip top border
        }
    }

    // Draw X-axis time labels
    int time_label_interval = std::max((int)aggregated_times.size() / 5, 1);
    for (int i = 0; i < aggregated_times.size(); i += time_label_interval) {
        int x = left_margin + i;

        char time_str[6];
        std::strftime(time_str, sizeof(time_str), "%H:%M", std::localtime(&aggregated_times[i]));

        mvwprintw(win, graph_height + 2, x - 2, "%s", time_str); // +2 for graph + border + label
    }

    wrefresh(win);
}

void draw_latest_trades(WINDOW* win, const MatchingEngine& engine) {
    werase(win);
    box(win, 0, 0);
    wrefresh(win);
    mvwprintw(win, 0, 2, "Latest Trades");

    const auto& trades = engine.get_latest_trades();

    int y = 1;
    for (const auto& trade : trades) {
        if (y >= getmaxy(win) - 1) break;
        mvwprintw(win, y++, 1, "%s", trade.c_str());
    }
    wrefresh(win);
}


void draw_order_book(WINDOW* win, const MatchingEngine& engine) {
    werase(win);
    box(win, 0, 0);
    wrefresh(win);
    int width, height;
    getmaxyx(win, height, width);

    mvwprintw(win, 1, (width / 2) - 10, "ORDER BOOK");

    mvwprintw(win, 3, 2,  "SELL ORDERS");
    mvwprintw(win, 3, width / 2 + 2, "BUY ORDERS");

    // Display only top 10
    int row = 5;
    int max_rows = std::min(20, height - row - 2);

    // Convert maps to sorted vectors
    auto bids_map = engine.get_bids(); // assume: std::map<double, std::queue<int>>
    auto asks_map = engine.get_asks();

    std::vector<std::pair<double, int>> bids;
    for (const auto& [price, queue] : bids_map) {
        int total_qty = 0;
        std::queue<Order> copy = queue; // copy constructor

        while (!copy.empty()) {
            total_qty += copy.front().quantity;
            copy.pop();
        }
        bids.emplace_back(price, total_qty);
    }

    std::vector<std::pair<double, int>> asks;
    for (const auto& [price, queue] : asks_map) {
        int total_qty = 0;
        std::queue<Order> copy = queue; // copy constructor
        
        while (!copy.empty()) {
            total_qty += copy.front().quantity;
            copy.pop();
        }
        asks.emplace_back(price, total_qty);
    }

    // Sort: buys descending, sells ascending
    std::sort(bids.rbegin(), bids.rend());
    std::sort(asks.begin(), asks.end());

    // Display asks (sells)
    for (int i = 0; i < std::min(max_rows, (int)asks.size()); ++i) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2)
           << "P: " << asks[i].first << " Q: " << asks[i].second;
        wattron(win, COLOR_PAIR(2));  // Turn on red color
        mvwprintw(win, row + i, 2, ss.str().c_str());
        wattroff(win, COLOR_PAIR(2));  // Turn on red color
    }

    // Display bids (buys)
    for (int i = 0; i < std::min(max_rows, (int)bids.size()); ++i) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2)
           << "P: " << bids[i].first << " Q: " << bids[i].second;
        wattron(win, COLOR_PAIR(1));  // Turn on green color
        mvwprintw(win, row + i, width / 2 + 2, ss.str().c_str());
        wattroff(win, COLOR_PAIR(1));  // Turn on green color
    }

    wrefresh(win);
}


int main() {
    initscr();
    start_color();          // Enable color functionality
    use_default_colors();   // Allow use of terminal's default colors (optional)
    noecho();
    curs_set(0);
  
    std::srand(std::time(nullptr));  // Seed rand() with current time
    if (!has_colors()) {
        endwin();
        std::cerr << "Your terminal does not support color\n";
        exit(1);
    }
    init_pair(1, COLOR_GREEN, -1); // Green text
    init_pair(2, COLOR_RED, -1);   // Red text
    // Create windows
    WINDOW* order_book_win = newwin(ORDER_BOOK_HEIGHT, ORDER_BOOK_WIDTH, 0, 0);
    WINDOW* graph_win = newwin(GRAPH_HEIGHT, GRAPH_WIDTH, 0, ORDER_BOOK_WIDTH + 2);
    WINDOW* trades_win = newwin(TRADES_HEIGHT, TRADES_WIDTH, 0, ORDER_BOOK_WIDTH + GRAPH_WIDTH + 2);
    // Initialize engine and price history
   // MatchingEngine engine;
    std::deque<double> price_history;

    // // Dummy initial orders
    // engine.add_order(100.0, 10, "buy");
    // engine.add_order(101.0, 5, "buy");
    // engine.add_order(102.0, 5, "sell");
    price_history.push_back(101.0);
    boost::asio::io_context io_context;
    MatchingEngine engine;  // OK here
    try {
        std::thread io_thread([&io_context,&engine]() {
            try {
                Server server(engine, io_context, 8080);
                io_context.run();
            } catch (const std::exception& e) {
                std::cerr << "Exception in io_context thread: " << e.what() << std::endl;
            }
        });
        // 🎉 Now you're free to do other non-blocking work here!
        //std::cout << "Server is running asynchronously..." << std::endl;
  
        std::vector<time_t> timestamps;
        // Main loop
        while (true) {
            timestamps.push_back(std::time(nullptr));
            draw_order_book(order_book_win, engine);

            // Keep last 80 prices
            if (price_history.size() > GRAPH_WIDTH - 2) {
                price_history.pop_front();
            }

            std::vector<double> prices(price_history.begin(), price_history.end());
            draw_price_graph(graph_win, prices, timestamps);
            draw_latest_trades(trades_win, engine);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            // Simulate live trading (random orders)
            double new_price = prices.back() + ((std::rand() % 101) - 50) / 10.0;
            int quantity = 1 + (std::rand() % 100);  // Random quantity between 1 and 10
            // std::cout << "Generated quantity: " << quantity << "\n";
            engine.add_order(new_price, quantity, (std::rand() % 2 == 0 ? "buy" : "sell"));
            price_history.push_back(new_price);
        }
      } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    endwin();
    return 0;
}
