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

#ifndef _OXIDE_SHARED_TEST_MOCK_CERT_STORE_H_
#define _OXIDE_SHARED_TEST_MOCK_CERT_STORE_H_

#include <map>

#include "base/macros.h"
#include "content/public/browser/cert_store.h"

namespace net {
class X509Certificate;
}

namespace oxide {

class MockCertStore : public content::CertStore {
 public:
  MockCertStore();
  ~MockCertStore() override;

  int AddCertForTesting(net::X509Certificate* cert);

  // content::CertStore implementation
  int StoreCert(net::X509Certificate* cert,
                int render_process_host_id) override;
  bool RetrieveCert(int cert_id,
                    scoped_refptr<net::X509Certificate>* cert) override;

 private:
  int next_id_;
  std::map<int, scoped_refptr<net::X509Certificate>> id_to_cert_;

  DISALLOW_COPY_AND_ASSIGN(MockCertStore);
};

} // namespace oxide

#endif // _OXIDE_SHARED_TEST_MOCK_CERT_STORE_H_
