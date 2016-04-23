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

struct Params {
  bool is_main_frame = true;
  bool is_subresource = false;
  oxide::CertError cert_error = oxide::CERT_ERROR_BAD_IDENTITY;
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

  std::unique_ptr<oxide::CertificateError> CreateError(
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

std::unique_ptr<oxide::CertificateError> CertificateErrorTest::CreateError(
    const Params& params) {
  return oxide::CertificateError::CreateForTesting(
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

// Test that is_main_frame is set correctly
TEST_F(CertificateErrorTest, is_main_frame) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  EXPECT_TRUE(error->is_main_frame());

  Params params;
  params.is_main_frame = false;

  error = CreateError(params);
  EXPECT_FALSE(error->is_main_frame());
}

// Test that is_subresource is set correctly
TEST_F(CertificateErrorTest, is_subresource) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  EXPECT_FALSE(error->is_subresource());

  Params params;
  params.is_subresource = true;

  error = CreateError(params);
  EXPECT_TRUE(error->is_subresource());
}

// Test that cert_error is set correctly
TEST_F(CertificateErrorTest, cert_error) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  EXPECT_EQ(error->cert_error(), oxide::CERT_ERROR_BAD_IDENTITY);

  Params params;
  params.cert_error = oxide::CERT_ERROR_GENERIC;

  error = CreateError(params);
  EXPECT_EQ(error->cert_error(), oxide::CERT_ERROR_GENERIC);
}

// Test that cert is set correctly
TEST_F(CertificateErrorTest, cert) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  EXPECT_EQ(error->cert(), cert());
}

// Test that url is set correctly
TEST_F(CertificateErrorTest, url) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  EXPECT_EQ(error->url(), GURL("https://www.google.com/"));
}

// Test that overridable is set correctly and that CertificateError ignores
// calls to Allow()
TEST_F(CertificateErrorTest, overridable) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  EXPECT_TRUE(error->overridable());

  Params params;
  params.overridable = false;

  error = CreateError(params);
  EXPECT_FALSE(error->overridable());
  ASSERT_EQ(responded_count(), 1);

  error->Allow();
  ASSERT_EQ(responded_count(), 1);

  error.reset();
  ASSERT_EQ(responded_count(), 1);
}

// Test that strict_enforcement is set correctly
TEST_F(CertificateErrorTest, strict_enforcement) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  EXPECT_FALSE(error->strict_enforcement());

  Params params;
  params.strict_enforcement = true;

  error = CreateError(params);
  EXPECT_TRUE(error->strict_enforcement());
}

namespace {

void CancelCallback(bool* cancelled) {
  *cancelled = true;
}

}

// Test that cancellation works and that calls to Allow() are ignored
TEST_F(CertificateErrorTest, IsCancelled) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();

  bool cancel_callback_ran = false;
  error->SetCancelCallback(base::Bind(&CancelCallback, &cancel_callback_ran));

  EXPECT_FALSE(error->IsCancelled());

  error->SimulateCancel();

  EXPECT_TRUE(error->IsCancelled());
  EXPECT_TRUE(cancel_callback_ran);
  EXPECT_FALSE(last_response());
  ASSERT_EQ(responded_count(), 1);

  error->Allow();
  ASSERT_EQ(responded_count(), 1);

  error.reset();
  ASSERT_EQ(responded_count(), 1);
}

// Test that calling Allow() works, and that the callback is only called once
TEST_F(CertificateErrorTest, Allow) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();

  error->Allow();
  EXPECT_TRUE(last_response());
  ASSERT_EQ(responded_count(), 1);

  error->Allow();
  ASSERT_EQ(responded_count(), 1);

  error->Deny();
  ASSERT_EQ(responded_count(), 1);

  error.reset();
  ASSERT_EQ(responded_count(), 1);
}

// Test that calling Deny() works, and that the callback is only called once
TEST_F(CertificateErrorTest, Deny) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();

  error->Deny();
  EXPECT_FALSE(last_response());
  ASSERT_EQ(responded_count(), 1);

  error->Deny();
  ASSERT_EQ(responded_count(), 1);

  error->Allow();
  ASSERT_EQ(responded_count(), 1);

  error.reset();
  ASSERT_EQ(responded_count(), 1);
}

// Test that we get a denied response if CertificateError is deleted without
// calling Allow() or Deny()
TEST_F(CertificateErrorTest, DeleteWithoutResponse) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  error.reset();

  EXPECT_EQ(responded_count(), 1);
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
  oxide::CertificateErrorPlaceholderPage::SetDontCreateViewForTesting(true);
  oxide::TestBrowserThreadBundle browser_thread_bundle;
  content::TestBrowserContext browser_context;
  content::TestWebContentsFactory web_contents_factory;

  std::unique_ptr<TestWebContentsDelegate> delegate =
      base::WrapUnique(new TestWebContentsDelegate());
  content::WebContents* contents =
      web_contents_factory.CreateWebContents(&browser_context);
  contents->SetDelegate(delegate.get());

  std::unique_ptr<oxide::CertificateError> error =
      oxide::CertificateError::CreateForTestingWithPlaceholder(
          contents,
          true,
          false,
          oxide::CERT_ERROR_BAD_IDENTITY,
          cert(),
          GURL("https://www.google.com/"),
          false,
          true,
          base::Bind(&DummyResponse));

  EXPECT_EQ(delegate->changed_flags(), content::INVALIDATE_TYPE_ALL);
  EXPECT_EQ(contents->GetVisibleURL(), GURL("https://www.google.com/"));
  EXPECT_NE(contents->GetController().GetTransientEntry(), nullptr);

  delegate->ResetChangedFlags();
  error->Deny();

  EXPECT_EQ(delegate->changed_flags(), content::INVALIDATE_TYPE_ALL);
  EXPECT_NE(contents->GetVisibleURL(), GURL("https://www.google.com/"));
  EXPECT_EQ(contents->GetController().GetTransientEntry(), nullptr);

  contents->SetDelegate(nullptr);
  oxide::CertificateErrorPlaceholderPage::SetDontCreateViewForTesting(false);
}
