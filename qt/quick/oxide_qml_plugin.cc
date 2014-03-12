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
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/quick/api/oxideqquickglobals_p.h"
#include "qt/quick/api/oxideqquicknavigationhistory_p.h"
#include "qt/quick/api/oxideqquicknetworkdelegateworker_p.h"
#include "qt/quick/api/oxideqquickscriptmessage_p.h"
#include "qt/quick/api/oxideqquickscriptmessagehandler_p.h"
#include "qt/quick/api/oxideqquickscriptmessagerequest_p.h"
#include "qt/quick/api/oxideqquickuserscript_p.h"
#include "qt/quick/api/oxideqquickwebcontext_p.h"
#include "qt/quick/api/oxideqquickwebframe_p.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

QT_USE_NAMESPACE

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

    qmlRegisterSingletonType<OxideQQuickGlobals>(
        uri, 0, 1, "Oxide", GlobalSingletonFactory);
    qmlRegisterUncreatableType<OxideQQuickScriptMessage>(uri, 0, 1, "ScriptMessage",
        "ScriptMessages are created automatically by Oxide");
    qmlRegisterUncreatableType<OxideQLoadEvent>(uri, 0, 1, "LoadEvent",
        "LoadEvent' are created automatically by Oxide");
    qmlRegisterUncreatableType<OxideQQuickScriptMessageRequest>(uri, 0, 1, "ScriptMessageRequest",
        "OutgoingMessageRequests are created automatically by WebFrame.sendMessage");
    qmlRegisterType<OxideQQuickUserScript>(uri, 0, 1, "UserScript");
    qmlRegisterType<OxideQQuickScriptMessageHandler>(uri, 0, 1, "ScriptMessageHandler");
    qmlRegisterUncreatableType<OxideQQuickWebFrame>(uri, 0, 1, "WebFrame",
        "Frames are created automatically by Oxide to represent frames in the renderer");
    qmlRegisterType<OxideQQuickWebContext>(uri, 0, 1, "WebContext");
    qmlRegisterUncreatableType<OxideQQuickNavigationHistory>(uri, 0, 1, "NavigationHistory",
        "Each WebView has a NavigationHistory automatically instantiated by Oxide");
    qmlRegisterType<OxideQWebPreferences>(uri, 0, 1, "WebPreferences");
    qmlRegisterType<OxideQQuickWebView>(uri, 0, 1, "WebView");
    qmlRegisterType<OxideQQuickNetworkDelegateWorker>(uri, 0, 1, "NetworkDelegateWorker");
  }
};

#include "oxide_qml_plugin.moc"
