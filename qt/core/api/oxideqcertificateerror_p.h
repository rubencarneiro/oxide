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

#ifndef _OXIDE_QT_CORE_API_CERTIFICATE_ERROR_P_H_
#define _OXIDE_QT_CORE_API_CERTIFICATE_ERROR_P_H_

#include <QtGlobal>

#include "base/memory/scoped_ptr.h"

#include "qt/core/api/oxideqcertificateerror.h"
#include "qt/core/api/oxideqsslcertificate.h"

namespace oxide {
class CertificateError;
}

class OxideQSslCertificate;

class OxideQCertificateErrorPrivate final {
  Q_DECLARE_PUBLIC(OxideQCertificateError)

 public:
  ~OxideQCertificateErrorPrivate();

  static OxideQCertificateError* Create(
      scoped_ptr<oxide::CertificateError> error,
      QObject* parent = nullptr);

 private:
  OxideQCertificateErrorPrivate(
      scoped_ptr<oxide::CertificateError> error);

  void OnCancel();
  void respond(bool accept);

  OxideQCertificateError* q_ptr;

  OxideQSslCertificate certificate_;
  scoped_ptr<oxide::CertificateError> error_;

  bool did_respond_;
};

#endif // _OXIDE_QT_CORE_API_CERTIFICATE_ERROR_P_H_
