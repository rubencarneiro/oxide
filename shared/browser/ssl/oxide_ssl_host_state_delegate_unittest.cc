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
#include <string>

#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "content/public/browser/ssl_host_state_delegate.h"
#include "net/cert/cert_status_flags.h"
#include "net/cert/x509_certificate.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "oxide_ssl_host_state_delegate.h"

namespace oxide {

class SSLHostStateDelegateTest : public testing::Test {
 public:
  SSLHostStateDelegateTest();

  SSLHostStateDelegate* ssl_host_state_delegate() const {
    return ssl_host_state_delegate_.get();
  }

 private:
  void SetUp() override;

  std::unique_ptr<SSLHostStateDelegate> ssl_host_state_delegate_;
};

void SSLHostStateDelegateTest::SetUp() {
  ssl_host_state_delegate_ = base::WrapUnique(new SSLHostStateDelegate());
}

SSLHostStateDelegateTest::SSLHostStateDelegateTest() {}

TEST_F(SSLHostStateDelegateTest, QueryPolicy) {
  scoped_refptr<net::X509Certificate> cert(
      new net::X509Certificate("https://www.example.com/",
                               "https://www.foo.com/",
                               base::Time(), base::Time()));
  EXPECT_EQ(content::SSLHostStateDelegate::DENIED,
            ssl_host_state_delegate()->QueryPolicy(
                "www.google.com",
                *cert.get(),
                net::CERT_STATUS_COMMON_NAME_INVALID,
                nullptr));
}

TEST_F(SSLHostStateDelegateTest, HostRanInsecureContent) {
  ssl_host_state_delegate()->HostRanInsecureContent("www.google.com", 1);
  ssl_host_state_delegate()->HostRanInsecureContent("www.example.co.uk", 3);

  EXPECT_TRUE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.google.com",
                                                           1));
  EXPECT_FALSE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.google.com",
                                                           2));
  EXPECT_FALSE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.google.com",
                                                           3));

  EXPECT_FALSE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.example.co.uk",
                                                           1));
  EXPECT_FALSE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.example.co.uk",
                                                           2));
  EXPECT_TRUE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.example.co.uk",
                                                           3));
  
  EXPECT_FALSE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.google.co.uk",
                                                           1));
  EXPECT_FALSE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.google.co.uk",
                                                           2));
  EXPECT_FALSE(
      ssl_host_state_delegate()->DidHostRunInsecureContent("www.google.co.uk",
                                                           3));
}

TEST_F(SSLHostStateDelegateTest, HasAllowException) {
  EXPECT_FALSE(ssl_host_state_delegate()->HasAllowException("www.google.com"));
}

} // namespace oxide
