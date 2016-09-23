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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_H_

#include <memory>
#include <QList>
#include <QtGlobal>
#include <QUrl>
#include <QWeakPointer>
#include <set>
#include <string>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "net/cookies/canonical_cookie.h"

#include "qt/core/glue/oxide_qt_web_context_proxy.h"
#include "shared/browser/media/oxide_media_capture_devices_context_client.h"
#include "shared/browser/oxide_browser_context.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace net {
class CookieStore;
}

namespace oxide {
namespace qt {

class SetCookiesContext;
class WebContextProxyClient;

class WebContext;

// Allows a weak handle to WebContext to be passed around on a thread other
// than the UI thread
class WebContextGetter : public base::RefCountedThreadSafe<WebContextGetter> {
 public:
  WebContextGetter(const base::WeakPtr<WebContext>& context);
  ~WebContextGetter();

  WebContext* GetContext() const;

 private:
  base::WeakPtr<WebContext> context_;

  DISALLOW_COPY_AND_ASSIGN(WebContextGetter);
};

class WebContext : public WebContextProxy,
                   public oxide::MediaCaptureDevicesContextClient {
 public:
  WebContext(WebContextProxyClient* client, QObject* handle);
  ~WebContext();

  static WebContext* FromBrowserContext(oxide::BrowserContext* context);

  oxide::BrowserContext* GetContext();

  QNetworkAccessManager* GetCustomNetworkAccessManager();

 private:
  bool IsInitialized() const;

  void UpdateUserScripts();

  void SetCookieCallback(scoped_refptr<SetCookiesContext> ctxt,
                         const QNetworkCookie& cookie,
                         bool success);
  void DeliverSetCookiesResponse(scoped_refptr<SetCookiesContext> ctxt);
  void GetCookiesCallback(int request_id,
                          const net::CookieList& cookies);
  void DeleteCookiesCallback(int request_id, int num_deleted);

  void SetAllowedExtraURLSchemes(const std::set<std::string>& schemes);

  // WebContextProxy implementation
  void init(
      const QWeakPointer<WebContextProxyClient::IOClient>& io_client) override;
  QString product() const override;
  void setProduct(const QString& product) override;
  QString userAgent() const override;
  void setUserAgent(const QString& user_agent) override;
  QUrl dataPath() const override;
  void setDataPath(const QUrl& url) override;
  QUrl cachePath() const override;
  void setCachePath(const QUrl& url) override;
  QString acceptLangs() const override;
  void setAcceptLangs(const QString& langs) override;
  QList<QObject*>& userScripts() override;
  void updateUserScripts() override;
  bool isInitialized() const override;
  CookiePolicy cookiePolicy() const override;
  void setCookiePolicy(CookiePolicy policy) override;
  SessionCookieMode sessionCookieMode() const override;
  void setSessionCookieMode(SessionCookieMode mode) override;
  bool popupBlockerEnabled() const override;
  void setPopupBlockerEnabled(bool enabled) override;
  bool devtoolsEnabled() const override;
  void setDevtoolsEnabled(bool enabled) override;
  int devtoolsPort() const override;
  void setDevtoolsPort(int port) override;
  QString devtoolsBindIp() const override;
  void setDevtoolsBindIp(const QString& ip) override;
  int setCookies(const QUrl& url,
                 const QList<QNetworkCookie>& cookies) override;
  int getCookies(const QUrl& url) override;
  int getAllCookies() override;
  int deleteAllCookies() override;
  QStringList hostMappingRules() const override;
  void setHostMappingRules(const QStringList& rules) override;
  void setAllowedExtraUrlSchemes(const QStringList& schemes) override;
  int maxCacheSizeHint() const override;
  void setMaxCacheSizeHint(int size) override;
  QString defaultAudioCaptureDeviceId() const override;
  bool setDefaultAudioCaptureDeviceId(const QString& id) override;
  QString defaultVideoCaptureDeviceId() const override;
  bool setDefaultVideoCaptureDeviceId(const QString& id) override;
  QList<UserAgentOverride> userAgentOverrides() const override;
  void setUserAgentOverrides(
      const QList<UserAgentOverride>& overrides) override;
  void clearTemporarySavedPermissionStatuses() override;
  void setLegacyUserAgentOverrideEnabled(bool enabled) override;

  bool doNotTrack() const override;
  void setDoNotTrack(bool dnt) override;

  // oxide::MediaCaptureDevicesContextClient implementation
  void DefaultAudioDeviceChanged() override;
  void DefaultVideoDeviceChanged() override;

  WebContextProxyClient* client_;

  BrowserContext::UniquePtr context_;

  struct ConstructProperties;
  std::unique_ptr<ConstructProperties> construct_props_;

  class BrowserContextDelegate;
  scoped_refptr<BrowserContextDelegate> delegate_;

  QList<QObject*> user_scripts_;

  base::WeakPtrFactory<WebContext> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebContext);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_CONTEXT_ADAPTER_P_H_
