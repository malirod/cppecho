// Copyright [2016] <Malinovsky Rodion>

#include "net/util.h"
#include "core/alias.h"
#include "core/async.h"

cppecho::net::NetworkServiceAccessor&
cppecho::net::GetNetworkServiceAccessorInstance() {
  return cppecho::util::single<NetworkServiceAccessor>();
}

cppecho::net::NetworkIoSchedulerAccessor&
cppecho::net::GetNetworkSchedulerAccessorInstance() {
  return cppecho::util::single<NetworkIoSchedulerAccessor>();
}

void cppecho::net::DeferIo(CallbackIoHandlerType callback) {
  ErrorType error;
  cppecho::core::DeferProceed(
      [callback, &error](cppecho::core::HandlerType proceed) {
        callback([proceed, &error](const ErrorType& e) {
          error = e;
          proceed();
        });
      });
  if (!!error) {
    throw boost::system::system_error(error, "Network");
  }
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
