#pragma once

#include <boost/asio.hpp>
#include <memory>
#include "matching_engine.hpp"

class Server {
    boost::asio::ip::tcp::acceptor acceptor_;
    MatchingEngine& engine_;  // store a reference to the engine

public:
    Server(MatchingEngine& engine, boost::asio::io_context& io_context, short port);

private:
    void do_accept();
};