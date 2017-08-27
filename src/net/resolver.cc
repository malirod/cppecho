// Copyright [2016] <Malinovsky Rodion>

#include "net/resolver.h"
#include "net/alias.h"
#include "net/util.h"

cppecho::net::Resolver::Resolver()
    : resolver_(GetNetworkServiceAccessorInstance().GetRef().GetAsioService()) {
}

cppecho::net::Resolver::EndPointsType cppecho::net::Resolver::Resolve(
    const std::string& hostname, int port) {
  boost::asio::ip::tcp::resolver::query query(hostname, std::to_string(port));
  EndPointsType result;
  DeferIo([this, &query, &result](IoHandlerType proceed) {
    resolver_.async_resolve(
        query,
        [proceed, &result](const ErrorType& error, EndPointsType end_points) {
          if (!error) {
            result = end_points;
          }
          proceed(error);
        });
  });
  return result;
}
