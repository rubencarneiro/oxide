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

#ifndef _OXIDE_QT_CORE_API_SECURITY_EVENTS_P_H_
#define _OXIDE_QT_CORE_API_SECURITY_EVENTS_P_H_

#include <QtGlobal>
#include <QUrl>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"

#include "qt/core/api/oxideqsecurityevents.h"

class OxideQSslCertificate;

class OxideQCertificateErrorPrivate Q_DECL_FINAL {
 public:
  ~OxideQCertificateErrorPrivate();

  static OxideQCertificateError* Create(
      const QUrl& url,
      bool is_main_frame,
      bool is_subresource,
      bool overridable,
      bool strict_enforcement,
      scoped_ptr<OxideQSslCertificate> certificate,
      OxideQCertificateError::CertError cert_error,
      const base::Callback<void(bool)>& callback,
      QObject* parent = NULL);

 private:
  friend class OxideQCertificateError;

  OxideQCertificateErrorPrivate(
      const QUrl& url,
      bool is_main_frame,
      bool is_subresource,
      bool overridable,
      bool strict_enforcement,
      scoped_ptr<OxideQSslCertificate> certificate,
      OxideQCertificateError::CertError cert_error,
      const base::Callback<void(bool)>& callback);

  void respond(bool accept);

  QUrl url_;
  bool is_main_frame_;
  bool is_subresource_;
  bool overridable_;
  bool strict_enforcement_;
  scoped_ptr<OxideQSslCertificate> certificate_;
  OxideQCertificateError::CertError cert_error_;
  base::Callback<void(bool)> callback_;
};

#endif // _OXIDE_QT_CORE_API_SECURITY_EVENTS_P_H_
