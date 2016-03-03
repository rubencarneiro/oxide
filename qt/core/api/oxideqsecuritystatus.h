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

#ifndef OXIDE_QTCORE_SECURITY_STATUS
#define OXIDE_QTCORE_SECURITY_STATUS

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QtGlobal>
#include <QtCore/QVariant>

#include <OxideQtCore/oxideqglobal.h>

class OxideQSecurityStatusPrivate;

class OXIDE_QTCORE_EXPORT OxideQSecurityStatus : public QObject {
  Q_OBJECT

  Q_PROPERTY(SecurityLevel securityLevel READ securityLevel NOTIFY securityLevelChanged)
  Q_PROPERTY(ContentStatus contentStatus READ contentStatus NOTIFY contentStatusChanged)
  Q_PROPERTY(CertStatus certStatus READ certStatus NOTIFY certStatusChanged)

  Q_PROPERTY(QVariant certificate READ certificate NOTIFY certificateChanged)

  Q_ENUMS(SecurityLevel)
  Q_FLAGS(ContentStatus)
  Q_FLAGS(CertStatus)
 
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

  enum ContentStatusFlags {
    ContentStatusNormal = 0,
    ContentStatusDisplayedInsecure = 1 << 0,
    ContentStatusRanInsecure = 1 << 1
  };
  Q_DECLARE_FLAGS(ContentStatus, ContentStatusFlags)

  enum CertStatusFlags {
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
  Q_DECLARE_FLAGS(CertStatus, CertStatusFlags)

  ~OxideQSecurityStatus() Q_DECL_OVERRIDE;

  SecurityLevel securityLevel() const;
  ContentStatus contentStatus() const;
  CertStatus certStatus() const;

  QVariant certificate() const;

 Q_SIGNALS:
  void securityLevelChanged();
  void contentStatusChanged();
  void certStatusChanged();
  void certificateChanged();

 private:
  Q_DECL_HIDDEN OxideQSecurityStatus();

  QScopedPointer<OxideQSecurityStatusPrivate> d_ptr;
};

#endif // OXIDE_QTCORE_SECURITY_STATUS
