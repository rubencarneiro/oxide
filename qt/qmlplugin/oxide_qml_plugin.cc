// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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
#include <QNetworkCookie>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QtQml>
#include <QtQml/private/qqmlvaluetype_p.h>

#include "qt/core/api/oxideqcertificateerror.h"
#include "qt/core/api/oxideqdownloadrequest.h"
#include "qt/core/api/oxideqfindcontroller.h"
#include "qt/core/api/oxideqhttpauthenticationrequest.h"
#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqnavigationrequest.h"
#include "qt/core/api/oxideqnewviewrequest.h"
#include "qt/core/api/oxideqpermissionrequest.h"
#include "qt/core/api/oxideqsecuritystatus.h"
#include "qt/core/api/oxideqsslcertificate.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/quick/api/oxideqquickcookiemanager_p.h"
#include "qt/quick/api/oxideqquickglobal_p.h"
#include "qt/quick/api/oxideqquicklocationbarcontroller.h"
#include "qt/quick/api/oxideqquicknavigationhistory.h"
#include "qt/quick/api/oxideqquickscriptmessage.h"
#include "qt/quick/api/oxideqquickscriptmessagehandler.h"
#include "qt/quick/api/oxideqquickscriptmessagerequest.h"
#include "qt/quick/api/oxideqquicktouchselectioncontroller.h"
#include "qt/quick/api/oxideqquickuserscript.h"
#include "qt/quick/api/oxideqquickwebcontext.h"
#include "qt/quick/api/oxideqquickwebcontextdelegateworker_p.h"
#include "qt/quick/api/oxideqquickwebframe.h"
#include "qt/quick/api/oxideqquickwebview.h"

#ifdef LEGACY_QMLVALUE_TYPES
#include "oxide_qml_download_request.h"
#include "oxide_qml_load_event.h"
#include "oxide_qml_navigation_item.h"
#include "oxide_qml_ssl_certificate.h"
#include "oxide_qml_value_type_provider.h"
#endif

typedef QList<QNetworkCookie> CookieList;

QML_DECLARE_TYPE(QNetworkCookie)
QML_DECLARE_TYPE(CookieList)

namespace {

QObject* GlobalSingletonFactory(QQmlEngine* engine,
                                QJSEngine* script_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(script_engine);

  return new OxideQQuickGlobal();
}

#ifdef LEGACY_QMLVALUE_TYPES
oxide::qmlplugin::ValueTypeProvider* GetValueTypeProvider() {
  static oxide::qmlplugin::ValueTypeProvider g_provider;
  return &g_provider;
}
#endif

}

class OxideQmlPlugin : public QQmlExtensionPlugin {
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "oxide_qml_plugin.json")
  Q_OBJECT
 public:
  void registerTypes(const char* uri) {
    Q_ASSERT(QLatin1String(uri) == QLatin1String("com.canonical.Oxide"));

    qRegisterMetaType<QNetworkCookie>();
    qRegisterMetaType<CookieList>();

    qRegisterMetaType<OxideQDownloadRequest>();
    qRegisterMetaType<OxideQLoadEvent>();
    qRegisterMetaType<OxideQSslCertificate>();

#ifdef LEGACY_QMLVALUE_TYPES
    QQml_addValueTypeProvider(GetValueTypeProvider());

    qmlRegisterUncreatableType<oxide::qmlplugin::DownloadRequest>(
        uri, 1, 0, "DownloadRequest",
        "DownloadRequest is delivered by WebView.downloadRequested");
    qmlRegisterUncreatableType<oxide::qmlplugin::LoadEvent>(
        uri, 1, 0, "LoadEvent",
        "LoadEvent is delivered by WebView.loadingChanged");
    qmlRegisterUncreatableType<oxide::qmlplugin::NavigationItem>(
        uri, 1, 0, "NavigationItem",
        "NavigationItem is created by NavigationHistory");
    qmlRegisterUncreatableType<oxide::qmlplugin::SslCertificate>(
        uri, 1, 0, "SslCertificate",
        "SslCertificate is accessed via SecurityStatus.certificate");
#else
    qmlRegisterValueTypeEnums<OxideQLoadEvent>(uri, 1, 0, "LoadEvent");
    qmlRegisterValueTypeEnums<OxideQSslCertificate>(uri, 1, 0, "SslCertificate");
#endif

    qmlRegisterSingletonType<OxideQQuickGlobal>(
        uri, 1, 0, "Oxide", GlobalSingletonFactory);

    qmlRegisterUncreatableType<OxideQCertificateError>(
        uri, 1, 0, "CertificateError",
        "CertificateError is delivered by WebView.certificateError");
    qmlRegisterUncreatableType<OxideQQuickCookieManager>(
        uri, 1, 0, "CookieManager",
        "CookieManager is accessed via WebContext.cookieManager");
    qmlRegisterUncreatableType<OxideQGeolocationPermissionRequest>(
        uri, 1, 0, "GeolocationPermissionRequest",
        "GeolocationPermissionRequest is delivered by "
        "WebView.geolocationPermissionRequested");
    qmlRegisterUncreatableType<OxideQQuickNavigationHistory>(
        uri, 1, 0, "NavigationHistory",
        "NavigationHistory is accessed via WebView.navigationHistory");
    qmlRegisterUncreatableType<OxideQNavigationRequest>(
        uri, 1, 0, "NavigationRequest",
        "NavigationRequest is delivered by WebView.navigationRequested");
    qmlRegisterUncreatableType<OxideQNewViewRequest>(
        uri, 1, 0, "NewViewRequest",
        "NewViewRequest is delivered by WebView.newViewRequested");
    qmlRegisterUncreatableType<OxideQPermissionRequest>(
        uri, 1, 0, "PermissionRequest",
        "PermissionRequest is delivered by one of the permission request "
        "signals on WebView");
    qmlRegisterUncreatableType<OxideQQuickScriptMessage>(
        uri, 1, 0, "ScriptMessage",
        "ScriptMessage is delivered by ScriptMessageHandler.callback");
    qmlRegisterUncreatableType<OxideQQuickScriptMessageRequest>(
        uri, 1, 0, "ScriptMessageRequest",
        "ScriptMessageRequest is returned from WebFrame.sendMessage");
    qmlRegisterUncreatableType<OxideQSecurityStatus>(
        uri, 1, 0, "SecurityStatus",
        "SecurityStatus is accessed via WebView.securityStatus");
    qmlRegisterUncreatableType<OxideQQuickWebFrame>(uri, 1, 0, "WebFrame",
        "WebFrame is accessed via WebView.rootFrame, WebFrame.childFrames and "
        "WebFrame.parentFrame");

    qmlRegisterType<OxideQQuickScriptMessageHandler>(uri, 1, 0,
        "ScriptMessageHandler");
    qmlRegisterType<OxideQQuickUserScript>(uri, 1, 0, "UserScript");
    qmlRegisterType<OxideQQuickWebContext>(uri, 1, 0, "WebContext");
    qmlRegisterType<OxideQQuickWebContextDelegateWorker>(uri, 1, 0,
        "WebContextDelegateWorker");
    qmlRegisterType<OxideQWebPreferences>(uri, 1, 0, "WebPreferences");
    qmlRegisterType<OxideQQuickWebView>(uri, 1, 0, "WebView");

    qmlRegisterUncreatableType<OxideQQuickCookieManager, 1>(
        uri, 1, 3, "CookieManager",
        "CookieManager is accessed via WebContext.cookieManager");
    qmlRegisterType<OxideQQuickWebContext, 1>(uri, 1, 3, "WebContext");
    qmlRegisterType<OxideQQuickWebView, 1>(uri, 1, 3, "WebView");

    qmlRegisterUncreatableType<OxideQQuickLocationBarController>(
        uri, 1, 4, "LocationBarController",
        "LocationBarController is accessed via WebView.locationBarController");
    qmlRegisterType<OxideQQuickWebView, 2>(uri, 1, 4, "WebView");

    qmlRegisterType<OxideQQuickWebView, 3>(uri, 1, 5, "WebView");

    qmlRegisterType<OxideQQuickWebContext, 2>(uri, 1, 6, "WebContext");

    qmlRegisterUncreatableType<OxideQQuickLocationBarController, 1>(
        uri, 1, 7, "LocationBarController",
        "LocationBarController is accessed via WebView.locationBarController");

    qmlRegisterUncreatableType<OxideQFindController>(
        uri, 1, 8, "FindController",
        "FindController is accessed via WebView.findController");
    qmlRegisterUncreatableType<OxideQMediaAccessPermissionRequest>(
        uri, 1, 0, "MediaAccessPermissionRequest",
        "MediaAccessPermissionRequest is delivered by "
        "WebView.mediaAccessPermissionRequested");
    qmlRegisterType<OxideQQuickWebView, 4>(uri, 1, 8, "WebView");

    qmlRegisterType<OxideQQuickWebContext, 3>(uri, 1, 9, "WebContext");
    qmlRegisterUncreatableType<OxideQHttpAuthenticationRequest>(
        uri, 1, 9, "HttpAuthenticationRequest",
        "HttpAuthenticationRequest is delivered by "
        "WebView.httpAuthenticationRequested");
    qmlRegisterType<OxideQQuickWebView, 5>(uri, 1, 9, "WebView");

    qmlRegisterType<OxideQQuickWebView, 6>(uri, 1, 11, "WebView");

    qmlRegisterUncreatableType<OxideQQuickTouchSelectionController>(
        uri, 1, 12, "TouchSelectionController",
        "TouchSelectionController is accessed via "
        "WebView.touchSelectionController");
    qmlRegisterType<OxideQQuickWebView, 7>(uri, 1, 12, "WebView");

    qmlRegisterUncreatableType<OxideQQuickTouchSelectionController, 1>(
        uri, 1, 15, "TouchSelectionController",
        "TouchSelectionController is accessed via "
        "WebView.touchSelectionController");
    qmlRegisterType<OxideQQuickWebView, 8>(uri, 1, 15, "WebView");

    qmlRegisterType<OxideQQuickWebView, 9>(uri, 1, 19, "WebView");
    qmlRegisterUncreatableType<OxideQQuickNavigationHistory, 1>(
        uri, 1, 19, "NavigationHistory",
        "NavigationHistory is accessed via WebView.navigationHistory");
  }
};

#include "oxide_qml_plugin.moc"
