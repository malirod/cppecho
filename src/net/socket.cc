// Copyright [2016] <Malinovsky Rodion>

#include "net/socket.h"

#include <utility>

#include "net/util.h"

cppecho::net::Socket::Socket()
    : socket_(GetNetworkServiceAccessorInstance().GetRef().GetAsioService()) {}

cppecho::net::BufferType cppecho::net::Socket::ReadExact(std::size_t size) {
  BufferType buffer(size, 0);
  DeferIo([&](IoHandlerType proceed) {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(&buffer[0], buffer.size()),
                            BufferIoHandler(buffer, std::move(proceed)));
  });
  return buffer;
}

cppecho::net::BufferType cppecho::net::Socket::ReadPartial() {
  BufferType buffer;
  DeferIo([&](IoHandlerType proceed) {
    socket_.async_read_some(boost::asio::buffer(&buffer[0], buffer.size()),
                            BufferIoHandler(buffer, std::move(proceed)));
  });
  return buffer;
}

cppecho::net::BufferType cppecho::net::Socket::ReadUntil(
    const std::string& delimeter) {
  boost::asio::streambuf buffer;
  DeferIo([&](IoHandlerType proceed) {
    boost::asio::async_read_until(
        socket_, buffer, delimeter, BufferIoHandler(std::move(proceed)));
  });
  return ToBuffer(buffer);
}

void cppecho::net::Socket::Write(const BufferType& buffer) {
  DeferIo([&](IoHandlerType proceed) {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(&buffer[0], buffer.size()),
                             BufferIoHandler(std::move(proceed)));
  });
}

void cppecho::net::Socket::Connect(const std::string& ip, int port) {
  DeferIo([&](IoHandlerType proceed) {
    socket_.async_connect(boost::asio::ip::tcp::endpoint(
                              boost::asio::ip::address::from_string(ip), port),
                          proceed);
  });
}

void cppecho::net::Socket::Connect(const EndPointType& end_point) {
  DeferIo([&](IoHandlerType proceed) {
    socket_.async_connect(end_point, proceed);
  });
}

void cppecho::net::Socket::Close() {
  socket_.close();
}

cppecho::net::Socket::SocketOptsMap cppecho::net::Socket::GetSocketOpts()
    const {
  boost::asio::ip::tcp::no_delay no_delay_option;
  socket_.get_option(no_delay_option);
  return {{SocketOpt::NoDelay, no_delay_option.value()}};
}
