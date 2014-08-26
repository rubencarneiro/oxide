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

#ifndef OXIDE_Q_SSL_CERTIFICATE
#define OXIDE_Q_SSL_CERTIFICATE

#include <QDateTime>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QtGlobal>

class OxideQSslCertificatePrivate;

class Q_DECL_EXPORT OxideQSslCertificate Q_DECL_FINAL : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
  Q_PROPERTY(QString subjectDisplayName READ subjectDisplayName CONSTANT)
  Q_PROPERTY(QString issuerDisplayName READ issuerDisplayName CONSTANT)
  Q_PROPERTY(QDateTime effectiveDate READ effectiveDate CONSTANT)
  Q_PROPERTY(QDateTime expiryDate READ expiryDate CONSTANT)
  Q_PROPERTY(QString fingerprintSHA1 READ fingerprintSHA1 CONSTANT)

  Q_PROPERTY(bool isExpired READ isExpired CONSTANT)

  Q_PROPERTY(OxideQSslCertificate* issuer READ issuer CONSTANT)

  Q_ENUMS(PrincipalAttr)

  Q_DECLARE_PRIVATE(OxideQSslCertificate)
  Q_DISABLE_COPY(OxideQSslCertificate)

 public:

  enum PrincipalAttr {
    PrincipalAttrOrganizationName,
    PrincipalAttrCommonName,
    PrincipalAttrLocalityName,
    PrincipalAttrOrganizationUnitName,
    PrincipalAttrCountryName,
    PrincipalAttrStateOrProvinceName
  };

  ~OxideQSslCertificate();

  QString serialNumber() const;

  QString subjectDisplayName() const;
  QString issuerDisplayName() const;

  Q_INVOKABLE QStringList getSubjectInfo(PrincipalAttr attr) const;
  Q_INVOKABLE QStringList getIssuerInfo(PrincipalAttr attr) const;

  QDateTime effectiveDate() const;
  QDateTime expiryDate() const;

  QString fingerprintSHA1() const;

  bool isExpired() const;

  OxideQSslCertificate* issuer() const;

  Q_INVOKABLE QString toPem() const;

 private:
  OxideQSslCertificate(QObject* parent = NULL);

  QScopedPointer<OxideQSslCertificatePrivate> d_ptr;
};

#endif // OXIDE_Q_SSL_CERTIFICATE
