// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QVariant>

class OxideQSslCertificateData;

class Q_DECL_EXPORT OxideQSslCertificate {
  Q_GADGET

  Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
  Q_PROPERTY(QString subjectDisplayName READ subjectDisplayName CONSTANT)
  Q_PROPERTY(QString issuerDisplayName READ issuerDisplayName CONSTANT)
  Q_PROPERTY(QDateTime effectiveDate READ effectiveDate CONSTANT)
  Q_PROPERTY(QDateTime expiryDate READ expiryDate CONSTANT)
  Q_PROPERTY(QString fingerprintSHA1 READ fingerprintSHA1 CONSTANT)

  Q_PROPERTY(bool isExpired READ isExpired CONSTANT)

  Q_PROPERTY(QVariant issuer READ issuer CONSTANT)

  Q_ENUMS(PrincipalAttr)

 public:

  enum PrincipalAttr {
    PrincipalAttrOrganizationName,
    PrincipalAttrCommonName,
    PrincipalAttrLocalityName,
    PrincipalAttrOrganizationUnitName,
    PrincipalAttrCountryName,
    PrincipalAttrStateOrProvinceName
  };

  OxideQSslCertificate();
  ~OxideQSslCertificate();

  OxideQSslCertificate(const OxideQSslCertificate& other);
  OxideQSslCertificate operator=(const OxideQSslCertificate& other);

  bool operator==(const OxideQSslCertificate& other) const;
  bool operator!=(const OxideQSslCertificate& other) const;

  QString serialNumber() const;

  QString subjectDisplayName() const;
  QString issuerDisplayName() const;

  Q_INVOKABLE QStringList getSubjectInfo(PrincipalAttr attr) const;
  Q_INVOKABLE QStringList getIssuerInfo(PrincipalAttr attr) const;

  QDateTime effectiveDate() const;
  QDateTime expiryDate() const;

  QString fingerprintSHA1() const;

  bool isExpired() const;

  QVariant issuer() const;
  Q_INVOKABLE OxideQSslCertificate copy() const;

  Q_INVOKABLE QString toPem() const;

  bool isValid() const;

 private:
  friend class OxideQSslCertificateData;
  OxideQSslCertificate(const QSharedDataPointer<OxideQSslCertificateData>& dd);

  QSharedDataPointer<OxideQSslCertificateData> d;
};

Q_DECLARE_METATYPE(OxideQSslCertificate)

#endif // OXIDE_Q_SSL_CERTIFICATE
