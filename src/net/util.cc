// Copyright [2016] <Malinovsky Rodion>

#include "net/util.h"
#include "core/alias.h"
#include "core/async.h"
#include "util/logger.h"

DECLARE_GLOBAL_GET_LOGGER("Net.Util");

cppecho::net::NetworkServiceAccessor&
cppecho::net::GetNetworkServiceAccessorInstance() {
  return cppecho::util::single<NetworkServiceAccessor>();
}

cppecho::net::NetworkIoSchedulerAccessor&
cppecho::net::GetNetworkSchedulerAccessorInstance() {
  return cppecho::util::single<NetworkIoSchedulerAccessor>();
}

cppecho::net::ErrorType cppecho::net::DeferIo(CallbackIoHandlerType callback) {
  ErrorType error;
  cppecho::core::DeferProceed(
      [callback, &error](cppecho::core::HandlerType proceed) {
        callback([proceed, &error](const ErrorType& e) {
          error = e;
          proceed();
        });
      });
  return error;
}

cppecho::net::BufferIoHandlerType cppecho::net::BufferIoHandler(
    BufferType& buffer, IoHandlerType proceed) {
  return [&buffer, proceed](const ErrorType& error, std::size_t size) {
    if (!error) {
      buffer.resize(size);
    }
    proceed(error);
  };
}

cppecho::net::BufferIoHandlerType cppecho::net::BufferIoHandler(
    IoHandlerType proceed) {
  return [proceed](const ErrorType& error, std::size_t) { proceed(error); };
}

cppecho::net::BufferType cppecho::net::ToBuffer(
    const boost::asio::streambuf& streambuf) {
  return {boost::asio::buffers_begin(streambuf.data()),
          boost::asio::buffers_end(streambuf.data())};
}
