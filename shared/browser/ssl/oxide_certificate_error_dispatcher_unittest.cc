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

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "content/public/browser/certificate_request_result_type.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/resource_type.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_web_contents_factory.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/ssl_info.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

#include "shared/common/oxide_enum_flags.h"
#include "shared/test/test_browser_thread_bundle.h"

#include "oxide_certificate_error.h"
#include "oxide_certificate_error_dispatcher.h"
#include "oxide_certificate_error_placeholder_page.h"
#include "oxide_security_types.h"

namespace oxide {

namespace {

scoped_refptr<net::X509Certificate> CreateCertificate(bool expired = false) {
  return make_scoped_refptr(
      new net::X509Certificate("https://www.google.com/",
                               "https://www.example.com/",
                               base::Time::UnixEpoch(),
                               base::Time::Now() +
                                   base::TimeDelta::FromDays(expired ? -1 : 1)));
}

}

class CertificateErrorDispatcherTest : public testing::Test {
 public:
  CertificateErrorDispatcherTest();
  ~CertificateErrorDispatcherTest() override;

 protected:
  content::WebContents* CreateWebContents();

  content::WebContents* web_contents() const { return web_contents_; }

  CertificateError* last_error() const { return last_error_.get(); }

  void ClearLastError();

  int response_count() const { return response_count_; }
  content::CertificateRequestResultType last_response() const {
    return last_response_;
  }

  base::Callback<void(content::CertificateRequestResultType)>
  CreateResponseCallback();

  void AttachCertificateErrorCallback(content::WebContents* contents,
                                      bool save = true);

 private:
  void SetUp() override;
  void TearDown() override;

  void OnCertificateError(bool save,
                          std::unique_ptr<CertificateError> error);
  void OnResponse(content::CertificateRequestResultType response);

  TestBrowserThreadBundle browser_thread_bundle_;
  content::TestBrowserContext browser_context_;
  content::TestWebContentsFactory web_contents_factory_;

  content::WebContents* web_contents_;

  std::unique_ptr<CertificateError> last_error_;

  int response_count_;
  content::CertificateRequestResultType last_response_;
};

content::WebContents* CertificateErrorDispatcherTest::CreateWebContents() {
  return web_contents_factory_.CreateWebContents(&browser_context_);
}

void CertificateErrorDispatcherTest::ClearLastError() {
  last_error_.reset();
}

base::Callback<void(content::CertificateRequestResultType)>
CertificateErrorDispatcherTest::CreateResponseCallback() {
  return base::Bind(&CertificateErrorDispatcherTest::OnResponse,
                    base::Unretained(this));
}

void CertificateErrorDispatcherTest::AttachCertificateErrorCallback(
    content::WebContents* contents, bool save) {
  CertificateErrorDispatcher::FromWebContents(contents)->SetCallback(
      base::Bind(&CertificateErrorDispatcherTest::OnCertificateError,
                 base::Unretained(this), save));
}

void CertificateErrorDispatcherTest::SetUp() {
  web_contents_ = CreateWebContents();
  CertificateErrorDispatcher::CreateForWebContents(web_contents_);
  CertificateErrorPlaceholderPage::SetDontCreateViewForTesting(true);
}

void CertificateErrorDispatcherTest::TearDown() {
  CertificateErrorPlaceholderPage::SetDontCreateViewForTesting(false);
}

void CertificateErrorDispatcherTest::OnCertificateError(
    bool save,
    std::unique_ptr<CertificateError> error) {
  if (!save) {
    return;
  }
  last_error_ = std::move(error);
}

void CertificateErrorDispatcherTest::OnResponse(
    content::CertificateRequestResultType response) {
  ++response_count_;
  last_response_ = response;
}

CertificateErrorDispatcherTest::CertificateErrorDispatcherTest()
    : web_contents_(nullptr),
      response_count_(0),
      last_response_(content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY) {}

CertificateErrorDispatcherTest::~CertificateErrorDispatcherTest() {
  web_contents_ = nullptr;
}

// Test that we deny an error if there's no dispatcher associated with
// the WebContents
TEST_F(CertificateErrorDispatcherTest, NoDispatcher) {
  content::WebContents* contents = CreateWebContents();
  ASSERT_EQ(CertificateErrorDispatcher::FromWebContents(contents), nullptr);

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      contents,
      true, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY, last_response());
  EXPECT_EQ(1, response_count());
}

// Test that we deny an error if there is no client callback registered with
// the WebContents
TEST_F(CertificateErrorDispatcherTest, NullCallback) {
  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY, last_response());
  EXPECT_EQ(1, response_count());
}

// Test that the dispatcher creates a CertificateError with is_main_frame set
// correctly
TEST_F(CertificateErrorDispatcherTest, is_main_frame) {
  AttachCertificateErrorCallback(web_contents());

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(0, response_count());
  ASSERT_NE(nullptr, last_error());
  EXPECT_TRUE(last_error()->is_main_frame());
  EXPECT_FALSE(last_error()->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, last_error()->cert_error());
  EXPECT_EQ(ssl_info.cert.get(), last_error()->cert());
  EXPECT_EQ(GURL("https://www.foo.com/"), last_error()->url());
  EXPECT_TRUE(last_error()->overridable());
  EXPECT_FALSE(last_error()->strict_enforcement());
  EXPECT_FALSE(last_error()->IsCancelled());

  ClearLastError();
  EXPECT_EQ(1, response_count());
  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL, last_response());

  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      false, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_SUB_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(2, response_count());
  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY, last_response());
  EXPECT_FALSE(last_error()->is_main_frame());
  EXPECT_FALSE(last_error()->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, last_error()->cert_error());
  EXPECT_EQ(ssl_info.cert.get(), last_error()->cert());
  EXPECT_EQ(GURL("https://www.foo.com/"), last_error()->url());
  EXPECT_FALSE(last_error()->overridable());
  EXPECT_FALSE(last_error()->strict_enforcement());
  EXPECT_FALSE(last_error()->IsCancelled());
}

struct CertErrorRow {
  CertErrorRow(int net_error, CertError cert_error, bool expired_cert)
      : net_error(net_error),
        cert_error(cert_error),
        expired_cert(expired_cert) {}

  int net_error;
  CertError cert_error;
  bool expired_cert;
};

class CertificateErrorDispatcherCertErrorConversionTest
    : public CertificateErrorDispatcherTest,
      public testing::WithParamInterface<CertErrorRow> {};

INSTANTIATE_TEST_CASE_P(
    CommonNameInvalid,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_COMMON_NAME_INVALID,
                                 CERT_ERROR_BAD_IDENTITY, false)));
INSTANTIATE_TEST_CASE_P(
    AuthorityInvalid,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_AUTHORITY_INVALID,
                                 CERT_ERROR_AUTHORITY_INVALID, false)));
INSTANTIATE_TEST_CASE_P(
    ContainsErrors,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_CONTAINS_ERRORS,
                                 CERT_ERROR_INVALID, false)));
INSTANTIATE_TEST_CASE_P(
    NoRevocationMechanism,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_NO_REVOCATION_MECHANISM,
                                 CERT_ERROR_GENERIC, false)));
INSTANTIATE_TEST_CASE_P(
    UnableToCheckRevocation,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_UNABLE_TO_CHECK_REVOCATION,
                                 CERT_ERROR_GENERIC, false)));
INSTANTIATE_TEST_CASE_P(
    Revoked,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_REVOKED,
                                 CERT_ERROR_REVOKED, false)));
INSTANTIATE_TEST_CASE_P(
    Invalid,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_INVALID,
                                 CERT_ERROR_INVALID, false)));
INSTANTIATE_TEST_CASE_P(
    WeakSignatureAlgorithm,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM,
                                 CERT_ERROR_INSECURE, false)));
INSTANTIATE_TEST_CASE_P(
    NonUniqueName,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_NON_UNIQUE_NAME,
                                 CERT_ERROR_GENERIC, false)));
INSTANTIATE_TEST_CASE_P(
    WeakKey,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_WEAK_KEY,
                                 CERT_ERROR_INSECURE, false)));
INSTANTIATE_TEST_CASE_P(
    NameConstraintViolation,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_NAME_CONSTRAINT_VIOLATION,
                                 CERT_ERROR_GENERIC, false)));
INSTANTIATE_TEST_CASE_P(
    ValidityTooLong,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_VALIDITY_TOO_LONG,
                                 CERT_ERROR_INVALID, false)));
INSTANTIATE_TEST_CASE_P(
    DateInvalid,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_DATE_INVALID,
                                 CERT_ERROR_DATE_INVALID, false)));
INSTANTIATE_TEST_CASE_P(
    CommonNameInvalidE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_COMMON_NAME_INVALID,
                                 CERT_ERROR_BAD_IDENTITY, true)));
INSTANTIATE_TEST_CASE_P(
    AuthorityInvalidE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_AUTHORITY_INVALID,
                                 CERT_ERROR_AUTHORITY_INVALID, true)));
INSTANTIATE_TEST_CASE_P(
    ContainsErrorsE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_CONTAINS_ERRORS,
                                 CERT_ERROR_INVALID, true)));
INSTANTIATE_TEST_CASE_P(
    NoRevocationMechanismE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_NO_REVOCATION_MECHANISM,
                                 CERT_ERROR_GENERIC, true)));
INSTANTIATE_TEST_CASE_P(
    UnableToCheckRevocationE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_UNABLE_TO_CHECK_REVOCATION,
                                 CERT_ERROR_GENERIC, true)));
INSTANTIATE_TEST_CASE_P(
    RevokedE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_REVOKED,
                                 CERT_ERROR_REVOKED, true)));
INSTANTIATE_TEST_CASE_P(
    InvalidE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_INVALID,
                                 CERT_ERROR_INVALID, true)));
INSTANTIATE_TEST_CASE_P(
    WeakSignatureAlgorithmE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM,
                                 CERT_ERROR_INSECURE, true)));
INSTANTIATE_TEST_CASE_P(
    NonUniqueNameE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_NON_UNIQUE_NAME,
                                 CERT_ERROR_GENERIC, true)));
INSTANTIATE_TEST_CASE_P(
    WeakKeyE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_WEAK_KEY,
                                 CERT_ERROR_INSECURE, true)));
INSTANTIATE_TEST_CASE_P(
    NameConstraintViolationE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_NAME_CONSTRAINT_VIOLATION,
                                 CERT_ERROR_GENERIC, true)));
INSTANTIATE_TEST_CASE_P(
    ValidityTooLongE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_VALIDITY_TOO_LONG,
                                 CERT_ERROR_INVALID, true)));
INSTANTIATE_TEST_CASE_P(
    DateInvalidE,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(CertErrorRow(net::ERR_CERT_DATE_INVALID,
                                 CERT_ERROR_EXPIRED, true)));

// Test that the dispatcher creates a CertificateError with cert_error set
// correctly
TEST_P(CertificateErrorDispatcherCertErrorConversionTest, ToCertError) {
  AttachCertificateErrorCallback(web_contents());

  const CertErrorRow& row = GetParam();

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate(row.expired_cert);
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame
      row.net_error,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(0, response_count());
  ASSERT_NE(nullptr, last_error());
  EXPECT_TRUE(last_error()->is_main_frame());
  EXPECT_FALSE(last_error()->is_subresource());
  EXPECT_EQ(row.cert_error, last_error()->cert_error());
  EXPECT_EQ(ssl_info.cert.get(), last_error()->cert());
  EXPECT_EQ(GURL("https://www.foo.com/"), last_error()->url());
  EXPECT_TRUE(last_error()->overridable());
  EXPECT_FALSE(last_error()->strict_enforcement());
  EXPECT_FALSE(last_error()->IsCancelled());
}

struct ResourceTypeRow {
  ResourceTypeRow(content::ResourceType resource_type,
                  bool overridable_in,
                  content::CertificateRequestResultType result,
                  bool overridable_out,
                  bool is_subresource)
      : resource_type(resource_type),
        overridable_in(overridable_in),
        result(result),
        overridable_out(overridable_out),
        is_subresource(is_subresource) {}

  content::ResourceType resource_type;
  bool overridable_in;
  content::CertificateRequestResultType result;
  bool overridable_out;
  bool is_subresource;
};

class CertificateErrorDispatcherResourceTypeTest
    : public CertificateErrorDispatcherTest,
      public testing::WithParamInterface<ResourceTypeRow> {};

INSTANTIATE_TEST_CASE_P(
    MainFrameOverridable,
    CertificateErrorDispatcherResourceTypeTest,
    testing::Values(
        ResourceTypeRow(content::RESOURCE_TYPE_MAIN_FRAME,
                        true,
                        content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE,
                        true, false)));
// Applications might want to show an error UI for non-overridable
// RESOURCE_TYPE_MAIN_FRAME errors, so the result here should be CANCEL
INSTANTIATE_TEST_CASE_P(
    MainFrameNonOverridable,
    CertificateErrorDispatcherResourceTypeTest,
    testing::Values(
        ResourceTypeRow(content::RESOURCE_TYPE_MAIN_FRAME,
                        false,
                        content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL,
                        false, false)));
// Always deny !RESOURCE_TYPE_MAIN_FRAME errors
INSTANTIATE_TEST_CASE_P(
    SubFrame,
    CertificateErrorDispatcherResourceTypeTest,
    testing::Values(
        ResourceTypeRow(content::RESOURCE_TYPE_SUB_FRAME,
                        true,
                        content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY,
                        false, false)));
INSTANTIATE_TEST_CASE_P(
    Script,
    CertificateErrorDispatcherResourceTypeTest,
    testing::Values(
        ResourceTypeRow(content::RESOURCE_TYPE_SCRIPT,
                        true,
                        content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY,
                        false, true)));

// Test that the dispatcher creates a CertificateError with overridable and
// is_subresource set correctly and responds with the correct result, depending
// on the supplied ResourceType and whether content allows overriding
TEST_P(CertificateErrorDispatcherResourceTypeTest, TestResourceType) {
  AttachCertificateErrorCallback(web_contents());

  const ResourceTypeRow& row = GetParam();

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  bool is_main_frame =
      row.resource_type == content::RESOURCE_TYPE_SUB_FRAME ? false : true;
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      is_main_frame,
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      row.resource_type,
      row.overridable_in,
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(row.overridable_out ? 0 : 1, response_count());
  if (!row.overridable_out) {
    EXPECT_EQ(row.result, last_response());
  }
  ASSERT_NE(nullptr, last_error());
  EXPECT_EQ(is_main_frame, last_error()->is_main_frame());
  EXPECT_EQ(row.is_subresource, last_error()->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, last_error()->cert_error());
  EXPECT_EQ(ssl_info.cert.get(), last_error()->cert());
  EXPECT_EQ(GURL("https://www.foo.com/"), last_error()->url());
  EXPECT_EQ(row.overridable_out, last_error()->overridable());
  EXPECT_FALSE(last_error()->strict_enforcement());
  EXPECT_FALSE(last_error()->IsCancelled());
}

// Test that the dispatcher creates a CertificateError with strict_enforcement
// set correctly
TEST_F(CertificateErrorDispatcherTest, strict_enforcement) {
  AttachCertificateErrorCallback(web_contents());

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(0, response_count());
  ASSERT_NE(nullptr, last_error());
  EXPECT_TRUE(last_error()->is_main_frame());
  EXPECT_FALSE(last_error()->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, last_error()->cert_error());
  EXPECT_EQ(ssl_info.cert.get(), last_error()->cert());
  EXPECT_EQ(GURL("https://www.foo.com/"), last_error()->url());
  EXPECT_TRUE(last_error()->overridable());
  EXPECT_FALSE(last_error()->strict_enforcement());
  EXPECT_FALSE(last_error()->IsCancelled());

  ClearLastError();
  EXPECT_EQ(1, response_count());
  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL, last_response());

  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      false, // overridable
      true, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(2, response_count());
  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL, last_response());
  EXPECT_TRUE(last_error()->is_main_frame());
  EXPECT_FALSE(last_error()->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, last_error()->cert_error());
  EXPECT_EQ(ssl_info.cert.get(), last_error()->cert());
  EXPECT_EQ(GURL("https://www.foo.com/"), last_error()->url());
  EXPECT_FALSE(last_error()->overridable());
  EXPECT_TRUE(last_error()->strict_enforcement());
  EXPECT_FALSE(last_error()->IsCancelled());
}

// Test that the CertificateError created by the dispatcher responds via the
// callback supplied by content
TEST_F(CertificateErrorDispatcherTest, Response) {
  AttachCertificateErrorCallback(web_contents());

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(0, response_count());
  ASSERT_NE(nullptr, last_error());

  last_error()->Allow();

  EXPECT_EQ(1, response_count());
  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE, last_response());
}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(content::InvalidateTypes)

namespace {

class TestWebContentsDelegate : public content::WebContentsDelegate {
 public:
  TestWebContentsDelegate()
      : changed_flags_(static_cast<content::InvalidateTypes>(0)) {}

  content::InvalidateTypes changed_flags() const {
    return changed_flags_;
  }

 private:
  // content::WebContentsDelegate implementation
  void NavigationStateChanged(content::WebContents* source,
                              content::InvalidateTypes changed_flags) override;

  content::InvalidateTypes changed_flags_;
};

void TestWebContentsDelegate::NavigationStateChanged(
    content::WebContents* source,
    content::InvalidateTypes changed_flags) {
  changed_flags_ |= changed_flags;
}

}

// Test that the dispatcher creates a placeholder page for errors where the
// resource type is content::RESOURCE_TYPE_MAIN_FRAME
TEST_F(CertificateErrorDispatcherTest, PlaceholderPage) {
  AttachCertificateErrorCallback(web_contents());

  std::unique_ptr<TestWebContentsDelegate> delegate =
      base::WrapUnique(new TestWebContentsDelegate());
  web_contents()->SetDelegate(delegate.get());

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame,
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(0, response_count());
  EXPECT_NE(nullptr, last_error());
  EXPECT_EQ(content::INVALIDATE_TYPE_ALL, delegate->changed_flags());
  EXPECT_EQ(GURL("https://www.foo.com/"), web_contents()->GetVisibleURL());
  EXPECT_NE(nullptr, web_contents()->GetController().GetTransientEntry());

  web_contents()->SetDelegate(nullptr);
}

// Test that the dispatcher does not create a placeholder page for errors where
// the resource type is not content::RESOURCE_TYPE_MAIN_FRAME
TEST_F(CertificateErrorDispatcherTest, NoPlaceholderPageForNonMainFrame) {
  AttachCertificateErrorCallback(web_contents());

  std::unique_ptr<TestWebContentsDelegate> delegate =
      base::WrapUnique(new TestWebContentsDelegate());
  web_contents()->SetDelegate(delegate.get());

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame,
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_SCRIPT,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(1, response_count());
  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY, last_response());
  EXPECT_NE(nullptr, last_error());
  EXPECT_EQ(static_cast<content::InvalidateTypes>(0),
            delegate->changed_flags());
  EXPECT_NE(GURL("https://www.foo.com/"), web_contents()->GetVisibleURL());
  EXPECT_EQ(nullptr, web_contents()->GetController().GetTransientEntry());

  web_contents()->SetDelegate(nullptr);
}

// Test that the dispatcher does not create a placeholder page for errors that
// are responded to synchronously
TEST_F(CertificateErrorDispatcherTest, NoPlaceholderForSyncResponse) {
  AttachCertificateErrorCallback(web_contents(), false);

  std::unique_ptr<TestWebContentsDelegate> delegate =
      base::WrapUnique(new TestWebContentsDelegate());
  web_contents()->SetDelegate(delegate.get());

  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame,
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback());

  EXPECT_EQ(1, response_count());
  EXPECT_EQ(content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL, last_response());
  EXPECT_EQ(static_cast<content::InvalidateTypes>(0),
            delegate->changed_flags());
  EXPECT_NE(GURL("https://www.foo.com/"), web_contents()->GetVisibleURL());
  EXPECT_EQ(nullptr, web_contents()->GetController().GetTransientEntry());

  web_contents()->SetDelegate(nullptr);
}

} // namespace oxide
