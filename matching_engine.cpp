#include "matching_engine.hpp"


MatchingEngine::MatchingEngine() : order_id_counter(1) {}

void MatchingEngine::add_order(double price, int quantity, const std::string& side) {
    Order order{order_id_counter++, price, quantity, side};
    // std::cout << "New Order: ID=" << order.id << ", " << side
    //           << " " << quantity << " @ " << price << std::endl;

    if (side == "buy") {
        match(order, asks, bids, "sell");
    } else if (side == "sell") {
        match(order, bids, asks, "buy");
    } else {
        std::cerr << "Invalid side: " << side << std::endl;
    }
}

void MatchingEngine::add_trade(const std::string& trade_desc) {
        // Add timestamp
        std::stringstream ss;
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        ss << std::put_time(std::localtime(&now_c), "%H:%M:%S") << " " << trade_desc;

        latest_trades_.push_front(ss.str());

        // Keep only last 10 trades
        if (latest_trades_.size() > 30) {
            latest_trades_.pop_back();
        }
    }

    
    