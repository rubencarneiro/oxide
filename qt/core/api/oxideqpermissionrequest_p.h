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

#ifndef _OXIDE_QT_CORE_API_PERMISSION_REQUEST_P_H_
#define _OXIDE_QT_CORE_API_PERMISSION_REQUEST_P_H_

#include <QtGlobal>
#include <QUrl>

#include "base/memory/scoped_ptr.h"

#include "qt/core/api/oxideqpermissionrequest.h"

namespace oxide {
class PermissionRequest;
class SimplePermissionRequest;
}

class OxideQPermissionRequest;

class OxideQPermissionRequestPrivate {
  Q_DECLARE_PUBLIC(OxideQPermissionRequest)

 public:
  virtual ~OxideQPermissionRequestPrivate();

 protected:
  OxideQPermissionRequestPrivate(const QUrl& url,
                                 const QUrl& embedder,
                                 scoped_ptr<oxide::PermissionRequest> request);

  OxideQPermissionRequest* q_ptr;
  scoped_ptr<oxide::PermissionRequest> request_;
  bool is_cancelled_;

 private:
  void OnCancelled();

  QUrl url_;
  QUrl embedder_;
};

class OxideQSimplePermissionRequestPrivate
    : public OxideQPermissionRequestPrivate {
 public:
  virtual ~OxideQSimplePermissionRequestPrivate();

  static OxideQSimplePermissionRequest* Create(
      const QUrl& url,
      const QUrl& embedder,
      scoped_ptr<oxide::SimplePermissionRequest> request);

 protected:
  OxideQSimplePermissionRequestPrivate(
      const QUrl& url,
      const QUrl& embedder,
      scoped_ptr<oxide::SimplePermissionRequest> request);

 private:
  friend class OxideQSimplePermissionRequest;

  bool canRespond() const;
  oxide::SimplePermissionRequest* request() const;

  bool did_respond_;
};

class OxideQGeolocationPermissionRequestPrivate final
    : public OxideQSimplePermissionRequestPrivate {
 public:
  ~OxideQGeolocationPermissionRequestPrivate();

  static OxideQGeolocationPermissionRequest* Create(
      const QUrl& url,
      const QUrl& embedder,
      scoped_ptr<oxide::SimplePermissionRequest> request);

 private:
  OxideQGeolocationPermissionRequestPrivate(
      const QUrl& url,
      const QUrl& embedder,
      scoped_ptr<oxide::SimplePermissionRequest> request);
};

#endif // _OXIDE_QT_CORE_API_PERMISSION_REQUEST_P_H_
