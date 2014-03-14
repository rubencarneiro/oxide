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

#include <QtGlobal>

#include "qt/core/glue/oxide_qt_web_context_adapter.h"

class OxideQQuickNetworkDelegateWorker;
class OxideQQuickWebContext;
class OxideQQuickUserScript;

QT_BEGIN_NAMESPACE
template <typename T> class QQmlListProperty;
class QThread;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {
class NetworkDelegateWorkerIOThreadController;
class WebContextIOThreadDelegate;
}
}

class OxideQQuickWebContextPrivate Q_DECL_FINAL :
    public oxide::qt::WebContextAdapter {
  Q_DECLARE_PUBLIC(OxideQQuickWebContext)

 public:
  ~OxideQQuickWebContextPrivate();

  void networkDelegateWorkerDestroyed(OxideQQuickNetworkDelegateWorker* worker);

  static OxideQQuickWebContextPrivate* get(OxideQQuickWebContext* context);

  static void ensureChromiumStarted();

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

  bool attachNetworkDelegateWorker(
      OxideQQuickNetworkDelegateWorker* worker,
      OxideQQuickNetworkDelegateWorker** ui_slot,
      oxide::qquick::NetworkDelegateWorkerIOThreadController** io_slot);

  OxideQQuickWebContext* q_ptr;

  oxide::qquick::WebContextIOThreadDelegate* io_thread_delegate_;

  OxideQQuickNetworkDelegateWorker* network_request_delegate_;
  OxideQQuickNetworkDelegateWorker* storage_access_permission_delegate_;

  Q_DISABLE_COPY(OxideQQuickWebContextPrivate);
};

#endif // _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_P_H_
