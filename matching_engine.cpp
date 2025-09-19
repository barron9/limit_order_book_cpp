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
