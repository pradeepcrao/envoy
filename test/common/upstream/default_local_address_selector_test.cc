#include "envoy/common/exception.h"
#include "envoy/network/socket.h"
#include "envoy/upstream/upstream.h"

#include "source/common/network/address_impl.h"
#include "source/common/network/utility.h"
#include "source/common/upstream/default_local_address_selector_factory.h"

#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Upstream {
namespace {

TEST(ConfigTest, EmptyUpstreamAddresses) {
  DefaultUpstreamLocalAddressSelectorFactory factory;
  std::vector<UpstreamLocalAddress> upstream_local_addresses;
  EXPECT_THROW_WITH_MESSAGE(
      factory.createLocalAddressSelector(upstream_local_addresses, absl::nullopt), EnvoyException,
      "Bootstrap's upstream binding config has no valid source address.");
}

TEST(ConfigTest, NullUpstreamAddress) {
  DefaultUpstreamLocalAddressSelectorFactory factory;
  std::vector<UpstreamLocalAddress> upstream_local_addresses;
  upstream_local_addresses.emplace_back(
      UpstreamLocalAddress{nullptr, std::make_shared<Network::ConnectionSocket::Options>()});
  // This should be exception free.
  UpstreamLocalAddressSelectorConstSharedPtr selector =
      factory.createLocalAddressSelector(upstream_local_addresses, absl::nullopt);
}

TEST(ConfigTest, NullUpstreamAddresses) {
  DefaultUpstreamLocalAddressSelectorFactory factory;
  std::vector<UpstreamLocalAddress> upstream_local_addresses;

  // 2 null addresses
  upstream_local_addresses.emplace_back(
      UpstreamLocalAddress{nullptr, std::make_shared<Network::ConnectionSocket::Options>()});
  upstream_local_addresses.emplace_back(
      UpstreamLocalAddress{nullptr, std::make_shared<Network::ConnectionSocket::Options>()});
  EXPECT_THROW_WITH_MESSAGE(
      factory.createLocalAddressSelector(upstream_local_addresses, absl::nullopt), EnvoyException,
      "Bootstrap's upstream binding config has invalid IP addresses.");

  // 1st non-null and 2nd null
  upstream_local_addresses[0].address_ =
      Envoy::Network::Utility::parseInternetAddress("1.2.3.5", 80, false);
  EXPECT_THROW_WITH_MESSAGE(
      factory.createLocalAddressSelector(upstream_local_addresses, absl::nullopt), EnvoyException,
      "Bootstrap's upstream binding config has invalid IP addresses.");

  // 1st null and 2nd non-null
  upstream_local_addresses[0].address_ = nullptr;
  upstream_local_addresses[1].address_ =
      Envoy::Network::Utility::parseInternetAddress("1.2.3.5", 80, false);
  EXPECT_THROW_WITH_MESSAGE(
      factory.createLocalAddressSelector(upstream_local_addresses, absl::nullopt), EnvoyException,
      "Bootstrap's upstream binding config has invalid IP addresses.");

  // 1st non IP and 2nd valid IP
  upstream_local_addresses[0].address_ =
      std::make_shared<Envoy::Network::Address::PipeInstance>("/abc", 400);
  EXPECT_THROW_WITH_MESSAGE(
      factory.createLocalAddressSelector(upstream_local_addresses, absl::nullopt), EnvoyException,
      "Bootstrap's upstream binding config has invalid IP addresses.");

  // 1st valid IP and 2nd non IP
  upstream_local_addresses[0].address_ =
      Envoy::Network::Utility::parseInternetAddress("1.2.3.5", 80, false);
  upstream_local_addresses[1].address_ =
      std::make_shared<Envoy::Network::Address::PipeInstance>("/abc", 400);
  EXPECT_THROW_WITH_MESSAGE(
      factory.createLocalAddressSelector(upstream_local_addresses, absl::nullopt), EnvoyException,
      "Bootstrap's upstream binding config has invalid IP addresses.");

  // 1 non IP address only
  upstream_local_addresses.pop_back();
  upstream_local_addresses[0].address_ =
      std::make_shared<Envoy::Network::Address::PipeInstance>("/abc", 400);
  EXPECT_THROW_WITH_MESSAGE(
      factory.createLocalAddressSelector(upstream_local_addresses, absl::nullopt), EnvoyException,
      "Bootstrap's upstream binding config has invalid IP addresses.");
}

} // namespace
} // namespace Upstream
} // namespace Envoy
