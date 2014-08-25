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
#include <QSharedPointer>
#include <QtGlobal>
#include <QUrl>

class OxideQBeforeSendHeadersEvent;
class OxideQBeforeURLRequestEvent;
class OxideQStoragePermissionRequest;
class OxideQQuickWebContextDelegateWorker;

QT_BEGIN_NAMESPACE
class QVariant;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {
namespace webcontextdelegateworker {

class HelperThreadController;

class IOThreadController Q_DECL_FINAL : public QObject {
  Q_OBJECT

 public:
  IOThreadController(
      const QSharedPointer<HelperThreadController>& helper_thread_controller);
  virtual ~IOThreadController();

 Q_SIGNALS:
  void callEntryPointInWorker(const QString& entry, QObject* data);

 private:
  QSharedPointer<HelperThreadController> ht_controller_;
};

}
}
}

class OxideQQuickWebContextDelegateWorkerPrivate Q_DECL_FINAL :
    public QObject {
  Q_OBJECT

 public:
  ~OxideQQuickWebContextDelegateWorkerPrivate();

  static OxideQQuickWebContextDelegateWorkerPrivate* get(
      OxideQQuickWebContextDelegateWorker* q);

  QSharedPointer<oxide::qquick::webcontextdelegateworker::IOThreadController>
      io_thread_controller() const { return io_thread_controller_; }

  void incAttachedCount() { attached_count_++; }
  bool decAttachedCount() {
    Q_ASSERT(attached_count_ > 0);
    return --attached_count_ == 0;
  }

  bool in_destruction() const { return in_destruction_; }

 Q_SIGNALS:
  void runScript(const QUrl& source);
  void sendMessage(const QVariant& message);

 private:
  friend class OxideQQuickWebContextDelegateWorker;

  OxideQQuickWebContextDelegateWorkerPrivate();

  bool constructed_;
  QUrl source_;

  unsigned attached_count_;
  bool in_destruction_;

  QSharedPointer<oxide::qquick::webcontextdelegateworker::HelperThreadController>
      helper_thread_controller_;
  QSharedPointer<oxide::qquick::webcontextdelegateworker::IOThreadController>
      io_thread_controller_;
};

#endif // _OXIDE_QT_QUICK_API_WEB_CONTEXT_DELEGATE_WORKER_P_P_H_
