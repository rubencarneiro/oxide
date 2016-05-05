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
#include "base/time/time.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_web_contents_factory.h"
#include "net/cert/x509_certificate.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "url/gurl.h"

#include "shared/common/oxide_enum_flags.h"
#include "shared/test/oxide_test_browser_thread_bundle.h"

#include "oxide_certificate_error.h"
#include "oxide_certificate_error_placeholder_page.h"
#include "oxide_security_types.h"

namespace oxide {

struct Params {
  bool is_main_frame = true;
  bool is_subresource = false;
  CertError cert_error = CERT_ERROR_BAD_IDENTITY;
  bool overridable = true;
  bool strict_enforcement = false;
};

class CertificateErrorTest : public testing::Test {
 protected:
  CertificateErrorTest()
      : cert_(new net::X509Certificate("https://www.google.com/",
                                       "https://www.example.com/",
                                       base::Time(), base::Time())),
        responded_count_(0),
        last_response_(false) {}

  std::unique_ptr<CertificateError> CreateError(
      const Params& params = Params());

  net::X509Certificate* cert() const { return cert_.get(); }

  int responded_count() const { return responded_count_; }
  bool last_response() const { return last_response_; }

  void OnResponse(bool response);

 private:
  scoped_refptr<net::X509Certificate> cert_;

  int responded_count_;
  int last_response_;
};

void CertificateErrorTest::OnResponse(bool response) {
  ++responded_count_;
  last_response_ = response;
}

std::unique_ptr<CertificateError> CertificateErrorTest::CreateError(
    const Params& params) {
  return CertificateError::CreateForTesting(
      params.is_main_frame,
      params.is_subresource,
      params.cert_error,
      cert_.get(),
      GURL("https://www.google.com/"),
      params.strict_enforcement,
      params.overridable,
      base::Bind(&CertificateErrorTest::OnResponse,
                 base::Unretained(this)));
}

// Test that constructing an error sets properties as expected
TEST_F(CertificateErrorTest, Construct) {
  std::unique_ptr<CertificateError> error = CreateError();
  EXPECT_TRUE(error->is_main_frame());
  EXPECT_FALSE(error->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, error->cert_error());
  EXPECT_EQ(cert(), error->cert());
  EXPECT_EQ(GURL("https://www.google.com/"), error->url());
  EXPECT_TRUE(error->overridable());
  EXPECT_FALSE(error->strict_enforcement());
  EXPECT_FALSE(error->IsCancelled());
}

// Test that is_main_frame can be flipped correctly
TEST_F(CertificateErrorTest, is_main_frame) {
  Params params;
  params.is_main_frame = false;

  std::unique_ptr<CertificateError> error = CreateError(params);
  EXPECT_FALSE(error->is_main_frame());
  EXPECT_FALSE(error->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, error->cert_error());
  EXPECT_EQ(cert(), error->cert());
  EXPECT_EQ(GURL("https://www.google.com/"), error->url());
  EXPECT_TRUE(error->overridable());
  EXPECT_FALSE(error->strict_enforcement());
  EXPECT_FALSE(error->IsCancelled());
}

// Test that is_subresource can be flipped correctly
TEST_F(CertificateErrorTest, is_subresource) {
  Params params;
  params.is_subresource = true;

  std::unique_ptr<CertificateError> error = CreateError(params);
  EXPECT_TRUE(error->is_main_frame());
  EXPECT_TRUE(error->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, error->cert_error());
  EXPECT_EQ(cert(), error->cert());
  EXPECT_EQ(GURL("https://www.google.com/"), error->url());
  EXPECT_TRUE(error->overridable());
  EXPECT_FALSE(error->strict_enforcement());
  EXPECT_FALSE(error->IsCancelled());
}

// Test that cert_error is set correctly
TEST_F(CertificateErrorTest, cert_error) {
  Params params;
  params.cert_error = CERT_ERROR_GENERIC;

  std::unique_ptr<CertificateError> error = CreateError(params);
  EXPECT_TRUE(error->is_main_frame());
  EXPECT_FALSE(error->is_subresource());
  EXPECT_EQ(CERT_ERROR_GENERIC, error->cert_error());
  EXPECT_EQ(cert(), error->cert());
  EXPECT_EQ(GURL("https://www.google.com/"), error->url());
  EXPECT_TRUE(error->overridable());
  EXPECT_FALSE(error->strict_enforcement());
  EXPECT_FALSE(error->IsCancelled());
}

// Test that overridable is set correctly and that CertificateError ignores
// calls to Allow()
TEST_F(CertificateErrorTest, overridable) {
  Params params;
  params.overridable = false;

  std::unique_ptr<CertificateError> error = CreateError(params);
  EXPECT_TRUE(error->is_main_frame());
  EXPECT_FALSE(error->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, error->cert_error());
  EXPECT_EQ(cert(), error->cert());
  EXPECT_EQ(GURL("https://www.google.com/"), error->url());
  EXPECT_FALSE(error->overridable());
  EXPECT_FALSE(error->strict_enforcement());
  EXPECT_FALSE(error->IsCancelled());

  ASSERT_EQ(0, responded_count());

  error->Allow();
  ASSERT_EQ(0, responded_count());

  error.reset();
  ASSERT_EQ(0, responded_count());
}

// Test that strict_enforcement is set correctly
TEST_F(CertificateErrorTest, strict_enforcement) {
  Params params;
  params.strict_enforcement = true;

  std::unique_ptr<CertificateError> error = CreateError(params);
  EXPECT_TRUE(error->is_main_frame());
  EXPECT_FALSE(error->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, error->cert_error());
  EXPECT_EQ(cert(), error->cert());
  EXPECT_EQ(GURL("https://www.google.com/"), error->url());
  EXPECT_TRUE(error->overridable());
  EXPECT_TRUE(error->strict_enforcement());
  EXPECT_FALSE(error->IsCancelled());
}

namespace {

void CancelCallback(bool* cancelled) {
  *cancelled = true;
}

}

// Test that cancellation works and that calls to Allow() are ignored
TEST_F(CertificateErrorTest, IsCancelled) {
  std::unique_ptr<CertificateError> error = CreateError();

  bool cancel_callback_ran = false;
  error->SetCancelCallback(base::Bind(&CancelCallback, &cancel_callback_ran));

  EXPECT_TRUE(error->is_main_frame());
  EXPECT_FALSE(error->is_subresource());
  EXPECT_EQ(CERT_ERROR_BAD_IDENTITY, error->cert_error());
  EXPECT_EQ(cert(), error->cert());
  EXPECT_EQ(GURL("https://www.google.com/"), error->url());
  EXPECT_TRUE(error->overridable());
  EXPECT_FALSE(error->strict_enforcement());
  EXPECT_FALSE(error->IsCancelled());

  error->SimulateCancel();

  EXPECT_TRUE(error->IsCancelled());
  EXPECT_TRUE(cancel_callback_ran);
  EXPECT_FALSE(last_response());
  ASSERT_EQ(1, responded_count());

  error->Allow();
  ASSERT_EQ(1, responded_count());

  error.reset();
  ASSERT_EQ(1, responded_count());
}

// Test that calling Allow() works, and that the callback is only called once
TEST_F(CertificateErrorTest, Allow) {
  std::unique_ptr<CertificateError> error = CreateError();

  error->Allow();
  EXPECT_TRUE(last_response());
  ASSERT_EQ(1, responded_count());

  error->Allow();
  ASSERT_EQ(1, responded_count());

  error->Deny();
  ASSERT_EQ(1, responded_count());

  error.reset();
  ASSERT_EQ(1, responded_count());
}

// Test that calling Deny() works, and that the callback is only called once
TEST_F(CertificateErrorTest, Deny) {
  std::unique_ptr<CertificateError> error = CreateError();

  error->Deny();
  EXPECT_FALSE(last_response());
  ASSERT_EQ(1, responded_count());

  error->Deny();
  ASSERT_EQ(1, responded_count());

  error->Allow();
  ASSERT_EQ(1, responded_count());

  error.reset();
  ASSERT_EQ(1, responded_count());
}

// Test that we get a denied response if CertificateError is deleted without
// calling Allow() or Deny()
TEST_F(CertificateErrorTest, DeleteWithoutResponse) {
  std::unique_ptr<CertificateError> error = CreateError();
  error.reset();

  EXPECT_EQ(1, responded_count());
  EXPECT_FALSE(last_response());
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

  void ResetChangedFlags();

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

void TestWebContentsDelegate::ResetChangedFlags() {
  changed_flags_ = static_cast<content::InvalidateTypes>(0);
}

void DummyResponse(bool response) {}

}

// Test that calling Deny() dismisses the placeholder interstitial
// XXX: Not sure how we unit-test that we call InterstitialPage::Proceed on
//  Allow(). This produces no observable effect because it doesn't drop the
//  transient entry
TEST_F(CertificateErrorTest, DenyDismissesPlaceholder) {
  CertificateErrorPlaceholderPage::SetDontCreateViewForTesting(true);
  TestBrowserThreadBundle browser_thread_bundle;
  content::TestBrowserContext browser_context;
  content::TestWebContentsFactory web_contents_factory;

  std::unique_ptr<TestWebContentsDelegate> delegate =
      base::WrapUnique(new TestWebContentsDelegate());
  content::WebContents* contents =
      web_contents_factory.CreateWebContents(&browser_context);
  contents->SetDelegate(delegate.get());

  std::unique_ptr<CertificateError> error =
      CertificateError::CreateForTestingWithPlaceholder(
          contents,
          true,
          false,
          CERT_ERROR_BAD_IDENTITY,
          cert(),
          GURL("https://www.google.com/"),
          false,
          true,
          base::Bind(&DummyResponse));

  EXPECT_EQ(content::INVALIDATE_TYPE_ALL, delegate->changed_flags());
  EXPECT_EQ(GURL("https://www.google.com/"), contents->GetVisibleURL());
  EXPECT_NE(nullptr, contents->GetController().GetTransientEntry());

  delegate->ResetChangedFlags();
  error->Deny();

  EXPECT_EQ(content::INVALIDATE_TYPE_ALL, delegate->changed_flags());
  EXPECT_NE(GURL("https://www.google.com/"), contents->GetVisibleURL());
  EXPECT_EQ(nullptr, contents->GetController().GetTransientEntry());

  contents->SetDelegate(nullptr);
  CertificateErrorPlaceholderPage::SetDontCreateViewForTesting(false);
}

} // namespace oxide
