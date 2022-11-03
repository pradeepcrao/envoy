#include "source/extensions/filters/http/custom_response/config.h"

#include <memory>

#include "envoy/config/core/v3/extension.pb.validate.h"
#include "envoy/extensions/filters/http/custom_response/v3/custom_response.pb.h"
#include "envoy/extensions/filters/http/custom_response/v3/custom_response.pb.validate.h"
#include "envoy/registry/registry.h"

#include "source/common/http/matching/data_impl.h"
#include "source/common/matcher/matcher.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace CustomResponse {

namespace {

class CustomResponseMatchActionValidationVisitor
    : public Matcher::MatchTreeValidationVisitor<Http::HttpMatchingData> {
public:
  absl::Status performDataInputValidation(const Matcher::DataInputFactory<Http::HttpMatchingData>&,
                                          absl::string_view) override {
    return absl::OkStatus();
  }
};

} // namespace

FilterConfig::FilterConfig(
    const envoy::extensions::filters::http::custom_response::v3::CustomResponse& config,
    Server::Configuration::ServerFactoryContext& context, Stats::StatName stats_prefix)
    : stats_prefix_(stats_prefix) {
  // Allow matcher to not be set, to allow for cases where we only have route or
  // virtual host specific configurations.
  if (config.has_custom_response_matcher()) {
    CustomResponseMatchActionValidationVisitor validation_visitor;
    CustomResponseActionFactoryContext action_factory_context{context, stats_prefix_};
    Matcher::MatchTreeFactory<Http::HttpMatchingData, CustomResponseActionFactoryContext> factory(
        action_factory_context, context, validation_visitor);
    matcher_ = factory.create(config.custom_response_matcher())();
  }
}

PolicySharedPtr FilterConfig::getPolicy(Http::ResponseHeaderMap& headers,
                                        const StreamInfo::StreamInfo& stream_info) const {
  if (!matcher_) {
    return PolicySharedPtr{};
  }

  Http::Matching::HttpMatchingDataImpl data(stream_info.downstreamAddressProvider());
  data.onResponseHeaders(headers);
  auto match = Matcher::evaluateMatch<Http::HttpMatchingData>(*matcher_, data);
  if (!match.result_) {
    return PolicySharedPtr{};
  }
  return match.result_()->getTyped<CustomResponseMatchAction>().policy_;
}

} // namespace CustomResponse
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
