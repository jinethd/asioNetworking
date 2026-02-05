#include <iostream>
#include <memory>
#include <boost/asio.hpp>

class session : public std::enable_shared_from_this<session> {
public:
    session(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}

    void start() { do_read(); }

private:
    void do_read() {
        std::shared_ptr<session> self = shared_from_this();
        socket_.async_read_some(boost::asio::buffer(data_, 1024),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "[Server] Echoing: " << std::string(data_, length);
                    do_write(length+1);
                }
            });
    }

    void do_write(std::size_t length) {
        std::shared_ptr<session> self = shared_from_this();
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    do_read();
                }
            });
    }

    boost::asio::ip::tcp::socket socket_;
    char data_[1024];
};

class server {
public:
    server(boost::asio::io_context& io_context, const short &port)
        : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
                if (!ec) {
                    std::make_shared<session>(std::move(socket))->start();
                }
                do_accept();
            });
    }
    boost::asio::ip::tcp::acceptor acceptor_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        server s(io_context, 12345);
        std::cout << "Server running on port 12345..." << std::endl;
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}