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

#include "oxide_mock_cert_store.h"

#include "base/memory/ref_counted.h"
#include "net/cert/x509_certificate.h"

namespace oxide {

MockCertStore::MockCertStore()
    : next_id_(1) {}

MockCertStore::~MockCertStore() {}

int MockCertStore::AddCertForTesting(net::X509Certificate* cert) {
  int id = next_id_++;
  id_to_cert_[id] = cert;
  return id;
}

int MockCertStore::StoreCert(net::X509Certificate* cert,
                             int render_process_host_id) {
  NOTREACHED();
  return 0;
}

bool MockCertStore::RetrieveCert(int cert_id,
                                 scoped_refptr<net::X509Certificate>* cert) {
  if (id_to_cert_.find(cert_id) == id_to_cert_.end()) {
    return false;
  }

  *cert = id_to_cert_[cert_id];
  return true;
}

} // namespace oxide
