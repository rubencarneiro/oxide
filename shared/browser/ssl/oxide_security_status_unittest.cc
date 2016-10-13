// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <map>

#include "base/bind.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/ssl_status.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/referrer.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_web_contents_factory.h"
#include "net/cert/x509_certificate.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/page_transition_types.h"
#include "url/gurl.h"

#include "shared/common/oxide_enum_flags.h"
#include "shared/test/oxide_test_browser_thread_bundle.h"

#include "oxide_security_status.h"
#include "oxide_security_types.h"

namespace oxide {

namespace {

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(SecurityStatus::ChangedFlags);

}

class SecurityStatusTest : public testing::Test {
 public:
  SecurityStatusTest();

 protected:
  content::WebContents* web_contents() const { return web_contents_; }

  net::X509Certificate* cert() const { return cert_.get(); }
  net::X509Certificate* expired_cert() const { return expired_cert_.get(); }

 private:
  void SetUp() override;

  TestBrowserThreadBundle browser_thread_bundle_;
  content::TestBrowserContext browser_context_;
  content::TestWebContentsFactory web_contents_factory_;

  content::WebContents* web_contents_;

  scoped_refptr<net::X509Certificate> cert_;
  scoped_refptr<net::X509Certificate> expired_cert_;
};

void SecurityStatusTest::SetUp() {
  web_contents_ = web_contents_factory_.CreateWebContents(&browser_context_);
  SecurityStatus::CreateForWebContents(web_contents_);

  cert_ =
      new net::X509Certificate("https://www.google.com/",
                               "https://www.example.com/",
                               base::Time::UnixEpoch(),
                               base::Time::Now() + base::TimeDelta::FromDays(1));

  expired_cert_ =
      new net::X509Certificate("https://www.google.com/",
                               "https://www.example.com/",
                               base::Time::UnixEpoch(),
                               base::Time::Now() - base::TimeDelta::FromDays(1));
}

SecurityStatusTest::SecurityStatusTest() {}

TEST_F(SecurityStatusTest, EmptyWebContents) {
  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  EXPECT_EQ(SECURITY_LEVEL_NONE, status->security_level());
  EXPECT_EQ(content::SSLStatus::NORMAL_CONTENT, status->content_status());
  EXPECT_EQ(CERT_STATUS_OK, status->cert_status());
  EXPECT_EQ(nullptr, status->cert());
}

TEST_F(SecurityStatusTest, NoSecurity) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_NONE, status->security_level());
  EXPECT_EQ(content::SSLStatus::NORMAL_CONTENT, status->content_status());
  EXPECT_EQ(CERT_STATUS_OK, status->cert_status());
  EXPECT_EQ(nullptr, status->cert());
}

TEST_F(SecurityStatusTest, Secure) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("https://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_NONE, status->security_level());
  EXPECT_EQ(content::SSLStatus::NORMAL_CONTENT, status->content_status());
  EXPECT_EQ(CERT_STATUS_OK, status->cert_status());
  EXPECT_EQ(nullptr, status->cert());

  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate = cert();

  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_SECURE, status->security_level());
  EXPECT_EQ(content::SSLStatus::NORMAL_CONTENT, status->content_status());
  EXPECT_EQ(CERT_STATUS_OK, status->cert_status());
  EXPECT_EQ(cert(), status->cert());
}

TEST_F(SecurityStatusTest, Broken) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("https://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate = cert();
  ssl_status.cert_status = net::CERT_STATUS_COMMON_NAME_INVALID;

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_ERROR, status->security_level());
  EXPECT_EQ(content::SSLStatus::NORMAL_CONTENT, status->content_status());
  EXPECT_EQ(CERT_STATUS_BAD_IDENTITY, status->cert_status());
  EXPECT_EQ(cert(), status->cert());
}

TEST_F(SecurityStatusTest, RanInsecure) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("https://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate = cert();
  ssl_status.content_status = content::SSLStatus::RAN_INSECURE_CONTENT;

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_ERROR, status->security_level());
  EXPECT_EQ(content::SSLStatus::RAN_INSECURE_CONTENT, status->content_status());
  EXPECT_EQ(CERT_STATUS_OK, status->cert_status());
  EXPECT_EQ(cert(), status->cert());
}

TEST_F(SecurityStatusTest, DisplayedInsecure) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("https://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate = cert();
  ssl_status.content_status = content::SSLStatus::DISPLAYED_INSECURE_CONTENT;

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_WARNING, status->security_level());
  EXPECT_EQ(content::SSLStatus::DISPLAYED_INSECURE_CONTENT,
            status->content_status());
  EXPECT_EQ(CERT_STATUS_OK, status->cert_status());
  EXPECT_EQ(cert(), status->cert());
}

TEST_F(SecurityStatusTest, DisplayedAndRanInsecure) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("https://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate = cert();
  ssl_status.content_status =
      content::SSLStatus::DISPLAYED_INSECURE_CONTENT |
      content::SSLStatus::RAN_INSECURE_CONTENT;

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_ERROR, status->security_level());
  EXPECT_EQ(content::SSLStatus::DISPLAYED_INSECURE_CONTENT |
                content::SSLStatus::RAN_INSECURE_CONTENT,
            status->content_status());
  EXPECT_EQ(CERT_STATUS_OK, status->cert_status());
  EXPECT_EQ(cert(), status->cert());
}

TEST_F(SecurityStatusTest, MinorCertError) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("https://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate = cert();
  ssl_status.cert_status = net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION;

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_WARNING, status->security_level());
  EXPECT_EQ(content::SSLStatus::NORMAL_CONTENT, status->content_status());
  EXPECT_EQ(CERT_STATUS_REVOCATION_CHECK_FAILED, status->cert_status());
  EXPECT_EQ(cert(), status->cert());
}

TEST_F(SecurityStatusTest, SecureEV) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("https://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate = cert();
  ssl_status.cert_status = net::CERT_STATUS_IS_EV;

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  EXPECT_EQ(SECURITY_LEVEL_SECURE_EV, status->security_level());
  EXPECT_EQ(content::SSLStatus::NORMAL_CONTENT, status->content_status());
  EXPECT_EQ(CERT_STATUS_OK, status->cert_status());
  EXPECT_EQ(cert(), status->cert());
}

struct CertStatusRow {
  CertStatusRow(net::CertStatus cert_status_in,
                bool expired_cert,
                CertStatusFlags cert_status_out)
      : cert_status_in(cert_status_in),
        expired_cert(expired_cert),
        cert_status_out(cert_status_out) {}

  net::CertStatus cert_status_in;
  bool expired_cert;
  CertStatusFlags cert_status_out;
};

class SecurityStatusCertStatusTest
    : public SecurityStatusTest,
      public testing::WithParamInterface<CertStatusRow> {};

INSTANTIATE_TEST_CASE_P(
    Ok,
    SecurityStatusCertStatusTest,
    testing::Values(CertStatusRow(0, false, CERT_STATUS_OK)));
INSTANTIATE_TEST_CASE_P(
    CommonNameInvalid,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_COMMON_NAME_INVALID,
                      false,
                      CERT_STATUS_BAD_IDENTITY)));
INSTANTIATE_TEST_CASE_P(
    AuthorityInvalid,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_AUTHORITY_INVALID,
                      false,
                      CERT_STATUS_AUTHORITY_INVALID)));
INSTANTIATE_TEST_CASE_P(
    UnableToCheckRevocation,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION,
                      false,
                      CERT_STATUS_REVOCATION_CHECK_FAILED)));
INSTANTIATE_TEST_CASE_P(
    Revoked,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_REVOKED, false, CERT_STATUS_REVOKED)));
INSTANTIATE_TEST_CASE_P(
    Invalid,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_INVALID, false, CERT_STATUS_INVALID)));
INSTANTIATE_TEST_CASE_P(
    WeakSignatureAlgorithm,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM,
                      false,
                      CERT_STATUS_INSECURE)));
INSTANTIATE_TEST_CASE_P(
    WeakKey,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_WEAK_KEY, false, CERT_STATUS_INSECURE)));
INSTANTIATE_TEST_CASE_P(
    NoRevocationMechanism,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NO_REVOCATION_MECHANISM,
                      false,
                      CERT_STATUS_OK)));
INSTANTIATE_TEST_CASE_P(
    NonUniqueName,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NON_UNIQUE_NAME,
                      false,
                      CERT_STATUS_GENERIC_ERROR)));
INSTANTIATE_TEST_CASE_P(
    PinnedKeyMissing,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_PINNED_KEY_MISSING,
                      false,
                      CERT_STATUS_GENERIC_ERROR)));
INSTANTIATE_TEST_CASE_P(
    NameConstraintViolation,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NAME_CONSTRAINT_VIOLATION,
                      false,
                      CERT_STATUS_GENERIC_ERROR)));
INSTANTIATE_TEST_CASE_P(
    ValidityTooLong,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_VALIDITY_TOO_LONG,
                      false,
                      CERT_STATUS_GENERIC_ERROR)));
INSTANTIATE_TEST_CASE_P(
    DateInvalid,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_DATE_INVALID,
                      false,
                      CERT_STATUS_DATE_INVALID)));

INSTANTIATE_TEST_CASE_P(
    OkE,
    SecurityStatusCertStatusTest,
    testing::Values(CertStatusRow(0, true, CERT_STATUS_OK)));
INSTANTIATE_TEST_CASE_P(
    CommonNameInvalidE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_COMMON_NAME_INVALID,
                      true,
                      CERT_STATUS_BAD_IDENTITY)));
INSTANTIATE_TEST_CASE_P(
    AuthorityInvalidE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_AUTHORITY_INVALID,
                      true,
                      CERT_STATUS_AUTHORITY_INVALID)));
INSTANTIATE_TEST_CASE_P(
    UnableToCheckRevocationE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION,
                      true,
                      CERT_STATUS_REVOCATION_CHECK_FAILED)));
INSTANTIATE_TEST_CASE_P(
    RevokedE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_REVOKED, true, CERT_STATUS_REVOKED)));
INSTANTIATE_TEST_CASE_P(
    InvalidE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_INVALID, true, CERT_STATUS_INVALID)));
INSTANTIATE_TEST_CASE_P(
    WeakSignatureAlgorithmE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM,
                      true,
                      CERT_STATUS_INSECURE)));
INSTANTIATE_TEST_CASE_P(
    WeakKeyE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_WEAK_KEY, true, CERT_STATUS_INSECURE)));
INSTANTIATE_TEST_CASE_P(
    NoRevocationMechanismE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NO_REVOCATION_MECHANISM,
                      true,
                      CERT_STATUS_OK)));
INSTANTIATE_TEST_CASE_P(
    NonUniqueNameE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NON_UNIQUE_NAME,
                      true,
                      CERT_STATUS_GENERIC_ERROR)));
INSTANTIATE_TEST_CASE_P(
    PinnedKeyMissingE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_PINNED_KEY_MISSING,
                      true,
                      CERT_STATUS_GENERIC_ERROR)));
INSTANTIATE_TEST_CASE_P(
    NameConstraintViolationE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NAME_CONSTRAINT_VIOLATION,
                      true,
                      CERT_STATUS_GENERIC_ERROR)));
INSTANTIATE_TEST_CASE_P(
    ValidityTooLongE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_VALIDITY_TOO_LONG,
                      true,
                      CERT_STATUS_GENERIC_ERROR)));
INSTANTIATE_TEST_CASE_P(
    DateInvalidE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_DATE_INVALID,
                      true,
                      CERT_STATUS_EXPIRED)));

TEST_P(SecurityStatusCertStatusTest, TestCertStatus) {
  const CertStatusRow& row = GetParam();

  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("https://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate = row.expired_cert ? expired_cert() : cert();
  ssl_status.cert_status = row.cert_status_in;

  scoped_refptr<net::X509Certificate> expected_cert = ssl_status.certificate;

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  SecurityLevel expected_security_level;
  if (net::IsCertStatusError(row.cert_status_in)) {
    if (net::IsCertStatusMinorError(row.cert_status_in)) {
      expected_security_level = SECURITY_LEVEL_WARNING;
    } else {
      expected_security_level = SECURITY_LEVEL_ERROR;
    }
  } else {
    expected_security_level = SECURITY_LEVEL_SECURE;
  }

  EXPECT_EQ(expected_security_level, status->security_level());
  EXPECT_EQ(content::SSLStatus::NORMAL_CONTENT, status->content_status());
  EXPECT_EQ(row.cert_status_out, status->cert_status());
  EXPECT_EQ(expected_cert.get(), status->cert());
}

struct ObserverCallbackRow {
  struct InputData {
    InputData(bool https,
              bool expired_cert,
              net::CertStatus cert_status,
              content::SSLStatus::ContentStatusFlags content_status)
        : https(https),
          expired_cert(expired_cert),
          cert_status(cert_status),
          content_status(content_status) {}

    bool https;
    bool expired_cert;
    net::CertStatus cert_status;
    content::SSLStatus::ContentStatusFlags content_status;
  };

  ObserverCallbackRow(
      const InputData& initial_data,
      const InputData& test_data,
      SecurityLevel expected_security_level,
      content::SSLStatus::ContentStatusFlags expected_content_status,
      CertStatusFlags expected_cert_status,
      bool cert_expected_expired,
      SecurityStatus::ChangedFlags expected_flags)
      : initial_data(initial_data),
        test_data(test_data),
        expected_security_level(expected_security_level),
        expected_content_status(expected_content_status),
        expected_cert_status(expected_cert_status),
        cert_expected_expired(cert_expected_expired),
        expected_flags(expected_flags) {}

  InputData initial_data;
  InputData test_data;

  SecurityLevel expected_security_level;
  content::SSLStatus::ContentStatusFlags expected_content_status;
  CertStatusFlags expected_cert_status;
  bool cert_expected_expired;
  SecurityStatus::ChangedFlags expected_flags;
};

class SecurityStatusObserverCallbackTest
    : public SecurityStatusTest,
      public testing::WithParamInterface<ObserverCallbackRow> {};

INSTANTIATE_TEST_CASE_P(
    NoUpdate,
    SecurityStatusObserverCallbackTest,
    testing::Values(
        ObserverCallbackRow(
            ObserverCallbackRow::InputData(
                true, false, 0, content::SSLStatus::NORMAL_CONTENT),
            ObserverCallbackRow::InputData(
                true, false, 0, content::SSLStatus::NORMAL_CONTENT),
            SECURITY_LEVEL_SECURE,
            content::SSLStatus::NORMAL_CONTENT,
            CERT_STATUS_OK,
            false,
            SecurityStatus::CHANGED_FLAG_NONE)));
INSTANTIATE_TEST_CASE_P(
    SecurityLevel,
    SecurityStatusObserverCallbackTest,
    testing::Values(
        ObserverCallbackRow(
            ObserverCallbackRow::InputData(
                false, false, 0, content::SSLStatus::NORMAL_CONTENT),
            ObserverCallbackRow::InputData(
                true, false, 0, content::SSLStatus::NORMAL_CONTENT),
            SECURITY_LEVEL_SECURE,
            content::SSLStatus::NORMAL_CONTENT,
            CERT_STATUS_OK,
            false,
            SecurityStatus::CHANGED_FLAG_SECURITY_LEVEL)));
INSTANTIATE_TEST_CASE_P(
    ContentStatus,
    SecurityStatusObserverCallbackTest,
    testing::Values(
        ObserverCallbackRow(
            ObserverCallbackRow::InputData(
                true, false, 0, content::SSLStatus::NORMAL_CONTENT),
            ObserverCallbackRow::InputData(
                true, false, 0, content::SSLStatus::DISPLAYED_INSECURE_CONTENT),
            SECURITY_LEVEL_WARNING,
            content::SSLStatus::DISPLAYED_INSECURE_CONTENT,
            CERT_STATUS_OK,
            false,
            SecurityStatus::CHANGED_FLAG_SECURITY_LEVEL |
                SecurityStatus::CHANGED_FLAG_CONTENT_STATUS)));
INSTANTIATE_TEST_CASE_P(
    CertStatus,
    SecurityStatusObserverCallbackTest,
    testing::Values(
        ObserverCallbackRow(
            ObserverCallbackRow::InputData(
                true, false, 0, content::SSLStatus::NORMAL_CONTENT),
            ObserverCallbackRow::InputData(
                true, false,
                net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION,
                content::SSLStatus::NORMAL_CONTENT),
            SECURITY_LEVEL_WARNING,
            content::SSLStatus::NORMAL_CONTENT,
            CERT_STATUS_REVOCATION_CHECK_FAILED,
            false,
            SecurityStatus::CHANGED_FLAG_SECURITY_LEVEL |
                SecurityStatus::CHANGED_FLAG_CERT_STATUS)));
INSTANTIATE_TEST_CASE_P(
    Cert,
    SecurityStatusObserverCallbackTest,
    testing::Values(
        ObserverCallbackRow(
            ObserverCallbackRow::InputData(
                true, false, 0, content::SSLStatus::NORMAL_CONTENT),
            ObserverCallbackRow::InputData(
                true, true, 0, content::SSLStatus::NORMAL_CONTENT),
            SECURITY_LEVEL_SECURE,
            content::SSLStatus::NORMAL_CONTENT,
            CERT_STATUS_OK,
            true,
            SecurityStatus::CHANGED_FLAG_CERT)));

namespace {
void TestCallback(SecurityStatus* status,
                  SecurityLevel* security_level_out,
                  content::SSLStatus::ContentStatusFlags* content_status_out,
                  CertStatusFlags* cert_status_out,
                  net::X509Certificate** cert_out,
                  SecurityStatus::ChangedFlags* flags_out,
                  SecurityStatus::ChangedFlags flags) {
  *security_level_out = status->security_level();
  *content_status_out = status->content_status();
  *cert_status_out = status->cert_status();
  *cert_out = status->cert();
  *flags_out = flags;
}
}

TEST_P(SecurityStatusObserverCallbackTest, TestObserver) {
  const ObserverCallbackRow& params = GetParam();

  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(params.initial_data.https ?
                         GURL("https://www.google.com/")
                         : GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.certificate =
      params.initial_data.expired_cert ? expired_cert() : cert();
  ssl_status.cert_status = params.initial_data.cert_status;
  ssl_status.content_status = params.initial_data.content_status;

  SecurityStatus* status = SecurityStatus::FromWebContents(web_contents());
  status->VisibleSSLStateChanged();

  SecurityStatus::ChangedFlags flags = SecurityStatus::CHANGED_FLAG_NONE;
  SecurityLevel security_level = status->security_level();
  content::SSLStatus::ContentStatusFlags content_status =
      status->content_status();
  CertStatusFlags cert_status = status->cert_status();
  net::X509Certificate* cert = status->cert();

  std::unique_ptr<SecurityStatus::Subscription> subscription =
      status->AddChangeCallback(base::Bind(&TestCallback,
                                           status,
                                           &security_level,
                                           &content_status,
                                           &cert_status,
                                           &cert,
                                           &flags));

  controller.GetVisibleEntry()->SetURL(
      params.test_data.https ?
          GURL("https://www.google.com") : GURL("http://www.google.com/"));
  ssl_status.certificate =
      params.test_data.expired_cert ? expired_cert() : this->cert();
  ssl_status.cert_status = params.test_data.cert_status;
  ssl_status.content_status = params.test_data.content_status;

  scoped_refptr<net::X509Certificate> expected_cert = ssl_status.certificate;

  status->VisibleSSLStateChanged();

  EXPECT_EQ(params.expected_flags, flags);
  EXPECT_EQ(params.expected_security_level, security_level);
  EXPECT_EQ(params.expected_content_status, content_status);
  EXPECT_EQ(params.expected_cert_status, cert_status);
  EXPECT_EQ(expected_cert.get(), cert);
}

} // namespace oxide
