// Copyright [2018] <Malinovsky Rodion>

#include "util/thread_util.h"
#include <cassert>
#include <chrono>
#include <thread>

namespace {

thread_local int tls_thread_in_pool_number = 0;

thread_local const char* tls_pool_name = "default";

thread_local rms::core::IIoService* thrd_ptr_ioservice = nullptr;

}  // namespace

void rms::util::ThreadUtil::SetCurrentThreadName(const char* name) {
  tls_pool_name = name;
}

void rms::util::ThreadUtil::SetCurrentThreadNumber(int number) {
  tls_thread_in_pool_number = number;
}

const char* rms::util::ThreadUtil::GetCurrentThreadName() {
  return tls_pool_name;
}

int rms::util::ThreadUtil::GetCurrentThreadNumber() {
  return tls_thread_in_pool_number;
}

void rms::util::SleepFor(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::string rms::util::ThreadUtil::GetCurrentThreadId() {
  return "[" + std::string(GetCurrentThreadName()) + ":" + std::to_string(GetCurrentThreadNumber()) + "]";
}

void rms::util::ThreadUtil::SetCurrentThreadIoService(rms::core::IIoService& ioservice) {
  thrd_ptr_ioservice = &ioservice;
}

rms::core::IIoService& rms::util::ThreadUtil::GetCurrentThreadIoService() {
  assert(thrd_ptr_ioservice != nullptr);
  return *thrd_ptr_ioservice;
}
