// Copyright [2018] <Malinovsky Rodion>

#include "net/util.h"

#include <functional>
#include <utility>

#include "core/alias.h"
#include "core/async.h"
#include "util/logger.h"

DECLARE_GLOBAL_GET_LOGGER("Net.Util")

rms::net::NetworkServiceAccessor& rms::net::GetNetworkServiceAccessorInstance() {
  return rms::util::single<NetworkServiceAccessor>();
}

rms::net::NetworkIoSchedulerAccessor& rms::net::GetNetworkSchedulerAccessorInstance() {
  return rms::util::single<NetworkIoSchedulerAccessor>();
}

rms::net::ErrorType rms::net::DeferIo(CallbackIoHandlerType callback) {
  ErrorType error;
  rms::core::DeferProceed([callback = std::move(callback), &error](rms::core::HandlerType proceed) {
    callback([proceed, &error](const ErrorType& e) {
      error = e;
      proceed();
    });
  });
  return error;
}

rms::net::BufferIoHandlerType rms::net::BufferIoHandler(BufferType& buffer, IoHandlerType proceed) {
  return [&buffer, proceed = std::move(proceed)](const ErrorType& error, std::size_t size) {
    if (!error) {
      buffer.resize(size);
    }
    proceed(error);
  };
}

rms::net::BufferIoHandlerType rms::net::BufferIoHandler(IoHandlerType proceed) {
  return [proceed = std::move(proceed)](const ErrorType& error, std::size_t) { proceed(error); };
}

rms::net::BufferType rms::net::ToBuffer(const boost::asio::streambuf& streambuf) {
  return {boost::asio::buffers_begin(streambuf.data()), boost::asio::buffers_end(streambuf.data())};
}
