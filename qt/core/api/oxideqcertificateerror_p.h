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

#ifndef _OXIDE_QT_CORE_API_CERTIFICATE_ERROR_P_H_
#define _OXIDE_QT_CORE_API_CERTIFICATE_ERROR_P_H_

#include <QtGlobal>
#include <QUrl>

#include "base/memory/scoped_ptr.h"

#include "qt/core/api/oxideqcertificateerror.h"

namespace oxide {
class SimplePermissionRequest;
}

class OxideQSslCertificate;

class OxideQCertificateErrorPrivate final {
  Q_DECLARE_PUBLIC(OxideQCertificateError)

 public:
  ~OxideQCertificateErrorPrivate();

  static OxideQCertificateError* Create(
      const QUrl& url,
      bool is_main_frame,
      bool is_subresource,
      bool strict_enforcement,
      scoped_ptr<OxideQSslCertificate> certificate,
      OxideQCertificateError::Error cert_error,
      scoped_ptr<oxide::SimplePermissionRequest> request,
      QObject* parent = NULL);

 private:
  OxideQCertificateErrorPrivate(
      const QUrl& url,
      bool is_main_frame,
      bool is_subresource,
      bool strict_enforcement,
      scoped_ptr<OxideQSslCertificate> certificate,
      OxideQCertificateError::Error cert_error,
      scoped_ptr<oxide::SimplePermissionRequest> request);

  void OnCancel();
  void respond(bool accept);

  OxideQCertificateError* q_ptr;

  QUrl url_;
  bool is_main_frame_;
  bool is_subresource_;
  bool strict_enforcement_;
  scoped_ptr<OxideQSslCertificate> certificate_;
  OxideQCertificateError::Error cert_error_;
  scoped_ptr<oxide::SimplePermissionRequest> request_;

  bool did_respond_;
  bool is_cancelled_;
};

#endif // _OXIDE_QT_CORE_API_CERTIFICATE_ERROR_P_H_
