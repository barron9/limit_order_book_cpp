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