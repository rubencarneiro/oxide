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

#include <memory>
#include <QMetaType>
#include <QObject>
#include <QPointer>
#include <QVariant>

#include "base/memory/ptr_util.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/referrer.h"
#include "content/public/common/security_style.h"
#include "content/public/common/ssl_status.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_web_contents_factory.h"
#include "net/cert/x509_certificate.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "qt/core/api/oxideqsecuritystatus.h"
#include "qt/core/api/oxideqsecuritystatus_p.h"
#include "qt/core/api/oxideqsslcertificate.h"
#include "qt/core/api/oxideqsslcertificate_p.h"
#include "shared/test/oxide_mock_cert_store.h"
#include "shared/test/oxide_test_browser_thread_bundle.h"

#include "oxide_qt_security_status.h"

namespace oxide {
namespace qt {

using oxide::MockCertStore;
using oxide::TestBrowserThreadBundle;

class SecurityStatusTest : public testing::Test {
 public:
  SecurityStatusTest();

  int cert_id() const { return cert_id_; }
  int expired_cert_id() const { return expired_cert_id_; }

  content::CertStore* cert_store() { return &cert_store_; }

  OxideQSecurityStatus* status() const { return security_status_.get(); }

  content::WebContents* web_contents() const { return web_contents_; }

 private:
  void SetUp() override;

  MockCertStore cert_store_;
  TestBrowserThreadBundle browser_thread_bundle_;
  content::TestBrowserContext browser_context_;
  content::TestWebContentsFactory web_contents_factory_;

  std::unique_ptr<OxideQSecurityStatus> security_status_;
  content::WebContents* web_contents_;

  int cert_id_;
  int expired_cert_id_;
};

SecurityStatusTest::SecurityStatusTest()
    : web_contents_(nullptr),
      cert_id_(0),
      expired_cert_id_(0) {}

void SecurityStatusTest::SetUp() {
  security_status_ = base::WrapUnique(OxideQSecurityStatusPrivate::Create());
  web_contents_ = web_contents_factory_.CreateWebContents(&browser_context_);
  oxide::SecurityStatus::CreateForWebContents(web_contents_);
  oxide::SecurityStatus::FromWebContents(web_contents_)->SetCertStoreForTesting(
      &cert_store_);

  OxideQSecurityStatusPrivate::get(security_status_.get())->proxy()->Init(
      web_contents_);

  scoped_refptr<net::X509Certificate> cert(
      new net::X509Certificate("https://www.google.com/",
                               "https://www.example.com/",
                               base::Time::UnixEpoch(),
                               base::Time::Now() + base::TimeDelta::FromDays(1)));
  cert_id_ = cert_store_.AddCertForTesting(cert.get());

  cert =
      new net::X509Certificate("https://www.google.com/",
                               "https://www.example.com/",
                               base::Time::UnixEpoch(),
                               base::Time::Now() - base::TimeDelta::FromDays(1));
  expired_cert_id_ = cert_store_.AddCertForTesting(cert.get());
}

TEST_F(SecurityStatusTest, Uninitialized) {
  std::unique_ptr<OxideQSecurityStatus> status =
      base::WrapUnique(OxideQSecurityStatusPrivate::Create());
  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelNone, status->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusNormal, status->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusOk, status->certStatus());
  EXPECT_TRUE(status->certificate().isNull());
  EXPECT_EQ(QMetaType::VoidStar,
            static_cast<QMetaType::Type>(status->certificate().type()));
}

TEST_F(SecurityStatusTest, EmptyWebContents) {
  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelNone, status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusNormal,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusOk, status()->certStatus());
  EXPECT_TRUE(status()->certificate().isNull());
  EXPECT_EQ(QMetaType::VoidStar,
            static_cast<QMetaType::Type>(status()->certificate().type()));
}

TEST_F(SecurityStatusTest, NoSecurity) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_UNAUTHENTICATED;

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelNone, status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusNormal,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusOk, status()->certStatus());
  EXPECT_TRUE(status()->certificate().isNull());
  EXPECT_EQ(QMetaType::VoidStar,
            static_cast<QMetaType::Type>(status()->certificate().type()));
}

TEST_F(SecurityStatusTest, Secure) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_status.cert_id = cert_id();

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(cert_id(), &cert));

  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelSecure,
            status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusNormal,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusOk, status()->certStatus());
  EXPECT_FALSE(status()->certificate().isNull());
  EXPECT_EQ(QVariant::UserType, status()->certificate().type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(),
            status()->certificate().userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                status()->certificate().value<OxideQSslCertificate>()));
}

TEST_F(SecurityStatusTest, Broken) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATION_BROKEN;
  ssl_status.cert_id = cert_id();
  ssl_status.cert_status = net::CERT_STATUS_COMMON_NAME_INVALID;

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(cert_id(), &cert));

  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelError,
            status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusNormal,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusBadIdentity,
            status()->certStatus());
  EXPECT_FALSE(status()->certificate().isNull());
  EXPECT_EQ(QVariant::UserType, status()->certificate().type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(),
            status()->certificate().userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                status()->certificate().value<OxideQSslCertificate>()));
}

TEST_F(SecurityStatusTest, RanInsecure) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATION_BROKEN;
  ssl_status.cert_id = cert_id();
  ssl_status.content_status = content::SSLStatus::RAN_INSECURE_CONTENT;

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(cert_id(), &cert));

  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelError,
            status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusRanInsecure,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusOk, status()->certStatus());
  EXPECT_FALSE(status()->certificate().isNull());
  EXPECT_EQ(QVariant::UserType, status()->certificate().type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(),
            status()->certificate().userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                status()->certificate().value<OxideQSslCertificate>()));
}

TEST_F(SecurityStatusTest, DisplayedInsecure) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_status.cert_id = cert_id();
  ssl_status.content_status = content::SSLStatus::DISPLAYED_INSECURE_CONTENT;

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(cert_id(), &cert));

  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelWarning,
            status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusDisplayedInsecure,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusOk, status()->certStatus());
  EXPECT_FALSE(status()->certificate().isNull());
  EXPECT_EQ(QVariant::UserType, status()->certificate().type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(),
            status()->certificate().userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                status()->certificate().value<OxideQSslCertificate>()));
}

TEST_F(SecurityStatusTest, DisplayedAndRanInsecure) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATION_BROKEN;
  ssl_status.cert_id = cert_id();
  ssl_status.content_status =
      content::SSLStatus::DISPLAYED_INSECURE_CONTENT |
      content::SSLStatus::RAN_INSECURE_CONTENT;

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(cert_id(), &cert));

  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelError,
            status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusDisplayedInsecure |
                OxideQSecurityStatus::ContentStatusRanInsecure,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusOk, status()->certStatus());
  EXPECT_FALSE(status()->certificate().isNull());
  EXPECT_EQ(QVariant::UserType, status()->certificate().type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(),
            status()->certificate().userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                status()->certificate().value<OxideQSslCertificate>()));
}

TEST_F(SecurityStatusTest, MinorCertError) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_status.cert_id = cert_id();
  ssl_status.cert_status = net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION;

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(cert_id(), &cert));

  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelWarning,
            status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusNormal,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusRevocationCheckFailed,
            status()->certStatus());
  EXPECT_FALSE(status()->certificate().isNull());
  EXPECT_EQ(QVariant::UserType, status()->certificate().type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(),
            status()->certificate().userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                status()->certificate().value<OxideQSslCertificate>()));
}

TEST_F(SecurityStatusTest, SecureEV) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_status.cert_id = cert_id();
  ssl_status.cert_status = net::CERT_STATUS_IS_EV;

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(cert_id(), &cert));

  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelSecureEV,
            status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusNormal,
            status()->contentStatus());
  EXPECT_EQ(OxideQSecurityStatus::CertStatusOk, status()->certStatus());
  EXPECT_FALSE(status()->certificate().isNull());
  EXPECT_EQ(QVariant::UserType, status()->certificate().type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(),
            status()->certificate().userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                status()->certificate().value<OxideQSslCertificate>()));
}

struct CertStatusRow {
  CertStatusRow(net::CertStatus cert_status_in,
                bool expired_cert,
                OxideQSecurityStatus::CertStatus cert_status_out)
      : cert_status_in(cert_status_in),
        expired_cert(expired_cert),
        cert_status_out(cert_status_out) {}

  net::CertStatus cert_status_in;
  bool expired_cert;
  OxideQSecurityStatus::CertStatus cert_status_out;
};

class SecurityStatusCertStatusTest
    : public SecurityStatusTest,
      public testing::WithParamInterface<CertStatusRow> {};

INSTANTIATE_TEST_CASE_P(
    Ok,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(0, false, OxideQSecurityStatus::CertStatusOk)));
INSTANTIATE_TEST_CASE_P(
    CommonNameInvalid,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_COMMON_NAME_INVALID,
                      false,
                      OxideQSecurityStatus::CertStatusBadIdentity)));
INSTANTIATE_TEST_CASE_P(
    AuthorityInvalid,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_AUTHORITY_INVALID,
                      false,
                      OxideQSecurityStatus::CertStatusAuthorityInvalid)));
INSTANTIATE_TEST_CASE_P(
    UnableToCheckRevocation,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION,
                      false,
                      OxideQSecurityStatus::CertStatusRevocationCheckFailed)));
INSTANTIATE_TEST_CASE_P(
    Revoked,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_REVOKED,
                      false,
                      OxideQSecurityStatus::CertStatusRevoked)));
INSTANTIATE_TEST_CASE_P(
    Invalid,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_INVALID,
                      false,
                      OxideQSecurityStatus::CertStatusInvalid)));
INSTANTIATE_TEST_CASE_P(
    WeakSignatureAlgorithm,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM,
                      false,
                      OxideQSecurityStatus::CertStatusInsecure)));
INSTANTIATE_TEST_CASE_P(
    WeakKey,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_WEAK_KEY,
                      false,
                      OxideQSecurityStatus::CertStatusInsecure)));
INSTANTIATE_TEST_CASE_P(
    NoRevocationMechanism,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NO_REVOCATION_MECHANISM,
                      false,
                      OxideQSecurityStatus::CertStatusOk)));
INSTANTIATE_TEST_CASE_P(
    NonUniqueName,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NON_UNIQUE_NAME,
                      false,
                      OxideQSecurityStatus::CertStatusGenericError)));
INSTANTIATE_TEST_CASE_P(
    PinnedKeyMissing,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_PINNED_KEY_MISSING,
                      false,
                      OxideQSecurityStatus::CertStatusGenericError)));
INSTANTIATE_TEST_CASE_P(
    NameConstraintViolation,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NAME_CONSTRAINT_VIOLATION,
                      false,
                      OxideQSecurityStatus::CertStatusGenericError)));
INSTANTIATE_TEST_CASE_P(
    ValidityTooLong,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_VALIDITY_TOO_LONG,
                      false,
                      OxideQSecurityStatus::CertStatusGenericError)));
INSTANTIATE_TEST_CASE_P(
    DateInvalid,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_DATE_INVALID,
                      false,
                      OxideQSecurityStatus::CertStatusDateInvalid)));

INSTANTIATE_TEST_CASE_P(
    OkE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(0, true, OxideQSecurityStatus::CertStatusOk)));
INSTANTIATE_TEST_CASE_P(
    CommonNameInvalidE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_COMMON_NAME_INVALID,
                      true,
                      OxideQSecurityStatus::CertStatusBadIdentity)));
INSTANTIATE_TEST_CASE_P(
    AuthorityInvalidE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_AUTHORITY_INVALID,
                      true,
                      OxideQSecurityStatus::CertStatusAuthorityInvalid)));
INSTANTIATE_TEST_CASE_P(
    UnableToCheckRevocationE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION,
                      true,
                      OxideQSecurityStatus::CertStatusRevocationCheckFailed)));
INSTANTIATE_TEST_CASE_P(
    RevokedE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_REVOKED,
                      true,
                      OxideQSecurityStatus::CertStatusRevoked)));
INSTANTIATE_TEST_CASE_P(
    InvalidE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_INVALID,
                      true,
                      OxideQSecurityStatus::CertStatusInvalid)));
INSTANTIATE_TEST_CASE_P(
    WeakSignatureAlgorithmE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM,
                      true,
                      OxideQSecurityStatus::CertStatusInsecure)));
INSTANTIATE_TEST_CASE_P(
    WeakKeyE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_WEAK_KEY,
                      true,
                      OxideQSecurityStatus::CertStatusInsecure)));
INSTANTIATE_TEST_CASE_P(
    NoRevocationMechanismE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NO_REVOCATION_MECHANISM,
                      true,
                      OxideQSecurityStatus::CertStatusOk)));
INSTANTIATE_TEST_CASE_P(
    NonUniqueNameE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NON_UNIQUE_NAME,
                      true,
                      OxideQSecurityStatus::CertStatusGenericError)));
INSTANTIATE_TEST_CASE_P(
    PinnedKeyMissingE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_PINNED_KEY_MISSING,
                      true,
                      OxideQSecurityStatus::CertStatusGenericError)));
INSTANTIATE_TEST_CASE_P(
    NameConstraintViolationE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_NAME_CONSTRAINT_VIOLATION,
                      true,
                      OxideQSecurityStatus::CertStatusGenericError)));
INSTANTIATE_TEST_CASE_P(
    ValidityTooLongE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_VALIDITY_TOO_LONG,
                      true,
                      OxideQSecurityStatus::CertStatusGenericError)));
INSTANTIATE_TEST_CASE_P(
    DateInvalidE,
    SecurityStatusCertStatusTest,
    testing::Values(
        CertStatusRow(net::CERT_STATUS_DATE_INVALID,
                      true,
                      OxideQSecurityStatus::CertStatusExpired)));

TEST_P(SecurityStatusCertStatusTest, TestCertStatus) {
  const CertStatusRow& row = GetParam();

  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  if (net::IsCertStatusError(row.cert_status_in) &&
      !net::IsCertStatusMinorError(row.cert_status_in)) {
    ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATION_BROKEN;
  } else {
    ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  }
  ssl_status.cert_id = row.expired_cert ? expired_cert_id() : cert_id();
  ssl_status.cert_status = row.cert_status_in;

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(ssl_status.cert_id, &cert));

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  OxideQSecurityStatus::SecurityLevel expected_security_level;
  if (net::IsCertStatusError(row.cert_status_in)) {
    if (net::IsCertStatusMinorError(row.cert_status_in)) {
      expected_security_level = OxideQSecurityStatus::SecurityLevelWarning;
    } else {
      expected_security_level = OxideQSecurityStatus::SecurityLevelError;
    }
  } else {
    expected_security_level = OxideQSecurityStatus::SecurityLevelSecure;
  }

  EXPECT_EQ(expected_security_level, status()->securityLevel());
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusNormal,
            status()->contentStatus());
  EXPECT_EQ(row.cert_status_out, status()->certStatus());
  EXPECT_FALSE(status()->certificate().isNull());
  EXPECT_EQ(QVariant::UserType, status()->certificate().type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(),
            status()->certificate().userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                status()->certificate().value<OxideQSslCertificate>()));
}

namespace {

class Observer : public QObject {
  Q_OBJECT

 public:
  Observer(OxideQSecurityStatus* status);

  int update_count;

  OxideQSecurityStatus::SecurityLevel security_level;
  OxideQSecurityStatus::ContentStatus content_status;
  OxideQSecurityStatus::CertStatus cert_status;
  QVariant cert;

 public Q_SLOTS:
  void OnUpdate();

 private:
  QPointer<OxideQSecurityStatus> status_;
};

Observer::Observer(OxideQSecurityStatus* status)
    : update_count(0),
      security_level(OxideQSecurityStatus::SecurityLevelNone),
      content_status(OxideQSecurityStatus::ContentStatusNormal),
      cert_status(OxideQSecurityStatus::CertStatusOk),
      status_(status) {}

void Observer::OnUpdate() {
  ++update_count;
  security_level = status_->securityLevel();
  content_status = status_->contentStatus();
  cert_status = status_->certStatus();
  cert = status_->certificate();
}

}

TEST_F(SecurityStatusTest, SecurityLevelUpdate) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_status.cert_id = cert_id();

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  std::unique_ptr<Observer> observer = base::WrapUnique(new Observer(status()));
  observer->connect(status(), SIGNAL(securityLevelChanged()),
                    SLOT(OnUpdate()));

  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATION_BROKEN;
  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  EXPECT_EQ(1, observer->update_count);
  EXPECT_EQ(OxideQSecurityStatus::SecurityLevelError, observer->security_level);
}

TEST_F(SecurityStatusTest, ContentStatusUpdate) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_status.cert_id = cert_id();

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  std::unique_ptr<Observer> observer = base::WrapUnique(new Observer(status()));
  observer->connect(status(), SIGNAL(contentStatusChanged()),
                    SLOT(OnUpdate()));

  ssl_status.content_status = content::SSLStatus::DISPLAYED_INSECURE_CONTENT;
  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  EXPECT_EQ(1, observer->update_count);
  EXPECT_EQ(OxideQSecurityStatus::ContentStatusDisplayedInsecure,
            observer->content_status);
}

TEST_F(SecurityStatusTest, CertStatusUpdate) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_status.cert_id = cert_id();

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  std::unique_ptr<Observer> observer = base::WrapUnique(new Observer(status()));
  observer->connect(status(), SIGNAL(certStatusChanged()),
                    SLOT(OnUpdate()));

  ssl_status.cert_status = net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION;
  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  EXPECT_EQ(1, observer->update_count);
  EXPECT_EQ(OxideQSecurityStatus::CertStatusRevocationCheckFailed,
            observer->cert_status);
}

TEST_F(SecurityStatusTest, CertUpdate) {
  content::NavigationController& controller = web_contents()->GetController();
  controller.LoadURL(GURL("http://www.google.com/"),
                     content::Referrer(),
                     ui::PAGE_TRANSITION_TYPED,
                     std::string());
  content::SSLStatus& ssl_status = controller.GetVisibleEntry()->GetSSL();
  ssl_status.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_status.cert_id = cert_id();

  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  std::unique_ptr<Observer> observer = base::WrapUnique(new Observer(status()));
  observer->connect(status(), SIGNAL(certificateChanged()),
                    SLOT(OnUpdate()));

  ssl_status.cert_id = expired_cert_id();
  oxide::SecurityStatus::FromWebContents(web_contents())
      ->VisibleSSLStateChanged();

  scoped_refptr<net::X509Certificate> cert;
  ASSERT_TRUE(cert_store()->RetrieveCert(expired_cert_id(), &cert));

  EXPECT_EQ(1, observer->update_count);
  EXPECT_FALSE(observer->cert.isNull());
  EXPECT_EQ(QVariant::UserType, observer->cert.type());
  EXPECT_EQ(qMetaTypeId<OxideQSslCertificate>(), observer->cert.userType());
  EXPECT_EQ(cert.get(),
            OxideQSslCertificateData::GetX509Certificate(
                observer->cert.value<OxideQSslCertificate>()));
}

} // namespace qt
} // namespace oxide

#include "qt/core/browser/ssl/oxide_qt_security_status_unittest.moc"
