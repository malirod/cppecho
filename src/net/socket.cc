// Copyright [2016] <Malinovsky Rodion>

#include "net/socket.h"
#include <utility>
#include "net/util.h"

cppecho::net::Socket::Socket()
    : socket_(GetNetworkServiceAccessorInstance().GetRef().GetAsioService()) {}

void cppecho::net::Socket::Read(BufferType& buffer) {
  DeferIo([&buffer, this](IoHandlerType proceed) {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(&buffer[0], buffer.size()),
                            BufferIoHandler(buffer, std::move(proceed)));
  });
}

void cppecho::net::Socket::PartialRead(BufferType& buffer) {
  DeferIo([&buffer, this](IoHandlerType proceed) {
    socket_.async_read_some(boost::asio::buffer(&buffer[0], buffer.size()),
                            BufferIoHandler(buffer, std::move(proceed)));
  });
}

void cppecho::net::Socket::Write(const BufferType& buffer) {
  DeferIo([&buffer, this](IoHandlerType proceed) {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(&buffer[0], buffer.size()),
                             BufferIoHandler(std::move(proceed)));
  });
}

void cppecho::net::Socket::Connect(const std::string& ip, int port) {
  DeferIo([&ip, port, this](IoHandlerType proceed) {
    socket_.async_connect(boost::asio::ip::tcp::endpoint(
                              boost::asio::ip::address::from_string(ip), port),
                          proceed);
  });
}

void cppecho::net::Socket::Connect(const EndPointType& end_point) {
  DeferIo([&end_point, this](IoHandlerType proceed) {
    socket_.async_connect(end_point, proceed);
  });
}

void cppecho::net::Socket::Close() {
  socket_.close();
}
