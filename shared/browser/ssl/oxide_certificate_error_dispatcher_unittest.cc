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

#include "shared/browser/oxide_security_types.h"
#include "shared/common/oxide_enum_flags.h"
#include "shared/test/oxide_test_browser_thread_bundle.h"

#include "oxide_certificate_error.h"
#include "oxide_certificate_error_dispatcher.h"
#include "oxide_certificate_error_placeholder_page.h"

using oxide::CertificateErrorDispatcher;

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

  oxide::CertificateError* last_error() const { return last_error_.get(); }

  int response_count() const { return response_count_; }
  bool last_response() const { return last_response_; }

  base::Callback<void(bool)> CreateResponseCallback();

  void AttachCertificateErrorCallback(content::WebContents* contents,
                                      bool save = true);

 private:
  void SetUp() override;
  void TearDown() override;

  void OnCertificateError(bool save,
                          std::unique_ptr<oxide::CertificateError> error);
  void OnResponse(bool response);

  oxide::TestBrowserThreadBundle browser_thread_bundle_;
  content::TestBrowserContext browser_context_;
  content::TestWebContentsFactory web_contents_factory_;

  content::WebContents* web_contents_;

  std::unique_ptr<oxide::CertificateError> last_error_;

  int response_count_;
  bool last_response_;
};

content::WebContents* CertificateErrorDispatcherTest::CreateWebContents() {
  return web_contents_factory_.CreateWebContents(&browser_context_);
}

base::Callback<void(bool)>
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
  oxide::CertificateErrorPlaceholderPage::SetDontCreateViewForTesting(true);
}

void CertificateErrorDispatcherTest::TearDown() {
  oxide::CertificateErrorPlaceholderPage::SetDontCreateViewForTesting(false);
}

void CertificateErrorDispatcherTest::OnCertificateError(
    bool save,
    std::unique_ptr<oxide::CertificateError> error) {
  if (!save) {
    return;
  }
  last_error_ = std::move(error);
}

void CertificateErrorDispatcherTest::OnResponse(bool response) {
  ++response_count_;
  last_response_ = response;
}

CertificateErrorDispatcherTest::CertificateErrorDispatcherTest()
    : web_contents_(nullptr),
      response_count_(0),
      last_response_(false) {}

CertificateErrorDispatcherTest::~CertificateErrorDispatcherTest() {
  web_contents_ = nullptr;
}

// Test that we deny an error if there's no dispatcher associated with
// the WebContents
TEST_F(CertificateErrorDispatcherTest, NoDispatcher) {
  content::WebContents* contents = CreateWebContents();
  ASSERT_EQ(CertificateErrorDispatcher::FromWebContents(contents), nullptr);

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY);
  EXPECT_EQ(response_count(), 0);
}

// Test that we deny an error if there is no client callback registered with
// the WebContents
TEST_F(CertificateErrorDispatcherTest, NullCallback) {
  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY);
  EXPECT_EQ(response_count(), 0);
}

// Test that the dispatcher creates a CertificateError with is_main_frame set
// correctly
TEST_F(CertificateErrorDispatcherTest, is_main_frame) {
  AttachCertificateErrorCallback(web_contents());

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE);
  ASSERT_NE(last_error(), nullptr);
  EXPECT_TRUE(last_error()->is_main_frame());

  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      false, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_SUB_FRAME,
      true, // overridable
      false, // strict_enforcement
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY);
  EXPECT_FALSE(last_error()->is_main_frame());
}

struct CertErrorRow {
  CertErrorRow(int net_error, oxide::CertError cert_error, bool expired_cert)
      : net_error(net_error),
        cert_error(cert_error),
        expired_cert(expired_cert) {}

  int net_error;
  oxide::CertError cert_error;
  bool expired_cert;
};

class CertificateErrorDispatcherCertErrorConversionTest
    : public CertificateErrorDispatcherTest,
      public testing::WithParamInterface<CertErrorRow> {};

#define COMMON_CASES(expired_cert) \
    CertErrorRow(net::ERR_CERT_COMMON_NAME_INVALID, \
                 oxide::CERT_ERROR_BAD_IDENTITY, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_AUTHORITY_INVALID, \
                 oxide::CERT_ERROR_AUTHORITY_INVALID, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_CONTAINS_ERRORS, \
                 oxide::CERT_ERROR_INVALID, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_NO_REVOCATION_MECHANISM, \
                 oxide::CERT_ERROR_GENERIC, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_UNABLE_TO_CHECK_REVOCATION, \
                 oxide::CERT_ERROR_GENERIC, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_REVOKED, \
                 oxide::CERT_ERROR_REVOKED, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_INVALID, \
                 oxide::CERT_ERROR_INVALID, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM, \
                 oxide::CERT_ERROR_INSECURE, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_NON_UNIQUE_NAME, \
                 oxide::CERT_ERROR_GENERIC, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_WEAK_KEY, \
                 oxide::CERT_ERROR_INSECURE, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_NAME_CONSTRAINT_VIOLATION, \
                 oxide::CERT_ERROR_GENERIC, \
                 expired_cert), \
    CertErrorRow(net::ERR_CERT_VALIDITY_TOO_LONG, \
                 oxide::CERT_ERROR_INVALID, \
                 expired_cert)

INSTANTIATE_TEST_CASE_P(
    CertErrorsNotExpiredCert,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(COMMON_CASES(false),
                    CertErrorRow(net::ERR_CERT_DATE_INVALID,
                                 oxide::CERT_ERROR_DATE_INVALID,
                                 false)));
INSTANTIATE_TEST_CASE_P(
    CertErrorsExpiredCert,
    CertificateErrorDispatcherCertErrorConversionTest,
    testing::Values(COMMON_CASES(true),
                    CertErrorRow(net::ERR_CERT_DATE_INVALID,
                                 oxide::CERT_ERROR_EXPIRED,
                                 true)));

// Test that the dispatcher creates a CertificateError with cert_error set
// correctly
TEST_P(CertificateErrorDispatcherCertErrorConversionTest, ToCertError) {
  AttachCertificateErrorCallback(web_contents());

  const CertErrorRow& row = GetParam();

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE);
  ASSERT_NE(last_error(), nullptr);
  EXPECT_EQ(last_error()->cert_error(), row.cert_error);
}

// Test that the dispatcher creates a CertificateError with the supplied
// certificate
TEST_F(CertificateErrorDispatcherTest, ssl_info) {
  AttachCertificateErrorCallback(web_contents());

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE);
  ASSERT_NE(last_error(), nullptr);
  EXPECT_EQ(last_error()->cert(), ssl_info.cert.get());
}

// Test that the dispatcher creates a CertificateError with the correct url
TEST_F(CertificateErrorDispatcherTest, request_url) {
  AttachCertificateErrorCallback(web_contents());

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE);
  ASSERT_NE(last_error(), nullptr);
  EXPECT_EQ(last_error()->url(), GURL("https://www.foo.com/"));
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
    ResourceTypes,
    CertificateErrorDispatcherResourceTypeTest,
    testing::Values(
        ResourceTypeRow(content::RESOURCE_TYPE_MAIN_FRAME,
                        true,
                        content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE,
                        true, false),
        // Applications might want to show an error UI for non-overridable
        // RESOURCE_TYPE_MAIN_FRAME errors, so the result here should be CANCEL
        ResourceTypeRow(content::RESOURCE_TYPE_MAIN_FRAME,
                        false,
                        content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL,
                        false, false),
        // Always deny !RESOURCE_TYPE_MAIN_FRAME errors
        ResourceTypeRow(content::RESOURCE_TYPE_SUB_FRAME,
                        true,
                        content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY,
                        false, false),
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

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
  net::SSLInfo ssl_info;
  ssl_info.cert = CreateCertificate();
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      row.resource_type == content::RESOURCE_TYPE_SUB_FRAME ? false : true,
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      row.resource_type,
      row.overridable_in,
      false, // strict_enforcement
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, row.result);
  ASSERT_NE(last_error(), nullptr);
  EXPECT_EQ(last_error()->overridable(), row.overridable_out);
  EXPECT_EQ(last_error()->is_subresource(), row.is_subresource);
}

// Test that the dispatcher creates a CertificateError with strict_enforcement
// set correctly
TEST_F(CertificateErrorDispatcherTest, strict_enforcement) {
  AttachCertificateErrorCallback(web_contents());

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE);
  ASSERT_NE(last_error(), nullptr);
  EXPECT_FALSE(last_error()->strict_enforcement());
  
  CertificateErrorDispatcher::AllowCertificateError(
      web_contents(),
      true, // is_main_frame
      net::ERR_CERT_COMMON_NAME_INVALID,
      ssl_info,
      GURL("https://www.foo.com/"),
      content::RESOURCE_TYPE_MAIN_FRAME,
      false, // overridable
      true, // strict_enforcement
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL);
  EXPECT_TRUE(last_error()->strict_enforcement());
}

// Test that the CertificateError created by the dispatcher responds via the
// callback supplied by content
TEST_F(CertificateErrorDispatcherTest, Response) {
  AttachCertificateErrorCallback(web_contents());

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE);
  ASSERT_NE(last_error(), nullptr);

  last_error()->Allow();

  EXPECT_EQ(response_count(), 1);
  EXPECT_TRUE(last_response());
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

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE);
  EXPECT_NE(last_error(), nullptr);
  EXPECT_EQ(delegate->changed_flags(), content::INVALIDATE_TYPE_ALL);
  EXPECT_EQ(web_contents()->GetVisibleURL(), GURL("https://www.foo.com/"));
  EXPECT_NE(web_contents()->GetController().GetTransientEntry(), nullptr);

  web_contents()->SetDelegate(nullptr);
}

// Test that the dispatcher does not create a placeholder page for errors where
// the resource type is not content::RESOURCE_TYPE_MAIN_FRAME
TEST_F(CertificateErrorDispatcherTest, NoPlaceholderPageForNonMainFrame) {
  AttachCertificateErrorCallback(web_contents());

  std::unique_ptr<TestWebContentsDelegate> delegate =
      base::WrapUnique(new TestWebContentsDelegate());
  web_contents()->SetDelegate(delegate.get());

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY);
  EXPECT_NE(last_error(), nullptr);
  EXPECT_EQ(delegate->changed_flags(), static_cast<content::InvalidateTypes>(0));
  EXPECT_NE(web_contents()->GetVisibleURL(), GURL("https://www.foo.com/"));
  EXPECT_EQ(web_contents()->GetController().GetTransientEntry(), nullptr);

  web_contents()->SetDelegate(nullptr);
}

// Test that the dispatcher does not create a placeholder page for errors that
// are responded to synchronously
TEST_F(CertificateErrorDispatcherTest, NoPlaceholderForSyncResponse) {
  AttachCertificateErrorCallback(web_contents(), false);

  std::unique_ptr<TestWebContentsDelegate> delegate =
      base::WrapUnique(new TestWebContentsDelegate());
  web_contents()->SetDelegate(delegate.get());

  content::CertificateRequestResultType result =
      content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE;
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
      CreateResponseCallback(),
      &result);

  EXPECT_EQ(result, content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE);
  EXPECT_EQ(response_count(), 1);
  EXPECT_FALSE(last_response());
  EXPECT_EQ(delegate->changed_flags(), static_cast<content::InvalidateTypes>(0));
  EXPECT_NE(web_contents()->GetVisibleURL(), GURL("https://www.foo.com/"));
  EXPECT_EQ(web_contents()->GetController().GetTransientEntry(), nullptr);

  web_contents()->SetDelegate(nullptr);
}
