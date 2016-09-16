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

#include "oxideqsslcertificate.h"
#include "oxideqsslcertificate_p.h"

#include <string>
#include <vector>
#include <QByteArray>
#include <QCryptographicHash>
#include <QSsl>
#include <QSslCertificate>

#include "base/logging.h"
#include "base/time/time.h"
#include "net/base/hash_value.h"
#include "net/cert/x509_certificate.h"
#include "net/cert/x509_cert_types.h"

namespace {

QStringList GetPrincipalValue(const net::CertPrincipal& principal,
                              OxideQSslCertificate::PrincipalAttr attr) {
  switch (attr) {
    case OxideQSslCertificate::PrincipalAttrOrganizationName: {
      QStringList rv;
      for (size_t i = 0; i < principal.organization_names.size(); ++i) {
        rv.push_back(QString::fromStdString(principal.organization_names[i]));
      }
      return rv;
    }
    case OxideQSslCertificate::PrincipalAttrCommonName:
      return QStringList(QString::fromStdString(principal.common_name));
    case OxideQSslCertificate::PrincipalAttrLocalityName:
      return QStringList(QString::fromStdString(principal.locality_name));
    case OxideQSslCertificate::PrincipalAttrOrganizationUnitName: {
      QStringList rv;
      for (size_t i = 0; i < principal.organization_unit_names.size(); ++i) {
        rv.push_back(QString::fromStdString(principal.organization_unit_names[i]));
      }
      return rv;
    }
    case OxideQSslCertificate::PrincipalAttrCountryName:
      return QStringList(QString::fromStdString(principal.country_name));
    case OxideQSslCertificate::PrincipalAttrStateOrProvinceName:
      return QStringList(QString::fromStdString(principal.state_or_province_name));
    default:
      NOTREACHED();
      return QStringList();
  }
}

QDateTime ToQDateTime(const base::Time& time) {
  int64_t ms = (time - base::Time::UnixEpoch()).InMilliseconds();
  return QDateTime::fromMSecsSinceEpoch(ms);
}

base::Time ToChromiumTime(const QDateTime& time) {
  return base::Time::UnixEpoch() +
         base::TimeDelta::FromMilliseconds(time.toMSecsSinceEpoch());
}

}

OxideQSslCertificateData::OxideQSslCertificateData()
    : x509_cert_(nullptr) {}

OxideQSslCertificateData::OxideQSslCertificateData(net::X509Certificate* cert)
    : x509_cert_(cert) {
  if (cert) {
    cert->AddRef();
  }
}

OxideQSslCertificateData::~OxideQSslCertificateData() {
  if (x509_cert_) {
    x509_cert_->Release();
  }
}

// static
net::X509Certificate* OxideQSslCertificateData::GetX509Certificate(
    const OxideQSslCertificate& cert) {
  if (!cert.isValid()) {
    return nullptr;
  }

  return cert.d->x509_cert_;
}

// static
OxideQSslCertificate OxideQSslCertificateData::Create(
    net::X509Certificate* cert) {
  QSharedDataPointer<OxideQSslCertificateData> data(
      new OxideQSslCertificateData(cert));
  return OxideQSslCertificate(data);
}

// static
OxideQSslCertificate OxideQSslCertificateData::CreateForTesting(
    const QString& subject,
    const QString& issuer,
    const QDateTime& effective_date,
    const QDateTime& expiry_date) {
  scoped_refptr<net::X509Certificate> x509_cert(
      new net::X509Certificate(subject.toStdString(),
                               issuer.toStdString(),
                               ToChromiumTime(effective_date),
                               ToChromiumTime(expiry_date)));
  return Create(x509_cert.get());
}

/*!
\class OxideQSslCertificate
\inheaderfile oxideqsslcertificate.h
\inmodule OxideQtCore

\brief X.509 certificate

OxideQSslCertificate represents a X.509 certificate provided by a remote server.

Information about the subject of the certificate can be discovered by calling
getSubjectInfo. Information about the issuer of the certificate (the entity
that signed it) can be discovered by calling getIssuerInfo. For self-signed
certificates, this will return the same as getSubjectInfo.

For certificates that aren't self-signed, the issuer's certificate can be
accessed via \l{issuer}.
*/

/*!
\enum OxideQSslCertificate::PrincipalAttr

This represents an issuer or subject field, which you can pass to
getSubjectInfo or getIssuerInfo

\value PrincipalAttrOrganizationName
'O' - The name of the organization.

\value PrincipalAttrCommonName
'CN' - The Common Name. Generally used to store the host name.

\value PrincipalAttrLocalityName
'L' - The locality.

\value PrincipalAttrOrganizationUnitName
'OU' - The organizational unit name.

\value PrincipalAttrCountryName
'C' - The country name.

\value PrincipalAttrStateOrProvinceName
'ST' - The state or province name.
*/

OxideQSslCertificate::OxideQSslCertificate(
    const QSharedDataPointer<OxideQSslCertificateData>& dd)
    : d(dd) {}

/*!
\internal
*/

OxideQSslCertificate::OxideQSslCertificate()
    : d(new OxideQSslCertificateData()) {}

/*!
Destroys this certificate.
*/

OxideQSslCertificate::~OxideQSslCertificate() {}

/*!
Copy constructs a certificate from \a other.
*/

OxideQSslCertificate::OxideQSslCertificate(const OxideQSslCertificate& other)
    : d(other.d) {}

/*!
Assigns \a other to this certificate.
*/

OxideQSslCertificate OxideQSslCertificate::operator=(
    const OxideQSslCertificate& other) {
  d = other.d;
  return *this;
}

/*!
Returns true if this certificate equals \a other. A certificate will only be
equal to one that it was copied from.
*/

bool OxideQSslCertificate::operator==(
    const OxideQSslCertificate& other) const {
  return d == other.d;
}

/*!
Returns true if this certificate does not equal \a other.
*/

bool OxideQSslCertificate::operator!=(
    const OxideQSslCertificate& other) const {
  return !(*this == other);
}

/*!
Returns the serial number of this certificate as a hex encoded string.

\sa QByteArray::toHex
*/

QString OxideQSslCertificate::serialNumber() const {
  if (!isValid()) {
    return QString();
  }

  const std::string& serial_number = d->x509_cert_->serial_number();
  QByteArray ba(serial_number.data(), int(serial_number.size()));

  return QString::fromUtf8(ba.toHex());
}

/*!
Returns the display name of the subject of this certificate.
*/

QString OxideQSslCertificate::subjectDisplayName() const {
  if (!isValid()) {
    return QString();
  }

  return QString::fromStdString(d->x509_cert_->subject().GetDisplayName());
}

/*!
Returns the display name of the issuer of this certificate.
*/

QString OxideQSslCertificate::issuerDisplayName() const {
  if (!isValid()) {
    return QString();
  }

  return QString::fromStdString(d->x509_cert_->issuer().GetDisplayName());
}

/*!
Returns the value of the field specified by \a attr for this certificate's
subject.
*/

QStringList OxideQSslCertificate::getSubjectInfo(PrincipalAttr attr) const {
  if (!isValid()) {
    return QStringList();
  }

  return GetPrincipalValue(d->x509_cert_->subject(), attr);
}

/*!
Returns the value of the field specified by \a attr for this certificate's
issuer.
*/

QStringList OxideQSslCertificate::getIssuerInfo(PrincipalAttr attr) const {
  if (!isValid()) {
    return QStringList();
  }

  return GetPrincipalValue(d->x509_cert_->issuer(), attr);
}

/*!
Returns the start date for this certificate.
*/

QDateTime OxideQSslCertificate::effectiveDate() const {
  if (!isValid()) {
    return QDateTime();
  }

  return ToQDateTime(d->x509_cert_->valid_start());
}

/*!
Returns the expiry date for this certificate.
*/

QDateTime OxideQSslCertificate::expiryDate() const {
  if (!isValid()) {
    return QDateTime();
  }

  return ToQDateTime(d->x509_cert_->valid_expiry());
}

/*!
Returns the SHA1 fingerprint of this certificate as a hex encoded string.
*/

QString OxideQSslCertificate::fingerprintSHA1() const {
  if (!isValid()) {
    return QString();
  }

  std::string der;
  if (!net::X509Certificate::GetDEREncoded(d->x509_cert_->os_cert_handle(),
                                           &der)) {
    return QString();
  }

  QByteArray qder(der.data(), der.size());
  QSslCertificate qcert(qder, QSsl::Der);

  return QString::fromUtf8(qcert.digest(QCryptographicHash::Sha1).toHex());
}

/*!
Returns true if this certificate has expired.
*/

bool OxideQSslCertificate::isExpired() const {
  if (!isValid()) {
    return false;
  }

  return d->x509_cert_->HasExpired();
}

/*!
Returns the issuer of this certificate. If the certificate is not self-signed,
this will be a valid OxideQSslCertificate. If the certificate is self-signed,
this will be a null variant.

\sa QVariant::isNull
*/

QVariant OxideQSslCertificate::issuer() const {
  if (!isValid()) {
    // We return a null QVariant with the type VoidStar, as this gets converted
    // to null in QML engine
    return QVariant(static_cast<QVariant::Type>(QMetaType::VoidStar));
  }

  if (d->issuer_.get()) {
    return QVariant::fromValue(*d->issuer_);
  }

  const net::X509Certificate::OSCertHandles& handles =
      d->x509_cert_->GetIntermediateCertificates();
  if (handles.empty()) {
    return QVariant(static_cast<QVariant::Type>(QMetaType::VoidStar));
  }

  net::X509Certificate::OSCertHandle handle = handles[0];

  net::X509Certificate::OSCertHandles intermediates;
  for (size_t i = 1; i < handles.size(); ++i) {
    intermediates.push_back(handles[i]);
  }

  scoped_refptr<net::X509Certificate> cert =
      net::X509Certificate::CreateFromHandle(handle, intermediates);

  QSharedDataPointer<OxideQSslCertificateData> data(
      new OxideQSslCertificateData(cert.get()));
  d->issuer_.reset(new OxideQSslCertificate(data));

  return QVariant::fromValue(*d->issuer_);
}

/*!
\deprecated
*/

OxideQSslCertificate OxideQSslCertificate::copy() const {
  return OxideQSslCertificate(*this);
}

/*!
Returns a PEM encoded version of this certificate.
*/

QString OxideQSslCertificate::toPem() const {
  if (!isValid()) {
    return QString();
  }

  std::string pem;
  if (!net::X509Certificate::GetPEMEncoded(d->x509_cert_->os_cert_handle(),
                                           &pem)) {
    return QString();
  }

  return QString::fromStdString(pem);
}

/*!
Returns true if this is a valid certificate instance. Instances created with the
default constructor are invalid.
*/

bool OxideQSslCertificate::isValid() const {
  return !!d->x509_cert_;
}
