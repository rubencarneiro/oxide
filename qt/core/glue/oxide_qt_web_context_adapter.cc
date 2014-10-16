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

#include <string>

#include <QNetworkCookie>
#include <QtDebug>

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/cookie_store_factory.h"
#include "net/base/static_cookie_policy.h"

#include "qt/core/browser/oxide_qt_web_context.h"

namespace oxide {
namespace qt {

QNetworkAccessManager* WebContextAdapter::GetCustomNetworkAccessManager() {
  return NULL;
}

WebContextAdapter::~WebContextAdapter() {
  context_->Destroy();
  context_->Release();
}

// static
WebContextAdapter* WebContextAdapter::FromWebContext(WebContext* context) {
  if (!context) {
    return NULL;
  }

  return context->GetAdapter();
}

void WebContextAdapter::init(const QWeakPointer<IODelegate>& io_delegate) {
  context_->Init(io_delegate);
}

QString WebContextAdapter::product() const {
  return QString::fromStdString(context_->GetProduct());
}

void WebContextAdapter::setProduct(const QString& product) {
  context_->SetProduct(product.toStdString());
}

QString WebContextAdapter::userAgent() const {
  return QString::fromStdString(context_->GetUserAgent());
}

void WebContextAdapter::setUserAgent(const QString& user_agent) {
  context_->SetUserAgent(user_agent.toStdString());
}

QUrl WebContextAdapter::dataPath() const {
  base::FilePath path = context_->GetDataPath();
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

  context_->SetDataPath(base::FilePath(url.toLocalFile().toStdString()));
}

QUrl WebContextAdapter::cachePath() const {
  base::FilePath path = context_->GetCachePath();
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

  context_->SetCachePath(base::FilePath(url.toLocalFile().toStdString()));
}

QString WebContextAdapter::acceptLangs() const {
  return QString::fromStdString(context_->GetAcceptLangs());
}

void WebContextAdapter::setAcceptLangs(const QString& langs) {
  context_->SetAcceptLangs(langs.toStdString());
}

QList<UserScriptAdapter *>& WebContextAdapter::userScripts() {
  return user_scripts_;
}

void WebContextAdapter::updateUserScripts() {
  context_->UpdateUserScripts();
}

bool WebContextAdapter::isInitialized() const {
  return context_->IsInitialized();
}

WebContextAdapter::CookiePolicy WebContextAdapter::cookiePolicy() const {
  return static_cast<CookiePolicy>(context_->GetCookiePolicy());
}

void WebContextAdapter::setCookiePolicy(CookiePolicy policy) {
  context_->SetCookiePolicy(
      static_cast<net::StaticCookiePolicy::Type>(policy));
}

WebContextAdapter::SessionCookieMode
WebContextAdapter::sessionCookieMode() const {
  return static_cast<SessionCookieMode>(context_->GetSessionCookieMode());
}

void WebContextAdapter::setSessionCookieMode(SessionCookieMode mode) {
  context_->SetSessionCookieMode(
      static_cast<content::CookieStoreConfig::SessionCookieMode>(mode));
}

bool WebContextAdapter::popupBlockerEnabled() const {
  return context_->GetPopupBlockerEnabled();
}

void WebContextAdapter::setPopupBlockerEnabled(bool enabled) {
  context_->SetPopupBlockerEnabled(enabled);
}

bool WebContextAdapter::devtoolsEnabled() const {
  return context_->GetDevtoolsEnabled();
}

void WebContextAdapter::setDevtoolsEnabled(bool enabled) {
  context_->SetDevtoolsEnabled(enabled);
}

int WebContextAdapter::devtoolsPort() const {
  return context_->GetDevtoolsPort();
}

void WebContextAdapter::setDevtoolsPort(int port) {
  context_->SetDevtoolsPort(port);
}

QString WebContextAdapter::devtoolsBindIp() const {
  return QString::fromStdString(context_->GetDevtoolsBindIp());
}

void WebContextAdapter::setDevtoolsBindIp(const QString& ip) {
  context_->SetDevtoolsBindIp(ip.toStdString());
}

int WebContextAdapter::setCookies(const QUrl& url,
                                  const QList<QNetworkCookie>& cookies) {
  return context_->SetCookies(url, cookies);
}

int WebContextAdapter::getCookies(const QUrl& url) {
  return context_->GetCookies(url);
}

int WebContextAdapter::getAllCookies() {
  return context_->GetAllCookies();
}

int WebContextAdapter::deleteAllCookies() {
  return context_->DeleteAllCookies();
}

QStringList WebContextAdapter::hostMappingRules() const {
  std::vector<std::string> v = context_->GetHostMappingRules();

  QStringList rules;
  for (std::vector<std::string>::const_iterator it = v.cbegin();
       it != v.cend(); ++it) {
    rules.append(QString::fromStdString(*it));
  }

  return rules;
}

void WebContextAdapter::setHostMappingRules(const QStringList& rules) {
  std::vector<std::string> v;
  for (QStringList::const_iterator it = rules.cbegin();
       it != rules.cend(); ++it) {
    v.push_back((*it).toStdString());
  }

  context_->SetHostMappingRules(v);
}

void WebContextAdapter::setAllowedExtraUrlSchemes(const QStringList& schemes) {
  std::set<std::string> set;
  for (int i = 0; i < schemes.size(); ++i) {
    set.insert(schemes.at(i).toStdString());
  }

  context_->SetAllowedExtraURLSchemes(set);
}

WebContextAdapter::WebContextAdapter(QObject* q)
    : AdapterBase(q),
      context_(WebContext::Create(this)) {

  context_->AddRef();

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

  COMPILE_ASSERT(
      SessionCookieModeEphemeral == static_cast<SessionCookieMode>(
        content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES),
      session_cookie_mode_enums_ephemeral_doesnt_match);
  COMPILE_ASSERT(
      SessionCookieModePersistent == static_cast<SessionCookieMode>(
        content::CookieStoreConfig::PERSISTANT_SESSION_COOKIES),
      session_cookie_mode_enums_persistent_doesnt_match);
  COMPILE_ASSERT(
      SessionCookieModeRestored == static_cast<SessionCookieMode>(
        content::CookieStoreConfig::RESTORED_SESSION_COOKIES),
      session_cookie_mode_enums_restored_doesnt_match);
}

} // namespace qt
} // namespace oxide
