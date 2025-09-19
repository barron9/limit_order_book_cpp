#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include "matching_engine.hpp"  // Your matching engine header
#include "Session.hpp"
#include "Server.hpp"

// This line brings 'tcp' into scope
using boost::asio::ip::tcp;
Server::Server(MatchingEngine& engine, boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      engine_(engine)
{
    do_accept();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket), engine_)->start();
            }
            do_accept();
        });
}