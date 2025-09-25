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
    void add_trade(const std::string& trade_desc);
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

#include "MatchingEngine.tpp"

#endif // MATCHING_ENGINE_HPP
