#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>

class client : public std::enable_shared_from_this<client> {
public:
    client(boost::asio::io_context& io_context)
        : socket_(io_context) {}

    void connect(const std::string& host, const std::string& port) {
        boost::asio::ip::tcp::resolver resolver(socket_.get_executor());
        auto endpoints = resolver.resolve(host, port);
        
        std::shared_ptr<client> self = shared_from_this();
        boost::asio::async_connect(socket_, endpoints,
            [this, self](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
                if (!ec) {
                    std::cout << "Connected! Type messages below." << std::endl;
                    do_read(); 
                } else {
                    std::cerr << "Connect error: " << ec.message() << std::endl;
                }
            });
    }

    void send_message(std::string msg) {
        std::shared_ptr<client> self = shared_from_this();
        auto msg_ptr = std::make_shared<std::string>(msg + "\n");
        
        boost::asio::async_write(socket_, boost::asio::buffer(*msg_ptr),
            [this, self, msg_ptr](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                }
            });
    }

private:
    void do_read() {
        std::shared_ptr<client> self = shared_from_this();
        socket_.async_read_some(boost::asio::buffer(data_, 1024),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "\n[Server Echo]: " << std::string(data_, length);
                    std::cout << "> " << std::flush;
                    
                    do_read();
                } else {
                    std::cout << "\nConnection closed by server." << std::endl;
                }
            });
    }

    boost::asio::ip::tcp::socket socket_;
    char data_[1024];
};

int main() {
    try {
        boost::asio::io_context io_context;
        auto c = std::make_shared<client>(io_context);
        c->connect("127.0.0.1", "12345");

        std::thread network_thread([&io_context]() { io_context.run(); });

        std::string input;
        std::cout << "> " << std::flush;
        while (std::getline(std::cin, input)) {
            if (input == "exit") break;
            c->send_message(input);
        }

        io_context.stop();
        if (network_thread.joinable()) network_thread.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}