// Copyright [2017] <Malinovsky Rodion>

#include "util/thread_util.h"
#include <atomic>
#include <thread>
#include "boost/format.hpp"
#include "core/iioservice.h"

namespace {

thread_local int tls_thread_in_pool_number = 0;

thread_local const char* tls_pool_name = "default";

thread_local cppecho::core::IIoService* thrd_ptr_ioservice = nullptr;

}  // namespace

void cppecho::util::ThreadUtil::SetCurrentThreadName(const char* name) {
  tls_pool_name = name;
}

void cppecho::util::ThreadUtil::SetCurrentThreadNumber(int number) {
  tls_thread_in_pool_number = number;
}

const char* cppecho::util::ThreadUtil::GetCurrentThreadName() {
  return tls_pool_name;
}

int cppecho::util::ThreadUtil::GetCurrentThreadNumber() {
  return tls_thread_in_pool_number;
}

void cppecho::util::SleepFor(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::string cppecho::util::ThreadUtil::GetCurrentThreadId() {
  return str(boost::format("[@%1%:%2%]") % GetCurrentThreadName() %
             GetCurrentThreadNumber());
}

void cppecho::util::ThreadUtil::SetCurrentThreadIoSerivce(
    cppecho::core::IIoService& ioservice) {
  thrd_ptr_ioservice = &ioservice;
}

cppecho::core::IIoService&
cppecho::util::ThreadUtil::GetCurrentThreadIoSerivce() {
  assert(thrd_ptr_ioservice != nullptr);
  return *thrd_ptr_ioservice;
}
