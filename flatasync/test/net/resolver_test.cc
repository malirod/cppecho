// Copyright [2018] <Malinovsky Rodion>

#include <boost/asio.hpp>
#include <memory>

#include <gtest/gtest.h>
#include "core/async.h"
#include "core/helper.h"
#include "net/alias.h"
#include "net/resolver.h"
#include "net/util.h"
#include "util/logger.h"

DECLARE_GLOBAL_GET_LOGGER("Test.Net.Resolver");

namespace {

using rms::core::SchedulersInitiator;
using rms::core::WaitAll;
using rms::net::EndPointType;
using rms::net::GetNetworkSchedulerAccessorInstance;
using rms::net::Resolver;

}  // namespace

TEST(TestResolver, ResolveLocalhost) {
  LOG_AUTO_TRACE();

  auto schedulers_initiator = std::make_unique<SchedulersInitiator>();

  RunAsync(
      [&] {
        Resolver resolver;
        const auto endpoints = resolver.Resolve("localhost", 10124);
        const decltype(endpoints) end;
        // ensure at least one endpoint exist
        ASSERT_TRUE(endpoints != end);
        // Take first endpoint
        const EndPointType endpoint = *endpoints;

        ASSERT_EQ(10124, endpoint.port());
        ASSERT_EQ("127.0.0.1", endpoint.address().to_v4().to_string());
      },
      GetNetworkSchedulerAccessorInstance().GetRef());

  LOG_DEBUG("Waiting all async tasks");
  WaitAll();
  LOG_DEBUG("Waited all async tasks");
}
