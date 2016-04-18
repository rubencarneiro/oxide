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

#include "testing/gtest/include/gtest/gtest.h"

#include <memory>

#include "base/bind.h"
#include "base/time/time.h"
#include "net/cert/x509_certificate.h"
#include "url/gurl.h"

#include "shared/browser/oxide_security_types.h"

#include "oxide_certificate_error.h"

struct Params {
  bool is_main_frame = true;
  bool is_subresource = false;
  oxide::CertError cert_error = oxide::CERT_ERROR_BAD_IDENTITY;
  bool overridable = true;
  bool strict_enforcement = false;
};

class CertificateErrorTest : public testing::Test {
 public:
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

  void ResetCount() { responded_count_ = 0; }

 private:
  void SetUp() override;

  void OnResponse(bool response);

  scoped_refptr<net::X509Certificate> cert_;

  int responded_count_;
  int last_response_;
};

void CertificateErrorTest::SetUp() {
  responded_count_ = 0;
  last_response_ = false;
}

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

TEST_F(CertificateErrorTest, is_main_frame) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  ASSERT_TRUE(error->is_main_frame());

  Params params;
  params.is_main_frame = false;

  error = CreateError(params);
  ASSERT_FALSE(error->is_main_frame());
}

TEST_F(CertificateErrorTest, is_subresource) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  ASSERT_FALSE(error->is_subresource());

  Params params;
  params.is_subresource = true;

  error = CreateError(params);
  ASSERT_TRUE(error->is_subresource());
}

TEST_F(CertificateErrorTest, cert_error) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  ASSERT_EQ(error->cert_error(), oxide::CERT_ERROR_BAD_IDENTITY);

  Params params;
  params.cert_error = oxide::CERT_ERROR_GENERIC;

  error = CreateError(params);
  ASSERT_EQ(error->cert_error(), oxide::CERT_ERROR_GENERIC);
}

TEST_F(CertificateErrorTest, cert) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  ASSERT_EQ(error->cert(), cert());
}

TEST_F(CertificateErrorTest, url) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  ASSERT_EQ(error->url(), GURL("https://www.google.com/"));
}

TEST_F(CertificateErrorTest, overridable) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  ASSERT_TRUE(error->overridable());

  Params params;
  params.overridable = false;

  error = CreateError(params);
  ResetCount();

  ASSERT_FALSE(error->overridable());

  error->Allow();
  ASSERT_EQ(responded_count(), 0);

  error.reset();
  ASSERT_EQ(responded_count(), 0);
}

TEST_F(CertificateErrorTest, strict_enforcement) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  ASSERT_FALSE(error->strict_enforcement());

  Params params;
  params.strict_enforcement = true;

  error = CreateError(params);
  ASSERT_TRUE(error->strict_enforcement());
}

void CancelCallback(bool* cancelled) {
  *cancelled = true;
}

TEST_F(CertificateErrorTest, IsCancelled) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();

  bool cancel_callback_ran = false;
  error->SetCancelCallback(base::Bind(&CancelCallback, &cancel_callback_ran));

  ASSERT_FALSE(error->IsCancelled());

  error->SimulateCancel();

  ASSERT_TRUE(error->IsCancelled());
  ASSERT_TRUE(cancel_callback_ran);
  ASSERT_EQ(responded_count(), 1);
  ASSERT_FALSE(last_response());

  error->Allow();
  ASSERT_EQ(responded_count(), 1);

  error.reset();
  ASSERT_EQ(responded_count(), 1);
}

TEST_F(CertificateErrorTest, Allow) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();

  error->Allow();
  ASSERT_EQ(responded_count(), 1);
  ASSERT_TRUE(last_response());

  error->Allow();
  ASSERT_EQ(responded_count(), 1);

  error->Deny();
  ASSERT_EQ(responded_count(), 1);

  error.reset();
  ASSERT_EQ(responded_count(), 1);
}

TEST_F(CertificateErrorTest, Deny) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();

  error->Deny();
  ASSERT_EQ(responded_count(), 1);
  ASSERT_FALSE(last_response());

  error->Deny();
  ASSERT_EQ(responded_count(), 1);

  error->Allow();
  ASSERT_EQ(responded_count(), 1);

  error.reset();
  ASSERT_EQ(responded_count(), 1);
}

TEST_F(CertificateErrorTest, DeleteWithResponse) {
  std::unique_ptr<oxide::CertificateError> error = CreateError();
  error.reset();

  ASSERT_EQ(responded_count(), 1);
  ASSERT_FALSE(last_response());
}
