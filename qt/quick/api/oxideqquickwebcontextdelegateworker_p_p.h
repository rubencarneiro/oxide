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

#ifndef _OXIDE_QT_QUICK_API_WEB_CONTEXT_DELEGATE_WORKER_P_P_H_
#define _OXIDE_QT_QUICK_API_WEB_CONTEXT_DELEGATE_WORKER_P_P_H_

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

class OxideQBeforeSendHeadersEvent;
class OxideQBeforeURLRequestEvent;
class OxideQStoragePermissionRequest;
class OxideQQuickWebContextDelegateWorker;

namespace oxide {
namespace qquick {

class WebContextDelegateWorkerHelperThreadController;
class WebContextDelegateWorkerUIThreadController;

class WebContextDelegateWorkerIOThreadController : public QObject {
  Q_OBJECT

 public:
  WebContextDelegateWorkerIOThreadController();
  virtual ~WebContextDelegateWorkerIOThreadController();

 Q_SIGNALS:
  void callEntryPointInWorker(const QString& entry, QObject* data);
};

}
}

class OxideQQuickWebContextDelegateWorkerPrivate Q_DECL_FINAL {
 public:
  ~OxideQQuickWebContextDelegateWorkerPrivate();

  static OxideQQuickWebContextDelegateWorkerPrivate* get(
      OxideQQuickWebContextDelegateWorker* q);

  QScopedPointer<oxide::qquick::WebContextDelegateWorkerIOThreadController>
      io_thread_controller;

 private:
  friend class OxideQQuickWebContextDelegateWorker;

  OxideQQuickWebContextDelegateWorkerPrivate();

  bool constructed_;
  QUrl source_;

  oxide::qquick::WebContextDelegateWorkerHelperThreadController*
      helper_thread_controller_;
  QScopedPointer<oxide::qquick::WebContextDelegateWorkerUIThreadController>
      ui_thread_controller_;
};

#endif // _OXIDE_QT_QUICK_API_WEB_CONTEXT_DELEGATE_WORKER_P_P_H_
