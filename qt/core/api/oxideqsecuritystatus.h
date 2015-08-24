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

#ifndef OXIDE_Q_SECURITY_STATUS
#define OXIDE_Q_SECURITY_STATUS

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QVariant>

class OxideQSecurityStatusPrivate;

class Q_DECL_EXPORT OxideQSecurityStatus Q_DECL_FINAL : public QObject {
  Q_OBJECT

  Q_PROPERTY(SecurityLevel securityLevel READ securityLevel NOTIFY securityLevelChanged)
  Q_PROPERTY(ContentStatusFlags contentStatus READ contentStatus NOTIFY contentStatusChanged)
  Q_PROPERTY(CertStatusFlags certStatus READ certStatus NOTIFY certStatusChanged)

  Q_PROPERTY(QVariant certificate READ certificate NOTIFY certificateChanged)

  Q_ENUMS(SecurityLevel)
  Q_FLAGS(ContentStatusFlags)
  Q_FLAGS(CertStatusFlags)
 
  Q_DECLARE_PRIVATE(OxideQSecurityStatus)
  Q_DISABLE_COPY(OxideQSecurityStatus)

 public:

  enum SecurityLevel {
    SecurityLevelNone,
    SecurityLevelSecure,
    SecurityLevelSecureEV,
    SecurityLevelWarning,
    SecurityLevelError
  };

  enum ContentStatus {
    ContentStatusNormal = 0,
    ContentStatusDisplayedInsecure = 1 << 0,
    ContentStatusRanInsecure = 1 << 1
  };
  Q_DECLARE_FLAGS(ContentStatusFlags, ContentStatus)

  enum CertStatus {
    CertStatusOk = 0,
    CertStatusBadIdentity = 1 << 0,
    CertStatusExpired = 1 << 1,
    CertStatusDateInvalid = 1 << 2,
    CertStatusAuthorityInvalid = 1 << 3,
    CertStatusRevocationCheckFailed = 1 << 4,
    CertStatusRevoked = 1 << 5,
    CertStatusInvalid = 1 << 6,
    CertStatusInsecure = 1 << 7,
    CertStatusGenericError = 1 << 8
  };
  Q_DECLARE_FLAGS(CertStatusFlags, CertStatus)

  ~OxideQSecurityStatus();

  SecurityLevel securityLevel() const;
  ContentStatusFlags contentStatus() const;
  CertStatusFlags certStatus() const;

  QVariant certificate() const;

 Q_SIGNALS:
  void securityLevelChanged();
  void contentStatusChanged();
  void certStatusChanged();
  void certificateChanged();

 private:
  OxideQSecurityStatus(OxideQSecurityStatusPrivate& dd,
                       QObject* parent = nullptr);

  QScopedPointer<OxideQSecurityStatusPrivate> d_ptr;
};

#endif // OXIDE_Q_SECURITY_STATUS
