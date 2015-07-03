// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_PROXY_H_

#include <QPair>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QUrl>
#include <QWeakPointer>

#include "qt/core/glue/oxide_qt_proxy_handle.h"
#include "qt/core/glue/oxide_qt_web_context_proxy_client.h"

QT_BEGIN_NAMESPACE
template <typename T> class QList;
class QNetworkCookie;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class UserScriptProxy;
class WebContext;
class WebContextProxy;
class WebContextProxyClient;

OXIDE_Q_DECL_PROXY_HANDLE(UserScriptProxy);
OXIDE_Q_DECL_PROXY_HANDLE(WebContextProxy);

class Q_DECL_EXPORT WebContextProxy {
  OXIDE_Q_DECL_PROXY_FOR(WebContext);
 public:
  static WebContextProxy* create(WebContextProxyClient* client);

  static void getValidDevtoolsPorts(int* min, int* max);
  static bool checkIPAddress(const QString& address);

  virtual ~WebContextProxy();

  static WebContextProxyHandle* defaultContext();

  enum CookiePolicy {
    CookiePolicyAllowAll,
    CookiePolicyBlockAll,
    CookiePolicyBlockThirdParty
  };

  enum SessionCookieMode {
    SessionCookieModeEphemeral,
    SessionCookieModePersistent,
    SessionCookieModeRestored
  };

  virtual void init(
      const QWeakPointer<WebContextProxyClient::IOClient>& io_client) = 0;
  virtual void makeDefault() = 0;

  virtual QString product() const = 0;
  virtual void setProduct(const QString& product) = 0;

  virtual QString userAgent() const = 0;
  virtual void setUserAgent(const QString& user_agent) = 0;

  virtual QUrl dataPath() const = 0;
  virtual void setDataPath(const QUrl& url) = 0;

  virtual QUrl cachePath() const = 0;
  virtual void setCachePath(const QUrl& url) = 0;

  virtual QString acceptLangs() const = 0;
  virtual void setAcceptLangs(const QString& langs) = 0;

  virtual QList<UserScriptProxyHandle*>& userScripts() = 0;
  virtual void updateUserScripts() = 0;

  virtual bool isInitialized() const = 0;

  virtual CookiePolicy cookiePolicy() const = 0;
  virtual void setCookiePolicy(CookiePolicy policy) = 0;

  virtual SessionCookieMode sessionCookieMode() const = 0;
  virtual void setSessionCookieMode(SessionCookieMode mode) = 0;

  virtual bool popupBlockerEnabled() const = 0;
  virtual void setPopupBlockerEnabled(bool enabled) = 0;

  virtual bool devtoolsEnabled() const = 0;
  virtual void setDevtoolsEnabled(bool enabled) = 0;

  virtual int devtoolsPort() const = 0;
  virtual void setDevtoolsPort(int port) = 0;

  virtual QString devtoolsBindIp() const = 0;
  virtual void setDevtoolsBindIp(const QString& ip) = 0;

  virtual int setCookies(const QUrl& url,
                         const QList<QNetworkCookie>& cookies) = 0;
  virtual int getCookies(const QUrl& url) = 0;
  virtual int getAllCookies() = 0;
  virtual int deleteAllCookies() = 0;

  virtual QStringList hostMappingRules() const = 0;
  virtual void setHostMappingRules(const QStringList& rules) = 0;

  virtual void setAllowedExtraUrlSchemes(const QStringList& schemes) = 0;

  virtual int maxCacheSizeHint() const = 0;
  virtual void setMaxCacheSizeHint(int size) = 0;

  virtual QString defaultAudioCaptureDeviceId() const = 0;
  virtual bool setDefaultAudioCaptureDeviceId(const QString& id) = 0;
  virtual QString defaultVideoCaptureDeviceId() const = 0;
  virtual bool setDefaultVideoCaptureDeviceId(const QString& id) = 0;

  typedef QPair<QString, QString> UserAgentOverride;

  virtual QList<UserAgentOverride> userAgentOverrides() const = 0;
  virtual void setUserAgentOverrides(
      const QList<UserAgentOverride>& overrides) = 0;

  virtual void clearTemporarySavedPermissionStatuses() = 0;

  virtual void setLegacyUserAgentOverrideEnabled(bool enabled) = 0;

  virtual bool doNotTrack() const = 0;
  virtual void setDoNotTrack(bool dnt) = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_PROXY_H_
