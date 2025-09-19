// // /* #include <drogon/drogon.h>

// // using namespace drogon;

// // int main()
// // {
// //     app()
// //         .setLogPath("./")                         // Log files saved in current directory
// //         .setLogLevel(trantor::Logger::kWarn)      // Log level set to warning
// //         .addListener("0.0.0.0", 8080)             // Listen on all interfaces, port 8080
// //         .setThreadNum(4)                          // Use 4 threads to handle requests
// //         .enableRunAsDaemon()                      // Run as background daemon
// //         .run();                                   // Start the server (blocking call)
// // } */
// // //  conan install .   --output-folder=.   --build=missing   --generator CMakeToolchain   --generator CMakeDeps
// // //   cmake .. -DCMAKE_TOOLCHAIN_FILE=Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
// // // cmake --build .
// // #include <drogon/drogon.h>
// // #include "matching_engine.hpp"
// // int main() {
// //     drogon::app().registerHandler("/", [](const drogon::HttpRequestPtr &,
// //                                           std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
// //         auto resp = drogon::HttpResponse::newHttpResponse();
// //         resp->setBody("Hello from Drogon!");
// //         callback(resp);
// //     });
// //     MatchingEngine engine;

// //     engine.add_order(100.0, 10, "buy");
// //     engine.add_order(101.0, 5, "buy");
// //     engine.add_order(99.0, 20, "sell");
// //     engine.add_order(101.0, 3, "sell");
// //     engine.add_order(100.0, 5, "sell");
// //     drogon::app().addListener("0.0.0.0", 8080);  // Optional: make sure it listens on all interfaces
// //     drogon::app().run();
// // }

// #include <boost/asio.hpp>
// #include <iostream>
// #include <string>
// #include <memory>
// #include <thread>
// #include "matching_engine.hpp"  // Your matching engine header

// using boost::asio::ip::tcp;

// // Session handles communication with a single client
// class Session : public std::enable_shared_from_this<Session> {
//     tcp::socket socket_;
//     MatchingEngine& engine_;
//     boost::asio::streambuf buffer_;

// public:
//     Session(tcp::socket socket, MatchingEngine& engine)
//         : socket_(std::move(socket)), engine_(engine) {}

//     void start() {
//         do_read();
//     }

// private:
//     void do_read() {
//         auto self(shared_from_this());
//         boost::asio::async_read_until(socket_, buffer_, '\n',
//             [this, self](boost::system::error_code ec, std::size_t length) {
//                 if (!ec) {
//                     std::istream is(&buffer_);
//                     std::string line;
//                     std::getline(is, line);

//                     // Example input format: "buy 100 10.5"
//                     // side quantity price
//                     std::istringstream iss(line);
//                     std::string side;
//                     int quantity;
//                     double price;
//                     if (iss >> side >> quantity >> price) {
//                         engine_.add_order(price, quantity, side);

//                         std::string response = "Order added: " + line + "\n";
//                         do_write(response);
//                     } else {
//                         std::string response = "Invalid order format\n";
//                         do_write(response);
//                     }
//                 }
//             });
//     }

//     void do_write(const std::string& message) {
//         auto self(shared_from_this());
//         boost::asio::async_write(socket_,
//             boost::asio::buffer(message),
//             [this, self](boost::system::error_code ec, std::size_t /*length*/) {
//                 if (!ec) {
//                     do_read();
//                 }
//             });
//     }
// };

// class Server {
//     tcp::acceptor acceptor_;
//     MatchingEngine engine_;

// public:
//     Server(boost::asio::io_context& io_context, short port)
//         : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
//         do_accept();
//     }

// private:
//     void do_accept() {
//         acceptor_.async_accept(
//             [this](boost::system::error_code ec, tcp::socket socket) {
//                 if (!ec) {
//                     std::make_shared<Session>(std::move(socket), engine_)->start();
//                 }
//                 do_accept();
//             });
//     }
// };

// int main() {
//     try {
//         boost::asio::io_context io_context;

//         Server server(io_context, 8080);

//         io_context.run();
//     } catch (std::exception& e) {
//         std::cerr << "Exception: " << e.what() << "\n";
//     }

//     return 0;
// }

#include <ncurses.h>
#include <deque>
#include <vector>
#include <chrono>
#include <thread>
#include "matching_engine.hpp"
#include <ncurses.h>
#include <iomanip>
#include <sstream>

// You may need these includes for sorting:
#include <map>
#include <vector>
#include <algorithm>
// Constants for layout
const int ORDER_BOOK_HEIGHT = 20;
const int ORDER_BOOK_WIDTH = 50;
const int GRAPH_HEIGHT = 20;
const int GRAPH_WIDTH = 80;
const int TRADES_HEIGHT = 20;
const int TRADES_WIDTH = 40;


// Draws the price graph in the given window
void draw_price_graph(WINDOW* win, const std::vector<double>& prices) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " Price Graph ");

    if (prices.empty()) {
        mvwprintw(win, 1, 1, "No price data.");
        wrefresh(win);
        return;
    }

    double min_price = *std::min_element(prices.begin(), prices.end());
    double max_price = *std::max_element(prices.begin(), prices.end());
    double range = max_price - min_price;
    if (range == 0) range = 10.0;

    int width = GRAPH_WIDTH - 2;
    int height = GRAPH_HEIGHT - 2;
    int max_points = std::min(width, static_cast<int>(prices.size()));

    for (int i = 0; i < max_points; ++i) {
        int index = prices.size() - max_points + i;
        double price = prices[index];
        int y = height - static_cast<int>(((price - min_price) / range) * height);
        int x = i + 1;
        mvwaddch(win, y + 1, x, ACS_DIAMOND);  // graphical block
    }

    wrefresh(win);
}

void draw_latest_trades(WINDOW* win, const MatchingEngine& engine) {
    werase(win);
    box(win, 0, 0);
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

    int width, height;
    getmaxyx(win, height, width);

    mvwprintw(win, 1, (width / 2) - 5, "ORDER BOOK");

    mvwprintw(win, 3, 2,  "SELL ORDERS");
    mvwprintw(win, 3, width / 2 + 2, "BUY ORDERS");

    // Display only top 10
    int row = 5;
    int max_rows = std::min(10, height - row - 2);

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
    noecho();
    curs_set(0);
    std::srand(std::time(nullptr));  // Seed rand() with current time
    if (!has_colors()) {
        endwin();
        std::cerr << "Your terminal does not support color\n";
        exit(1);
    }
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1); // Green text
    init_pair(2, COLOR_RED, -1);   // Red text
    // Create windows
    WINDOW* order_book_win = newwin(ORDER_BOOK_HEIGHT, ORDER_BOOK_WIDTH, 0, 0);
    WINDOW* graph_win = newwin(GRAPH_HEIGHT, GRAPH_WIDTH, 0, ORDER_BOOK_WIDTH + 2);
    WINDOW* trades_win = newwin(TRADES_HEIGHT, TRADES_WIDTH, 0, ORDER_BOOK_WIDTH + GRAPH_WIDTH + 2);
    // Initialize engine and price history
    MatchingEngine engine;
    std::deque<double> price_history;

    // Dummy initial orders
    engine.add_order(100.0, 10, "buy");
    engine.add_order(101.0, 5, "buy");
    engine.add_order(102.0, 5, "sell");
    price_history.push_back(101.0);

    // Main loop
    while (true) {
        draw_order_book(order_book_win, engine);

        // Keep last 80 prices
        if (price_history.size() > GRAPH_WIDTH - 2) {
            price_history.pop_front();
        }

        std::vector<double> prices(price_history.begin(), price_history.end());
        draw_price_graph(graph_win, prices);
        draw_latest_trades(trades_win, engine);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        // Simulate live trading (random orders)
        double new_price = 100.0 + ((std::rand() % 101) - 50) / 10.0;
        int quantity = 1 + (std::rand() % 100);  // Random quantity between 1 and 10
        // std::cout << "Generated quantity: " << quantity << "\n";
        engine.add_order(new_price, quantity, (std::rand() % 2 == 0 ? "buy" : "sell"));
        price_history.push_back(new_price);
    }

    endwin();
    return 0;
}
