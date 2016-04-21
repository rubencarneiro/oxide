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

#ifndef _OXIDE_QT_CORE_API_PERMISSION_REQUEST_P_H_
#define _OXIDE_QT_CORE_API_PERMISSION_REQUEST_P_H_

#include <memory>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/api/oxideqpermissionrequest.h"

namespace oxide {
class MediaAccessPermissionRequest;
class PermissionRequest;
}

class OxideQPermissionRequest;

class OxideQPermissionRequestPrivate {
  Q_DECLARE_PUBLIC(OxideQPermissionRequest)

 public:
  virtual ~OxideQPermissionRequestPrivate();

  static OxideQPermissionRequest* Create(
      std::unique_ptr<oxide::PermissionRequest> request);

 protected:
  OxideQPermissionRequestPrivate(
      std::unique_ptr<oxide::PermissionRequest> request);

  OxideQPermissionRequest* q_ptr;
  std::unique_ptr<oxide::PermissionRequest> request_;

 private:
  bool canRespond() const;

  void OnCancelled();
};

class OxideQGeolocationPermissionRequestPrivate
    : public OxideQPermissionRequestPrivate {
 public:
  ~OxideQGeolocationPermissionRequestPrivate() override;

  static OxideQGeolocationPermissionRequest* Create(
      std::unique_ptr<oxide::PermissionRequest> request);

 private:
  OxideQGeolocationPermissionRequestPrivate(
      std::unique_ptr<oxide::PermissionRequest> request);
};

class OxideQMediaAccessPermissionRequestPrivate
    : public OxideQPermissionRequestPrivate {
 public:
  ~OxideQMediaAccessPermissionRequestPrivate() override;

  static OxideQMediaAccessPermissionRequest* Create(
      std::unique_ptr<oxide::MediaAccessPermissionRequest> request);

 private:
  friend class OxideQMediaAccessPermissionRequest;

  OxideQMediaAccessPermissionRequestPrivate(
      std::unique_ptr<oxide::MediaAccessPermissionRequest> request);

  oxide::MediaAccessPermissionRequest* request() const;
};

#endif // _OXIDE_QT_CORE_API_PERMISSION_REQUEST_P_H_
