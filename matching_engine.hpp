#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP

#include <string>
#include <map>
#include <queue>
#include <iostream>
#include <algorithm>   // for min_element, max_element
#include <thread>      // for sleep_for
#include <chrono>      // for milliseconds
#include <sstream>
#include <iomanip>     // for put_time

struct Order {
    int id;
    double price;
    int quantity;
    std::string side; // "buy" or "sell"
};

class MatchingEngine {
private:
    int order_id_counter;

    std::map<double, std::queue<Order>, std::greater<>> bids; // descending order
    std::map<double, std::queue<Order>> asks; // ascending order

public:
    MatchingEngine();
    std::deque<std::string> latest_trades_;
    void add_order(double price, int quantity, const std::string& side);
      // Call this after every trade
    void add_trade(const std::string& trade_desc) {
        // Add timestamp
        std::stringstream ss;
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        ss << std::put_time(std::localtime(&now_c), "%H:%M:%S") << " " << trade_desc;

        latest_trades_.push_front(ss.str());

        // Keep only last 10 trades
        if (latest_trades_.size() > 10) {
            latest_trades_.pop_back();
        }
    }
    // Template match must be defined here (or in a separate .tpp included at end)
    template <typename OppBook, typename SameBook>
    void match(Order& incoming,
               OppBook& opposite_book,
               SameBook& same_book,
               const std::string& opposite_side);
      // Expose const references to internal order books
    const std::map<double, std::queue<Order>, std::greater<>>& get_bids() const {
        return bids;
    }

    const std::map<double, std::queue<Order>>& get_asks() const {
        return asks;
    }
    // Provide read-only access for UI to draw
    const std::deque<std::string>& get_latest_trades() const {
        return latest_trades_;
    }
};

// Template implementation goes here (or in .tpp included here)
template <typename OppBook, typename SameBook>
void MatchingEngine::match(Order& incoming,
                           OppBook& opposite_book,
                           SameBook& same_book,
                           const std::string& opposite_side)
{
    while (incoming.quantity > 0 && !opposite_book.empty()) {
        auto best_price_it = opposite_book.begin();
        double best_price = best_price_it->first;

        if ((incoming.side == "buy" && incoming.price >= best_price) ||
            (incoming.side == "sell" && incoming.price <= best_price)) {

            auto& queue = best_price_it->second;

            while (!queue.empty() && incoming.quantity > 0) {
                Order& resting = queue.front();
                int traded_qty = std::min(incoming.quantity, resting.quantity);

                // std::cout << "Trade: " << traded_qty << " @ " << best_price
                //           << " (Buy ID=" << (incoming.side == "buy" ? incoming.id : resting.id)
                //           << ", Sell ID=" << (incoming.side == "sell" ? incoming.id : resting.id) << ")"
                //           << std::endl;
                std::string trade_desc = (incoming.side == "buy" ? "BUY " : "SELL ") +
                                         std::to_string(traded_qty) + " @ " +
                                         std::to_string(best_price);
                add_trade(trade_desc);

                incoming.quantity -= traded_qty;
                resting.quantity -= traded_qty;

                if (resting.quantity == 0)
                    queue.pop();
            }

            if (queue.empty())
                opposite_book.erase(best_price_it);
        } else {
            break;
        }
    }

    if (incoming.quantity > 0) {
        same_book[incoming.price].push(incoming);
    }
}

#endif // MATCHING_ENGINE_HPP
