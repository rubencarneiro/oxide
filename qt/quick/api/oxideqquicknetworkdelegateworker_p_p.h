// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_API_NETWORK_DELEGATE_WORKER_P_P_H_
#define _OXIDE_QT_QUICK_API_NETWORK_DELEGATE_WORKER_P_P_H_

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

class OxideQBeforeSendHeadersEvent;
class OxideQBeforeURLRequestEvent;
class OxideQStoragePermissionRequest;
class OxideQQuickNetworkDelegateWorker;

namespace oxide {
namespace qquick {

class NetworkDelegateWorkerHelperThreadController;
class NetworkDelegateWorkerUIThreadController;

class NetworkDelegateWorkerIOThreadController : public QObject {
  Q_OBJECT

 public:
  NetworkDelegateWorkerIOThreadController();
  virtual ~NetworkDelegateWorkerIOThreadController();

 Q_SIGNALS:
  void beforeURLRequest(OxideQBeforeURLRequestEvent* event);
  void beforeSendHeaders(OxideQBeforeSendHeadersEvent* event);
  void storagePermissionRequest(OxideQStoragePermissionRequest* req);
};

}
}

class OxideQQuickNetworkDelegateWorkerPrivate Q_DECL_FINAL {
 public:
  ~OxideQQuickNetworkDelegateWorkerPrivate();

  static OxideQQuickNetworkDelegateWorkerPrivate* get(
      OxideQQuickNetworkDelegateWorker* q);

  QScopedPointer<oxide::qquick::NetworkDelegateWorkerIOThreadController>
      io_thread_controller;

 private:
  friend class OxideQQuickNetworkDelegateWorker;

  OxideQQuickNetworkDelegateWorkerPrivate();

  bool constructed_;
  QUrl source_;

  oxide::qquick::NetworkDelegateWorkerHelperThreadController*
      helper_thread_controller_;
  QScopedPointer<oxide::qquick::NetworkDelegateWorkerUIThreadController>
      ui_thread_controller_;
};

#endif // _OXIDE_QT_QUICK_API_NETWORK_DELEGATE_WORKER_P_P_H_
