// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#include <functional>
#include <memory>
#include <QtGlobal>

#include "qt/core/api/oxideqcertificateerror.h"
#include "qt/core/api/oxideqglobal.h"
#include "qt/core/api/oxideqsslcertificate.h"

namespace oxide {
class CertificateError;
}

class OXIDE_QTCORE_EXPORT OxideQCertificateErrorPrivate final {
  Q_DECLARE_PUBLIC(OxideQCertificateError)

 public:
  ~OxideQCertificateErrorPrivate();

  static OxideQCertificateErrorPrivate* get(OxideQCertificateError* q);

  static std::unique_ptr<OxideQCertificateError> Create(
      std::unique_ptr<oxide::CertificateError> error,
      QObject* parent = nullptr);

  static std::unique_ptr<OxideQCertificateError> CreateForTesting(
      bool is_main_frame,
      bool is_subresource,
      OxideQCertificateError::Error error,
      const OxideQSslCertificate& cert,
      const QUrl& url,
      bool strict_enforcement,
      bool overridable,
      const std::function<void(bool)>& callback);

  void SimulateCancel();

 private:
  OxideQCertificateErrorPrivate(
      std::unique_ptr<oxide::CertificateError> error);

  void OnCancel();
  void respond(bool accept);

  OxideQCertificateError* q_ptr;

  OxideQSslCertificate certificate_;
  std::unique_ptr<oxide::CertificateError> error_;

  bool did_respond_;
};

#endif // _OXIDE_QT_CORE_API_CERTIFICATE_ERROR_P_H_
