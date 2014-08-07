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

#include "oxide_qt_web_context_adapter.h"
#include "oxide_qt_web_context_adapter_p.h"

#include <string>
#include <vector>

#include <QCoreApplication>
#include <QDateTime>
#include <QGuiApplication>
#include <QObject>
#include <QtDebug>

#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QObject>
#include <QNetworkCookie>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/static_cookie_policy.h"
#include "net/cookies/cookie_monster.h"
#include "url/gurl.h"

#include "qt/core/app/oxide_qt_content_main_delegate.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_process_main.h"

namespace oxide {
namespace qt {

namespace {
QOpenGLContext* g_shared_gl_context;
}

WebContextAdapter::~WebContextAdapter() {
  priv->Destroy();
  priv->Release();
}

QString WebContextAdapter::product() const {
  if (isInitialized()) {
    return QString::fromStdString(priv->context_->GetProduct());
  }

  return QString::fromStdString(priv->construct_props_->product);
}

void WebContextAdapter::setProduct(const QString& product) {
  if (isInitialized()) {
    priv->context_->SetProduct(product.toStdString());
  } else {
    priv->construct_props_->product = product.toStdString();
  }
}

QString WebContextAdapter::userAgent() const {
  if (isInitialized()) {
    return QString::fromStdString(priv->context_->GetUserAgent());
  }

  return QString::fromStdString(priv->construct_props_->user_agent);
}

void WebContextAdapter::setUserAgent(const QString& user_agent) {
  if (isInitialized()) {
    priv->context_->SetUserAgent(user_agent.toStdString());
  } else {
    priv->construct_props_->user_agent = user_agent.toStdString();
  }
}

bool WebContextAdapter::devtoolsEnabled() const {
  if (isInitialized()) {
    return priv->context_->GetDevtoolsEnabled();
  }
  return priv->construct_props_->devtools_enabled;
}

void WebContextAdapter::setDevtoolsEnabled(bool enabled) {
  if (isInitialized()) {
    qWarning() << "Cannot change the devtools enabled after inititialization";
    return;
  }
  priv->construct_props_->devtools_enabled = enabled;
}

int WebContextAdapter::devtoolsPort() const {
  if (isInitialized()) {
    return priv->context_->GetDevtoolsPort();
  }

  return priv->construct_props_->devtools_port;
}

void WebContextAdapter::setDevtoolsPort(int port) {
  if (isInitialized()) {
    qWarning() << "Cannot change the devtools port after inititialization";
    return;
  }
  priv->construct_props_->devtools_port = port;
}

QUrl WebContextAdapter::dataPath() const {
  base::FilePath path;
  if (isInitialized()) {
    path = priv->context_->GetPath();
  } else {
    path = priv->construct_props_->data_path;
  }

  if (path.empty()) {
    return QUrl();
  }

  return QUrl::fromLocalFile(QString::fromStdString(path.value()));
}

void WebContextAdapter::setDataPath(const QUrl& url) {
  if (!url.isLocalFile() && !url.isEmpty()) {
    qWarning() << "dataPath only supports local files";
    return;
  }

  DCHECK(!isInitialized());
  priv->construct_props_->data_path =
      base::FilePath(url.toLocalFile().toStdString());
}

QUrl WebContextAdapter::cachePath() const {
  base::FilePath path;
  if (isInitialized()) {
    path = priv->context_->GetCachePath();
  } else {
    path = priv->construct_props_->cache_path;
  }

  if (path.empty()) {
    return QUrl();
  }

  return QUrl::fromLocalFile(QString::fromStdString(path.value()));
}

void WebContextAdapter::setCachePath(const QUrl& url) {
  if (!url.isLocalFile() && !url.isEmpty()) {
    qWarning() << "cachePath only supports local files";
    return;
  }

  DCHECK(!isInitialized());
  priv->construct_props_->cache_path =
      base::FilePath(url.toLocalFile().toStdString());
}

QString WebContextAdapter::acceptLangs() const {
  if (isInitialized()) {
    return QString::fromStdString(priv->context_->GetAcceptLangs());
  }

  return QString::fromStdString(priv->construct_props_->accept_langs);
}

void WebContextAdapter::setAcceptLangs(const QString& langs) {
  if (isInitialized()) {
    priv->context_->SetAcceptLangs(langs.toStdString());
  } else {
    priv->construct_props_->accept_langs = langs.toStdString();
  }
}

QList<UserScriptAdapter *>& WebContextAdapter::userScripts() {
  return priv->user_scripts_;
}

void WebContextAdapter::updateUserScripts() {
  priv->UpdateUserScripts();
}

bool WebContextAdapter::isInitialized() const {
  return priv->context_ != NULL;
}

/* static */
QOpenGLContext* WebContextAdapter::sharedGLContext() {
  return g_shared_gl_context;
}

/* static */
void WebContextAdapter::setSharedGLContext(QOpenGLContext* context) {
  CHECK(!oxide::BrowserProcessMain::GetInstance()->IsRunning()) <<
      "WebContextAdapter::setSharedGLContext must be called before the "
      "browser components are started!";

  g_shared_gl_context = context;
}

namespace {
void ShutdownChromium() {
  oxide::BrowserProcessMain::GetInstance()->Shutdown();
}
}

/* static */
void WebContextAdapter::ensureChromiumStarted() {
  if (!oxide::BrowserProcessMain::GetInstance()->IsRunning()) {
    CHECK(qobject_cast<QGuiApplication *>(QCoreApplication::instance())) <<
        "Your application doesn't have a QGuiApplication. Oxide will not "
        "function without one";
    scoped_ptr<ContentMainDelegate> delegate(ContentMainDelegate::Create());

    oxide::BrowserProcessMain::GetInstance()->Start(
        delegate.PassAs<oxide::ContentMainDelegate>());
    qAddPostRoutine(ShutdownChromium);
  }
}

WebContextAdapter::IOThreadDelegate*
WebContextAdapter::getIOThreadDelegate() const {
  return priv->io_thread_delegate_.get();
}

WebContextAdapter::CookiePolicy WebContextAdapter::cookiePolicy() const {
  if (isInitialized()) {
    return static_cast<CookiePolicy>(priv->context_->GetCookiePolicy());
  }

  return static_cast<CookiePolicy>(priv->construct_props_->cookie_policy);
}

void WebContextAdapter::setCookiePolicy(CookiePolicy policy) {
  if (isInitialized()) {
    priv->context_->SetCookiePolicy(
        static_cast<net::StaticCookiePolicy::Type>(policy));
  } else {
    priv->construct_props_->cookie_policy =
        static_cast<net::StaticCookiePolicy::Type>(policy);
  }
}

WebContextAdapter::SessionCookieMode
WebContextAdapter::sessionCookieMode() const {
  content::CookieStoreConfig::SessionCookieMode mode;
  if (isInitialized()) {
    mode = priv->context_->GetSessionCookieMode();
  } else {
    mode = priv->construct_props_->session_cookie_mode;
  }

  switch (mode) {
    case content::CookieStoreConfig::PERSISTANT_SESSION_COOKIES:
      return SessionCookieModePersistent;
    case content::CookieStoreConfig::RESTORED_SESSION_COOKIES:
      return SessionCookieModeRestored;
    default:
      return SessionCookieModeEphemeral;
  }
}

void WebContextAdapter::setSessionCookieMode(SessionCookieMode mode) {
  DCHECK(!isInitialized());
  content::CookieStoreConfig::SessionCookieMode cookie_mode;
  switch (mode) {
    case SessionCookieModePersistent:
      cookie_mode = content::CookieStoreConfig::PERSISTANT_SESSION_COOKIES;
      break;
    case SessionCookieModeRestored:
      cookie_mode = content::CookieStoreConfig::RESTORED_SESSION_COOKIES;
      break;
    default:
      cookie_mode = content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES;
  }
  priv->construct_props_->session_cookie_mode = cookie_mode;
}

bool WebContextAdapter::popupBlockerEnabled() const {
  if (isInitialized()) {
    return priv->context_->IsPopupBlockerEnabled();
  }

  return priv->construct_props_->popup_blocker_enabled;
}

void WebContextAdapter::setPopupBlockerEnabled(bool enabled) {
  if (isInitialized()) {
    priv->context_->SetIsPopupBlockerEnabled(enabled);
  } else {
    priv->construct_props_->popup_blocker_enabled = enabled;
  }
}

void WebContextAdapter::doSetCookies(
      const QString& url,
      OxideQQuickNetworkCookies* cookies,
      int requestId) {
  priv->doSetCookies(url, cookies, requestId);
}

void WebContextAdapter::doGetAllCookies(int requestId) {
  priv->doGetAllCookies(requestId);
}

void WebContextAdapter::CookiesSet(int requestId, bool status) {
}

void WebContextAdapter::CookiesRetrieved(
      int requestId, OxideQQuickNetworkCookies* cookies) {
}

WebContextAdapter::WebContextAdapter(
    QObject* q,
    IOThreadDelegate* io_delegate)
    : AdapterBase(q),
      priv(WebContextAdapterPrivate::Create(this, io_delegate)) {

  priv->AddRef();

  COMPILE_ASSERT(
      CookiePolicyAllowAll == static_cast<CookiePolicy>(
        net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
      cookie_enums_allowall_doesnt_match);
  COMPILE_ASSERT(
      CookiePolicyBlockAll == static_cast<CookiePolicy>(
        net::StaticCookiePolicy::BLOCK_ALL_COOKIES),
      cookie_enums_blockall_doesnt_match);
  COMPILE_ASSERT(
      CookiePolicyBlockThirdParty == static_cast<CookiePolicy>(
        net::StaticCookiePolicy::BLOCK_ALL_THIRD_PARTY_COOKIES),
      cookie_enums_blockall3rdparty_doesnt_match);
}

} // namespace qt
} // namespace oxide
