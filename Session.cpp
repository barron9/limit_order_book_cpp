#include "Session.hpp"
#include <sstream>

using boost::asio::ip::tcp;

Session::Session(tcp::socket socket, MatchingEngine& engine)
    : socket_(std::move(socket)), engine_(engine) {}

void Session::start() {
    do_read();
}

void Session::do_read() {
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, buffer_, '\n',
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);

                std::istringstream iss(line);
                std::string side;
                int quantity;
                double price;
                if (iss >> side >> quantity >> price) {
                    engine_.add_order(price, quantity, side);
                    std::string response = "Order added: " + line + "\n";
                    do_write(response);
                } else {
                    std::string response = "Invalid order format\n";
                    do_write(response);
                }
            }
        });
}

void Session::do_write(const std::string& message) {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
        boost::asio::buffer(message),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                do_read();
            }
        });
}
