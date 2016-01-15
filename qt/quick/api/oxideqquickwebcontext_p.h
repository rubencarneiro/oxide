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
#include <QSharedPointer>
#include <QStringList>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_web_context_proxy.h"
#include "qt/core/glue/oxide_qt_web_context_proxy_client.h"

#include "qt/quick/api/oxideqquickwebcontext.h"

class OxideQQuickWebContextDelegateWorker;
class OxideQQuickUserScript;

QT_BEGIN_NAMESPACE
template <typename T> class QQmlListProperty;
class QThread;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {
namespace webcontextdelegateworker {
class IOThreadController;
}
class WebContextIODelegate;
}
}

class Q_DECL_EXPORT OxideQQuickWebContextPrivate
    : public QObject,
      public oxide::qt::WebContextProxyHandle,
      public oxide::qt::WebContextProxyClient {
  Q_OBJECT
  Q_DECLARE_PUBLIC(OxideQQuickWebContext)
  OXIDE_Q_DECL_PROXY_HANDLE_CONVERTER(OxideQQuickWebContext, oxide::qt::WebContextProxyHandle)

 public:
  ~OxideQQuickWebContextPrivate();

  bool isConstructed() const { return constructed_; }

  void delegateWorkerDestroyed(OxideQQuickWebContextDelegateWorker* worker);

  static OxideQQuickWebContextPrivate* get(OxideQQuickWebContext* context);

  void clearTemporarySavedPermissionStatuses();

  // XXX(chrisccoulson): Add CookieManager proxy and remove these
  bool isInitialized() const;
  int setCookies(const QUrl& url,
                 const QList<QNetworkCookie>& cookies);
  int getCookies(const QUrl& url);
  int getAllCookies();
  int deleteAllCookies();

 Q_SIGNALS:
  void constructed();

 private:
  OxideQQuickWebContextPrivate(OxideQQuickWebContext* q);

  oxide::qt::WebContextProxy* proxy() const {
    return oxide::qt::WebContextProxyHandle::proxy();
  }

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

  bool prepareToAttachDelegateWorker(OxideQQuickWebContextDelegateWorker* delegate);
  void detachedDelegateWorker(OxideQQuickWebContextDelegateWorker* delegate);

  // oxide::qt::WebContextProxyClient implementation
  void CookiesSet(int request_id,
                  const QList<QNetworkCookie>& failed_cookies) override;
  void CookiesRetrieved(int request_id,
                        const QList<QNetworkCookie>& cookies) override;
  void CookiesDeleted(int request_id, int num_deleted) override;
  QNetworkAccessManager* GetCustomNetworkAccessManager() override;
  void DestroyDefault() override;
  void DefaultAudioCaptureDeviceChanged() override;
  void DefaultVideoCaptureDeviceChanged() override;

  bool constructed_;

  QSharedPointer<oxide::qquick::WebContextIODelegate> io_;

  OxideQQuickWebContextDelegateWorker* network_request_delegate_;
  OxideQQuickWebContextDelegateWorker* unused_storage_access_permission_delegate_;
  OxideQQuickWebContextDelegateWorker* user_agent_override_delegate_;

  mutable OxideQQuickCookieManager* cookie_manager_;

  QStringList allowed_extra_url_schemes_;

  Q_DISABLE_COPY(OxideQQuickWebContextPrivate);
};

#endif // _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_P_H_
