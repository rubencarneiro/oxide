// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_API_SSL_CERTIFICATE_P_H_
#define _OXIDE_QT_CORE_API_SSL_CERTIFICATE_P_H_

#include <QtGlobal>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

class OxideQSslCertificate;

namespace net {
class X509Certificate;
}

class OxideQSslCertificatePrivate Q_DECL_FINAL {
 public:
  ~OxideQSslCertificatePrivate();

  static OxideQSslCertificate* Create(
      const scoped_refptr<net::X509Certificate>& cert,
      QObject* parent = NULL);

 private:
  friend class OxideQSslCertificate;

  OxideQSslCertificatePrivate(const scoped_refptr<net::X509Certificate>& cert);

  scoped_refptr<net::X509Certificate> x509_cert_;
  mutable scoped_ptr<OxideQSslCertificate> issuer_;
};

#endif // _OXIDE_QT_CORE_API_SSL_CERTIFICATE_P_H_
