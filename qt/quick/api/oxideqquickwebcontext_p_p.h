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

#ifndef _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_P_H_
#define _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_P_H_

#include <QObject>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_web_context_adapter.h"

#include "qt/quick/api/oxideqquickwebcontext_p.h"

class OxideQQuickWebContextDelegateWorker;
class OxideQQuickUserScript;

QT_BEGIN_NAMESPACE
template <typename T> class QQmlListProperty;
class QThread;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {
class WebContextDelegateWorkerIOThreadController;
class WebContextIOThreadDelegate;
}
}

class OxideQQuickWebContextPrivate Q_DECL_FINAL :
    public QObject,
    public oxide::qt::WebContextAdapter {
  Q_OBJECT
  Q_DECLARE_PUBLIC(OxideQQuickWebContext)

 public:
  ~OxideQQuickWebContextPrivate();

  bool isConstructed() const { return constructed_; }

  void delegateWorkerDestroyed(OxideQQuickWebContextDelegateWorker* worker);

  static OxideQQuickWebContextPrivate* get(OxideQQuickWebContext* context);

  static void ensureChromiumStarted();

 Q_SIGNALS:
  void constructed();
  void willBeDestroyed();

 private:
  OxideQQuickWebContextPrivate(OxideQQuickWebContext* q);

  void userScriptUpdated();
  void userScriptWillBeDeleted();

  void detachUserScriptSignals(OxideQQuickUserScript* script);

  static void userScript_append(QQmlListProperty<OxideQQuickUserScript>* prop,
                                OxideQQuickUserScript* user_script);
  static int userScript_count(QQmlListProperty<OxideQQuickUserScript>* prop);
  static OxideQQuickUserScript* userScript_at(
      QQmlListProperty<OxideQQuickUserScript>* prop,
      int index);
  static void userScript_clear(QQmlListProperty<OxideQQuickUserScript>* prop);

  bool attachDelegateWorker(
      OxideQQuickWebContextDelegateWorker* worker,
      OxideQQuickWebContextDelegateWorker** ui_slot,
      oxide::qquick::WebContextDelegateWorkerIOThreadController** io_slot);

  bool constructed_;

  oxide::qquick::WebContextIOThreadDelegate* io_thread_delegate_;

  OxideQQuickWebContextDelegateWorker* network_request_delegate_;
  OxideQQuickWebContextDelegateWorker* storage_access_permission_delegate_;
  OxideQQuickWebContextDelegateWorker* user_agent_override_delegate_;

  Q_DISABLE_COPY(OxideQQuickWebContextPrivate);
};

#endif // _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_P_H_
