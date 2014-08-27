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

#ifndef OXIDE_Q_SECURITY_EVENTS
#define OXIDE_Q_SECURITY_EVENTS

#include <QObject>
#include <QtGlobal>
#include <QUrl>

class OxideQCertificateErrorPrivate;
class OxideQSslCertificate;

class Q_DECL_EXPORT OxideQCertificateError Q_DECL_FINAL : public QObject {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url CONSTANT)

  Q_PROPERTY(bool isMainFrame READ isMainFrame CONSTANT)
  Q_PROPERTY(bool isSubresource READ isSubresource CONSTANT)
  Q_PROPERTY(bool overridable READ overridable CONSTANT)
  Q_PROPERTY(bool strictEnforcement READ strictEnforcement CONSTANT)

  Q_PROPERTY(OxideQSslCertificate* certificate READ certificate CONSTANT)
  Q_PROPERTY(CertError certError READ certError CONSTANT)

  Q_ENUMS(CertError)

  Q_DECLARE_PRIVATE(OxideQCertificateError)
  Q_DISABLE_COPY(OxideQCertificateError)

 public:
  enum CertError {
    CertOK,
    CertErrorBadIdentity,
    CertErrorExpired,
    CertErrorDateInvalid,
    CertErrorAuthorityInvalid,
    CertErrorRevoked,
    CertErrorInvalid,
    CertErrorInsecure,
    CertErrorGeneric
  };

  ~OxideQCertificateError();

  QUrl url() const;

  bool isMainFrame() const;
  bool isSubresource() const;

  bool overridable() const;
  bool strictEnforcement() const;

  OxideQSslCertificate* certificate() const;
  CertError certError() const;

  Q_INVOKABLE void allow();
  Q_INVOKABLE void deny();

 private:
  OxideQCertificateError(OxideQCertificateErrorPrivate& dd,
                         QObject* parent = NULL);

  QScopedPointer<OxideQCertificateErrorPrivate> d_ptr;
};

#endif // OXIDE_Q_SECURITY_EVENTS
