// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef _OXIDE_QMLPLUGIN_SSL_CERTIFICATE_H_
#define _OXIDE_QMLPLUGIN_SSL_CERTIFICATE_H_

#include <QJSValue>
#include <QtGlobal>
#include <QtQml/private/qqmlvaluetype_p.h>

#include "qt/core/api/oxideqsslcertificate.h"

namespace oxide {
namespace qmlplugin {

class SslCertificate : public QQmlValueTypeBase<OxideQSslCertificate> {
  Q_OBJECT

  Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
  Q_PROPERTY(QString subjectDisplayName READ subjectDisplayName CONSTANT)
  Q_PROPERTY(QString issuerDisplayName READ issuerDisplayName CONSTANT)
  Q_PROPERTY(QDateTime effectiveDate READ effectiveDate CONSTANT)
  Q_PROPERTY(QDateTime expiryDate READ expiryDate CONSTANT)
  Q_PROPERTY(QString fingerprintSHA1 READ fingerprintSHA1 CONSTANT)

  Q_PROPERTY(bool isExpired READ isExpired CONSTANT)

  Q_PROPERTY(QJSValue issuer READ issuer CONSTANT)

  Q_ENUMS(PrincipalAttr)

  Q_DISABLE_COPY(SslCertificate)

 public:

  enum PrincipalAttr {
    PrincipalAttrOrganizationName =
        OxideQSslCertificate::PrincipalAttrOrganizationName,
    PrincipalAttrCommonName = OxideQSslCertificate::PrincipalAttrCommonName,
    PrincipalAttrLocalityName =
        OxideQSslCertificate::PrincipalAttrLocalityName,
    PrincipalAttrOrganizationUnitName =
        OxideQSslCertificate::PrincipalAttrOrganizationUnitName,
    PrincipalAttrCountryName = OxideQSslCertificate::PrincipalAttrCountryName,
    PrincipalAttrStateOrProvinceName =
        OxideQSslCertificate::PrincipalAttrStateOrProvinceName
  };

  SslCertificate(QObject* parent = nullptr);
  ~SslCertificate() override;

  QString serialNumber() const;

  QString subjectDisplayName() const;
  QString issuerDisplayName() const;

  Q_INVOKABLE QStringList getSubjectInfo(PrincipalAttr attr) const;
  Q_INVOKABLE QStringList getIssuerInfo(PrincipalAttr attr) const;

  QDateTime effectiveDate() const;
  QDateTime expiryDate() const;

  QString fingerprintSHA1() const;

  bool isExpired() const;

  QJSValue issuer() const;
  Q_INVOKABLE OxideQSslCertificate copy() const;

  Q_INVOKABLE QString toPem() const;

  // QQmlValueType implementation
  QString toString() const override;
  bool isEqual(const QVariant& other) const override;
};

} // namespace qmlplugin
} // namespace oxide

Q_DECLARE_METATYPE(oxide::qmlplugin::SslCertificate*)

#endif // _OXIDE_QMLPLUGIN_SSL_CERTIFICATE_H_
