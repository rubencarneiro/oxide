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

#include <QLatin1String>
#include <QtGlobal>
#include <QtQml>
#include <QNetworkCookie>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "qt/core/api/oxideqdownloadrequest.h"
#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqnavigationrequest.h"
#include "qt/core/api/oxideqnewviewrequest.h"
#include "qt/core/api/oxideqpermissionrequest.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/quick/api/oxideqquickcookiemanager_p.h"
#include "qt/quick/api/oxideqquickglobals_p.h"
#include "qt/quick/api/oxideqquicknavigationhistory_p.h"
#include "qt/quick/api/oxideqquicknetworkcookies_p.h"
#include "qt/quick/api/oxideqquickscriptmessage_p.h"
#include "qt/quick/api/oxideqquickscriptmessagehandler_p.h"
#include "qt/quick/api/oxideqquickscriptmessagerequest_p.h"
#include "qt/quick/api/oxideqquickuserscript_p.h"
#include "qt/quick/api/oxideqquickwebcontext_p.h"
#include "qt/quick/api/oxideqquickwebcontextdelegateworker_p.h"
#include "qt/quick/api/oxideqquickwebframe_p.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

QT_USE_NAMESPACE

typedef QList<QNetworkCookie> CookieList;

QML_DECLARE_TYPE(CookieList)

namespace {

QObject* GlobalSingletonFactory(QQmlEngine* engine,
                                QJSEngine* script_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(script_engine);

  return OxideQQuickGlobals::instance();
}

}

class OxideQmlPlugin : public QQmlExtensionPlugin {
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "oxide_qml_plugin.json")
  Q_OBJECT
 public:
  void registerTypes(const char* uri) {
    Q_ASSERT(QLatin1String(uri) == QLatin1String("com.canonical.Oxide"));

    qRegisterMetaType<QNetworkCookie>();
    qRegisterMetaType<CookieList>();

    qmlRegisterSingletonType<OxideQQuickGlobals>(
        uri, 1, 0, "Oxide", GlobalSingletonFactory);

    qmlRegisterUncreatableType<OxideQGeolocationPermissionRequest>(uri, 1, 0,
        "GeolocationPermissionRequest",
        "GeolocationPermissionRequest is created by Oxide");
    qmlRegisterUncreatableType<OxideQLoadEvent>(uri, 1, 0, "LoadEvent",
        "LoadEvent' are created automatically by Oxide");
    qmlRegisterUncreatableType<OxideQQuickNavigationHistory>(uri, 1, 0, "NavigationHistory",
        "Each WebView has a NavigationHistory automatically instantiated by Oxide");
    qmlRegisterUncreatableType<OxideQNavigationRequest>(uri, 1, 0, "NavigationRequest",
        "Cannot create separate instance of NavigationRequest");
    qmlRegisterUncreatableType<OxideQNewViewRequest>(uri, 1, 0, "NewViewRequest",
        "NewViewRequest is created by Oxide");
    qmlRegisterUncreatableType<OxideQQuickScriptMessage>(uri, 1, 0, "ScriptMessage",
        "ScriptMessages are created automatically by Oxide");
    qmlRegisterUncreatableType<OxideQQuickScriptMessageRequest>(uri, 1, 0, "ScriptMessageRequest",
        "OutgoingMessageRequests are created automatically by WebFrame.sendMessage");
    qmlRegisterUncreatableType<OxideQQuickWebFrame>(uri, 1, 0, "WebFrame",
        "Frames are created automatically by Oxide to represent frames in the renderer");
    qmlRegisterUncreatableType<OxideQDownloadRequest>(uri, 1, 0, "DownloadRequest",
        "Cannot create separate instance of DownloadRequest");
    qmlRegisterUncreatableType<OxideQQuickCookieManager>(uri, 1, 0, "CookieManager",
        "Cannot create instances of CookieManager");

    qmlRegisterType<OxideQQuickScriptMessageHandler>(uri, 1, 0, "ScriptMessageHandler");
    qmlRegisterType<OxideQQuickUserScript>(uri, 1, 0, "UserScript");
    qmlRegisterType<OxideQQuickWebContext>(uri, 1, 0, "WebContext");
    qmlRegisterType<OxideQQuickWebContextDelegateWorker>(uri, 1, 0, "WebContextDelegateWorker");
    qmlRegisterType<OxideQWebPreferences>(uri, 1, 0, "WebPreferences");
    qmlRegisterType<OxideQQuickWebView>(uri, 1, 0, "WebView");
    qmlRegisterType<OxideQQuickNetworkCookies>(uri, 1, 0, "NetworkCookies");
  }
};

QML_DECLARE_TYPE(QNetworkCookie)

#include "oxide_qml_plugin.moc"
