#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include "matching_engine.hpp"  // Ensure this file declares MatchingEngine

class Session : public std::enable_shared_from_this<Session> {

    boost::asio::ip::tcp::socket socket_;
    MatchingEngine& engine_;
    boost::asio::streambuf buffer_;

public:
    Session(boost::asio::ip::tcp::socket socket, MatchingEngine& engine);

    void start();

private:
    void do_read();
    void do_write(const std::string& message);
};
