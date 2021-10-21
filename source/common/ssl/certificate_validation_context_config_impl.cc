#include "source/common/ssl/certificate_validation_context_config_impl.h"

#include "envoy/common/exception.h"
#include "envoy/config/core/v3/extension.pb.h"
#include "envoy/extensions/transport_sockets/tls/v3/cert.pb.h"
#include "envoy/extensions/transport_sockets/tls/v3/common.pb.h"

#include "source/common/common/empty_string.h"
#include "source/common/common/fmt.h"
#include "source/common/common/logger.h"
#include "source/common/config/datasource.h"

namespace Envoy {
namespace Ssl {

static const std::string INLINE_STRING = "<inline>";

CertificateValidationContextConfigImpl::CertificateValidationContextConfigImpl(
    const envoy::extensions::transport_sockets::tls::v3::CertificateValidationContext& config,
    Api::Api& api)
    : ca_cert_(Config::DataSource::read(config.trusted_ca(), true, api)),
      ca_cert_path_(Config::DataSource::getPath(config.trusted_ca())
                        .value_or(ca_cert_.empty() ? EMPTY_STRING : INLINE_STRING)),
      certificate_revocation_list_(Config::DataSource::read(config.crl(), true, api)),
      certificate_revocation_list_path_(
          Config::DataSource::getPath(config.crl())
              .value_or(certificate_revocation_list_.empty() ? EMPTY_STRING : INLINE_STRING)),
      subject_alt_name_matchers_(getSubjectAltNameMatchers(config)),
      verify_certificate_hash_list_(config.verify_certificate_hash().begin(),
                                    config.verify_certificate_hash().end()),
      verify_certificate_spki_list_(config.verify_certificate_spki().begin(),
                                    config.verify_certificate_spki().end()),
      allow_expired_certificate_(config.allow_expired_certificate()),
      trust_chain_verification_(config.trust_chain_verification()),
      custom_validator_config_(
          config.has_custom_validator_config()
              ? absl::make_optional<envoy::config::core::v3::TypedExtensionConfig>(
                    config.custom_validator_config())
              : absl::nullopt),
      api_(api) {

  if (ca_cert_.empty() && custom_validator_config_ == absl::nullopt) {
    if (!certificate_revocation_list_.empty()) {
      throw EnvoyException(fmt::format("Failed to load CRL from {} without trusted CA",
                                       certificateRevocationListPath()));
    }
    if (!subject_alt_name_matchers_.empty()) {
      throw EnvoyException("SAN-based verification of peer certificates without "
                           "trusted CA is insecure and not allowed");
    }
    if (allow_expired_certificate_) {
      throw EnvoyException("Certificate validity period is always ignored without trusted CA");
    }
  }
}

std::vector<envoy::extensions::transport_sockets::tls::v3::SubjectAltNameMatcher>
CertificateValidationContextConfigImpl::getSubjectAltNameMatchers(
    const envoy::extensions::transport_sockets::tls::v3::CertificateValidationContext& config) {
  if (!config.match_subject_alt_names_with_type().empty() &&
      !config.match_subject_alt_names().empty()) {
    throw EnvoyException("SAN-based verification using both match_subject_alt_names_with_type and "
                         "the deprecated match_subject_alt_names is not allowed");
  }
  std::vector<envoy::extensions::transport_sockets::tls::v3::SubjectAltNameMatcher>
      subject_alt_name_matchers(config.match_subject_alt_names_with_type().begin(),
                                config.match_subject_alt_names_with_type().end());
  // Handle deprecated string type san matchers without san type specified, by
  // creating backwards compatible san matcher configs.
  for (auto& matcher : config.match_subject_alt_names()) {
    subject_alt_name_matchers.emplace_back();
    subject_alt_name_matchers.back().mutable_typed_config()->mutable_typed_config()->PackFrom(
        matcher);
    subject_alt_name_matchers.back().mutable_typed_config()->mutable_name()->assign(
        "envoy.san_matchers.backward_compatible_san_matcher");
  }
  return subject_alt_name_matchers;
}

} // namespace Ssl
} // namespace Envoy
