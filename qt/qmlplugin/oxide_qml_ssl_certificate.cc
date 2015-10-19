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

#include "oxide_qml_ssl_certificate.h"

#include <QJSEngine>
#include <QMetaType>
#include <QQmlEngine>
#include <QtQml/private/qqmldata_p.h>
#include <QtQml/private/qv4engine_p.h>
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
#include <QtQml/private/qv4value_p.h>
#else
#include <QtQml/private/qv4persistent_p.h>
#endif
#include <QtQml/private/qv8engine_p.h>

namespace oxide {
namespace qmlplugin {

SslCertificate::SslCertificate(QObject* parent)
    : QQmlValueTypeBase<OxideQSslCertificate>(
        qMetaTypeId<OxideQSslCertificate>(),
        parent) {}

SslCertificate::~SslCertificate() {}

QString SslCertificate::serialNumber() const {
  return v.serialNumber();
}

QString SslCertificate::subjectDisplayName() const {
  return v.subjectDisplayName();
}

QString SslCertificate::issuerDisplayName() const {
  return v.issuerDisplayName();
}

QStringList SslCertificate::getSubjectInfo(PrincipalAttr attr) const {
  return v.getSubjectInfo(
      static_cast<OxideQSslCertificate::PrincipalAttr>(attr));
}

QStringList SslCertificate::getIssuerInfo(PrincipalAttr attr) const {
  return v.getIssuerInfo(
      static_cast<OxideQSslCertificate::PrincipalAttr>(attr));
}

QDateTime SslCertificate::effectiveDate() const {
  return v.effectiveDate();
}

QDateTime SslCertificate::expiryDate() const {
  return v.expiryDate();
}

QString SslCertificate::fingerprintSHA1() const {
  return v.fingerprintSHA1();
}

bool SslCertificate::isExpired() const {
  return v.isExpired();
}

QJSValue SslCertificate::issuer() const {
  // If we just return the variant here, the QML engine doesn't convert it
  // to a SslCertificate. See https://bugreports.qt.io/browse/QTBUG-47940
  QVariant i = v.issuer();
  if (i.isNull()) {
    return QJSValue(QJSValue::NullValue);
  }

  SslCertificate* rv = new SslCertificate();
  rv->setValue(i);

  QQmlData *data = QQmlData::get(this, false);
  if (!data || data->jsWrapper.isNullOrUndefined()) {
    return QJSValue(QJSValue::NullValue);
  }

  return data->jsWrapper.engine()->v8Engine->publicEngine()->newQObject(rv);
}

OxideQSslCertificate SslCertificate::copy() const {
  return v.copy();
}

QString SslCertificate::toPem() const {
  return v.toPem();
}

QString SslCertificate::toString() const {
  return QString();
}

bool SslCertificate::isEqual(const QVariant& other) const {
  if (other.userType() != qMetaTypeId<OxideQSslCertificate>()) {
    return false;
  }

  return v == other.value<OxideQSslCertificate>();
}

} // namespace qmlplugin
} // namespace oxide
