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

#ifndef OXIDE_Q_SECURITY_STATUS
#define OXIDE_Q_SECURITY_STATUS

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

class OxideQSecurityStatusPrivate;

class Q_DECL_EXPORT OxideQSecurityStatus Q_DECL_FINAL : public QObject {
  Q_OBJECT

  Q_PROPERTY(SecurityLevel securityLevel READ securityLevel NOTIFY securityLevelChanged)
  Q_PROPERTY(SecurityStyle securityStyle READ securityStyle NOTIFY securityStyleChanged)
  Q_PROPERTY(ContentStatus contentStatus READ contentStatus NOTIFY contentStatusChanged)
  Q_PROPERTY(CertErrorStatus certErrorStatus READ certErrorStatus NOTIFY certErrorStatusChanged)

  Q_ENUMS(SecurityLevel)
  Q_ENUMS(SecurityStyle)
  Q_FLAGS(ContentStatus)
  Q_FLAGS(CertErrorStatus)
 
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

  enum SecurityStyle {
    SecurityStyleUnknown,
    SecurityStyleUnauthenticated,
    SecurityStyleAuthenticationBroken,
    SecurityStyleAuthenticated
  };

  enum ContentStatusFlags {
    ContentStatusNormal = 0,
    ContentStatusDisplayedInsecure = 1 << 0,
    ContentStatusRanInsecure = 1 << 1
  };
  Q_DECLARE_FLAGS(ContentStatus, ContentStatusFlags)

  enum CertErrorStatusFlags {
    CertErrorStatusOk = 0,
    CertErrorStatusBadIdentity = 1 << 0,
    CertErrorStatusDateInvalid = 1 << 1,
    CertErrorStatusAuthorityInvalid = 1 << 2,
    CertErrorStatusRevocationCheckFailed = 1 << 3,
    CertErrorStatusRevoked = 1 << 4,
    CertErrorStatusInvalid = 1 << 5,
    CertErrorStatusInsecure = 1 << 6,
    CertErrorStatusGeneric = 1 << 7
  };
  Q_DECLARE_FLAGS(CertErrorStatus, CertErrorStatusFlags)

  Q_DECL_HIDDEN OxideQSecurityStatus(QObject* parent = NULL);
  ~OxideQSecurityStatus();

  SecurityLevel securityLevel() const;
  SecurityStyle securityStyle() const;
  ContentStatus contentStatus() const;
  CertErrorStatus certErrorStatus() const;

 Q_SIGNALS:
  void securityLevelChanged();
  void securityStyleChanged();
  void contentStatusChanged();
  void certErrorStatusChanged();

 private:
  QScopedPointer<OxideQSecurityStatusPrivate> d_ptr;
};

#endif // OXIDE_Q_SECURITY_STATUS
