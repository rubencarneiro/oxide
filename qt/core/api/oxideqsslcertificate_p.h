// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#include <memory>
#include <QSharedData>
#include <QtGlobal>

#include "qt/core/api/oxideqglobal.h"

namespace net {
class X509Certificate;
}

class OxideQSslCertificate;

class OXIDE_QTCORE_EXPORT OxideQSslCertificateData : public QSharedData {
  Q_DISABLE_COPY(OxideQSslCertificateData)

 public:
  ~OxideQSslCertificateData();

  static net::X509Certificate* GetX509Certificate(
      const OxideQSslCertificate& cert);

  static OxideQSslCertificate Create(net::X509Certificate* cert);

  static OxideQSslCertificate CreateForTesting(const QString& subject,
                                               const QString& issuer,
                                               const QDateTime& effective_date,
                                               const QDateTime& expiry_date);

 private:
  friend class OxideQSslCertificate;

  OxideQSslCertificateData();
  OxideQSslCertificateData(net::X509Certificate* cert);

  net::X509Certificate* x509_cert_; // strong ref, but can't use scoped_refptr
  mutable std::unique_ptr<OxideQSslCertificate> issuer_;
};

#endif // _OXIDE_QT_CORE_API_SSL_CERTIFICATE_P_H_
