# Simple Asio Async Echo Project

This project is a basic asynchronous TCP Client and Server using Boost.Asio. It implements the Proactor Design Pattern, where the io_context acts as a central coordinator; it manages long-running network tasks in the background and only notifies your code via 'callbacks' once a task is actually finished.

### 1. server.cpp
This is the "listener."
* **How it works:** It opens port 12345 and waits for a connection.
* **The "Session":** Every time a client connects, the server creates a new `session` object. This object lives on the heap (using `shared_ptr`) as long as the client is talking.
* **The Loop:** It uses `async_read_some` to wait for data. Once it gets something, it immediately uses `async_write` to send it back, then goes right back to reading.

### 2. client.cpp
This is the interactive terminal part.
* **How it works:** It connects to the server at 127.0.0.1.
* **Two Threads:** One thread runs the `io_context` (handling the network stuff), and the main thread stays open for `std::getline` so you can type messages.
* **The "Echo":** It starts a read loop as soon as it connects. When the server echoes your message, the client prints it out with a `[Server Echo]` tag.

### built with:
`g++ -std=c++11 server.cpp -o server -lboost_system -lpthread`

`g++ -std=c++11 client.cpp -o server -lboost_system -lpthread` on a different terminal (can do more than one if desired although 'chat room'-like behaviour is not yet implemented)

`./server` and `./client` on the respective terminals

#### Key Logic Concepts
* **shared_from_this()**: Used to keep session/client objects alive while async operations are "in flight."
* **Asynchronous Callbacks**: Instead of blocking, the program tells the OS to call a function only when data is ready.
* **recursive `do_accept()`** in server allows for multiple clients to connect. this does not flood the stack with recursive calls as after one is made, it is passed off to the io_context engine. the current do_accept's closing brace is called and the lambda is not invoked until io_context calls the lambda when a client connects
* **TCP IPv4**: Uses standard 32-bit IP addressing for local communication.
