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

#include "oxideqsslcertificate.h"
#include "oxideqsslcertificate_p.h"

#include <string>
#include <vector>
#include <QByteArray>

#include "base/logging.h"
#include "base/time/time.h"
#include "net/base/hash_value.h"
#include "net/cert/x509_certificate.h"
#include "net/cert/x509_cert_types.h"

static QStringList GetPrincipalValue(const net::CertPrincipal& principal,
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

static QDateTime ToQDateTime(const base::Time& time) {
  int64_t ms = (time - base::Time::UnixEpoch()).InMilliseconds();
  return QDateTime::fromMSecsSinceEpoch(ms);
}

OxideQSslCertificatePrivate::OxideQSslCertificatePrivate(
    const scoped_refptr<net::X509Certificate>& cert)
    : x509_cert_(cert) {}

OxideQSslCertificatePrivate::~OxideQSslCertificatePrivate() {}

// static
OxideQSslCertificate* OxideQSslCertificatePrivate::Create(
    const scoped_refptr<net::X509Certificate>& cert,
    QObject* parent) {
  return new OxideQSslCertificate(
      *new OxideQSslCertificatePrivate(cert),
      parent);
}

OxideQSslCertificate::OxideQSslCertificate(OxideQSslCertificatePrivate& dd,
                                           QObject* parent)
    : QObject(parent),
      d_ptr(&dd) {}

OxideQSslCertificate::~OxideQSslCertificate() {}

QString OxideQSslCertificate::serialNumber() const {
  Q_D(const OxideQSslCertificate);

  const std::string& serial_number = d->x509_cert_->serial_number();
  QByteArray ba(serial_number.data(), int(serial_number.size()));

  return QString::fromUtf8(ba.toHex());
}

QString OxideQSslCertificate::subjectDisplayName() const {
  Q_D(const OxideQSslCertificate);

  return QString::fromStdString(d->x509_cert_->subject().GetDisplayName());
}

QString OxideQSslCertificate::issuerDisplayName() const {
  Q_D(const OxideQSslCertificate);

  return QString::fromStdString(d->x509_cert_->issuer().GetDisplayName());
}

QStringList OxideQSslCertificate::getSubjectInfo(PrincipalAttr attr) const {
  Q_D(const OxideQSslCertificate);

  return GetPrincipalValue(d->x509_cert_->subject(), attr);
}

QStringList OxideQSslCertificate::getIssuerInfo(PrincipalAttr attr) const {
  Q_D(const OxideQSslCertificate);

  return GetPrincipalValue(d->x509_cert_->issuer(), attr);
}

QDateTime OxideQSslCertificate::effectiveDate() const {
  Q_D(const OxideQSslCertificate);

  return ToQDateTime(d->x509_cert_->valid_start());
}

QDateTime OxideQSslCertificate::expiryDate() const {
  Q_D(const OxideQSslCertificate);

  return ToQDateTime(d->x509_cert_->valid_expiry());
}

QString OxideQSslCertificate::fingerprintSHA1() const {
  Q_D(const OxideQSslCertificate);

  const net::SHA1HashValue& hash = d->x509_cert_->fingerprint();
  QByteArray ba(reinterpret_cast<const char *>(hash.data), sizeof(hash.data));

  return QString::fromUtf8(ba.toHex());
}

bool OxideQSslCertificate::isExpired() const {
  Q_D(const OxideQSslCertificate);

  return d->x509_cert_->HasExpired();
}

OxideQSslCertificate* OxideQSslCertificate::issuer() const {
  Q_D(const OxideQSslCertificate);

  if (d->issuer_) {
    return d->issuer_.get();
  }

  const net::X509Certificate::OSCertHandles& handles =
      d->x509_cert_->GetIntermediateCertificates();
  if (handles.empty()) {
    return NULL;
  }

  net::X509Certificate::OSCertHandle handle = handles[0];

  net::X509Certificate::OSCertHandles intermediates;
  for (size_t i = 1; i < handles.size(); ++i) {
    intermediates.push_back(handles[i]);
  }

  scoped_refptr<net::X509Certificate> cert =
      net::X509Certificate::CreateFromHandle(handle, intermediates);
  d->issuer_.reset(OxideQSslCertificatePrivate::Create(cert));

  return d->issuer_.get();
}

OxideQSslCertificate* OxideQSslCertificate::copy() const {
  Q_D(const OxideQSslCertificate);

  return OxideQSslCertificatePrivate::Create(d->x509_cert_);
}

QString OxideQSslCertificate::toPem() const {
  Q_D(const OxideQSslCertificate);

  std::string pem;
  if (!net::X509Certificate::GetPEMEncoded(d->x509_cert_->os_cert_handle(),
                                           &pem)) {
    return QString();
  }

  return QString::fromStdString(pem);
}
