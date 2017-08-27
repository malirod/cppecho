// Copyright [2017] <Malinovsky Rodion>

#include "core/async_proxy.h"
#include "core/async.h"
#include "core/async_runner.h"
#include "core/ischeduler.h"

cppecho::core::AsyncProxyBase::AsyncProxyBase(IScheduler& destination)
    : source(GetCurrentThreadAsyncRunner().GetScheduler()) {
  LOG_DEBUG("Switching context from " << source.GetName() << " to "
                                      << destination.GetName());
  SwitchTo(destination);
}

cppecho::core::AsyncProxyBase::~AsyncProxyBase() {
  SwitchTo(source);
}
