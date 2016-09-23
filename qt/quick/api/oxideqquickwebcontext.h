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

#ifndef OXIDE_QTQUICK_WEB_CONTEXT
#define OXIDE_QTQUICK_WEB_CONTEXT

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>
#include <QtCore/QVariantList>
#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QtQml>

#include <OxideQtQuick/oxideqquickglobal.h>

class OxideQQuickCookieManager;
class OxideQQuickWebContextDelegateWorker;
class OxideQQuickUserScript;
class OxideQQuickWebContextPrivate;

class OXIDE_QTQUICK_EXPORT OxideQQuickWebContext : public QObject,
                                                   public QQmlParserStatus {
  Q_OBJECT
  Q_PROPERTY(QString product READ product WRITE setProduct NOTIFY productChanged)
  Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)

  Q_PROPERTY(QUrl dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)
  Q_PROPERTY(QUrl cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)

  Q_PROPERTY(QString acceptLangs READ acceptLangs WRITE setAcceptLangs NOTIFY acceptLangsChanged)

  Q_PROPERTY(QQmlListProperty<OxideQQuickUserScript> userScripts READ userScripts NOTIFY userScriptsChanged)

  Q_PROPERTY(CookiePolicy cookiePolicy READ cookiePolicy WRITE setCookiePolicy NOTIFY cookiePolicyChanged)
  Q_PROPERTY(SessionCookieMode sessionCookieMode READ sessionCookieMode WRITE setSessionCookieMode NOTIFY sessionCookieModeChanged)

  Q_PROPERTY(bool popupBlockerEnabled READ popupBlockerEnabled WRITE setPopupBlockerEnabled NOTIFY popupBlockerEnabledChanged)

  Q_PROPERTY(OxideQQuickWebContextDelegateWorker* networkRequestDelegate READ networkRequestDelegate WRITE setNetworkRequestDelegate NOTIFY networkRequestDelegateChanged)
  Q_PROPERTY(OxideQQuickWebContextDelegateWorker* storageAccessPermissionDelegate READ storageAccessPermissionDelegate WRITE setStorageAccessPermissionDelegate NOTIFY storageAccessPermissionDelegateChanged)
  Q_PROPERTY(OxideQQuickWebContextDelegateWorker* userAgentOverrideDelegate READ userAgentOverrideDelegate WRITE setUserAgentOverrideDelegate NOTIFY userAgentOverrideDelegateChanged)

  Q_PROPERTY(bool devtoolsEnabled READ devtoolsEnabled WRITE setDevtoolsEnabled NOTIFY devtoolsEnabledChanged)
  Q_PROPERTY(int devtoolsPort READ devtoolsPort WRITE setDevtoolsPort NOTIFY devtoolsPortChanged)
  Q_PROPERTY(QString devtoolsIp READ devtoolsBindIp WRITE setDevtoolsBindIp NOTIFY devtoolsBindIpChanged)

  Q_PROPERTY(OxideQQuickCookieManager* cookieManager READ cookieManager CONSTANT)

  Q_PROPERTY(QStringList hostMappingRules READ hostMappingRules WRITE setHostMappingRules NOTIFY hostMappingRulesChanged REVISION 1)

  Q_PROPERTY(QStringList allowedExtraUrlSchemes READ allowedExtraUrlSchemes WRITE setAllowedExtraUrlSchemes NOTIFY allowedExtraUrlSchemesChanged REVISION 1)

  // maxCacheSizeHint is a soft limit, expressed in MB
  Q_PROPERTY(int maxCacheSizeHint READ maxCacheSizeHint WRITE setMaxCacheSizeHint NOTIFY maxCacheSizeHintChanged REVISION 2)

  Q_PROPERTY(QString defaultAudioCaptureDeviceId READ defaultAudioCaptureDeviceId WRITE setDefaultAudioCaptureDeviceId NOTIFY defaultAudioCaptureDeviceIdChanged REVISION 3)
  Q_PROPERTY(QString defaultVideoCaptureDeviceId READ defaultVideoCaptureDeviceId WRITE setDefaultVideoCaptureDeviceId NOTIFY defaultVideoCaptureDeviceIdChanged REVISION 3)

  Q_PROPERTY(QVariantList userAgentOverrides READ userAgentOverrides WRITE setUserAgentOverrides NOTIFY userAgentOverridesChanged REVISION 3)

  Q_PROPERTY(bool doNotTrackEnabled READ doNotTrack WRITE setDoNotTrack NOTIFY doNotTrackEnabledChanged REVISION 3)

  Q_ENUMS(CookiePolicy)
  Q_ENUMS(SessionCookieMode)

  Q_DECLARE_PRIVATE(OxideQQuickWebContext)
  Q_DISABLE_COPY(OxideQQuickWebContext)

  Q_INTERFACES(QQmlParserStatus)

 public:

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

  OxideQQuickWebContext(QObject* parent = nullptr);
  ~OxideQQuickWebContext() Q_DECL_OVERRIDE;

  static OxideQQuickWebContext* defaultContext(bool create);

  QString product() const;
  void setProduct(const QString& product);

  QString userAgent() const;
  void setUserAgent(const QString& userAgent);

  QUrl dataPath() const;
  void setDataPath(const QUrl& dataUrl);

  QUrl cachePath() const;
  void setCachePath(const QUrl& cacheUrl);

  QString acceptLangs() const;
  void setAcceptLangs(const QString& acceptLangs);

  QQmlListProperty<OxideQQuickUserScript> userScripts();
  Q_INVOKABLE void addUserScript(OxideQQuickUserScript* script);
  Q_INVOKABLE void removeUserScript(OxideQQuickUserScript* script);

  CookiePolicy cookiePolicy() const;
  void setCookiePolicy(CookiePolicy policy);

  SessionCookieMode sessionCookieMode() const;
  void setSessionCookieMode(SessionCookieMode mode);

  bool popupBlockerEnabled() const;
  void setPopupBlockerEnabled(bool enabled);

  OxideQQuickWebContextDelegateWorker* networkRequestDelegate() const;
  void setNetworkRequestDelegate(OxideQQuickWebContextDelegateWorker* delegate);

  OxideQQuickWebContextDelegateWorker* storageAccessPermissionDelegate() const;
  void setStorageAccessPermissionDelegate(OxideQQuickWebContextDelegateWorker* delegate);

  OxideQQuickWebContextDelegateWorker* userAgentOverrideDelegate() const;
  void setUserAgentOverrideDelegate(OxideQQuickWebContextDelegateWorker* delegate);

  bool devtoolsEnabled() const;
  void setDevtoolsEnabled(bool enabled);

  int devtoolsPort() const;
  void setDevtoolsPort(int port);

  QString devtoolsBindIp() const;
  void setDevtoolsBindIp(const QString& bindIp);

  OxideQQuickCookieManager* cookieManager() const;

  QStringList hostMappingRules() const;
  void setHostMappingRules(const QStringList& rules);

  QStringList allowedExtraUrlSchemes() const;
  void setAllowedExtraUrlSchemes(const QStringList& schemes);

  int maxCacheSizeHint() const;
  void setMaxCacheSizeHint(int size);

  QString defaultAudioCaptureDeviceId() const;
  void setDefaultAudioCaptureDeviceId(const QString& id);

  QString defaultVideoCaptureDeviceId() const;
  void setDefaultVideoCaptureDeviceId(const QString& id);

  QVariantList userAgentOverrides() const;
  void setUserAgentOverrides(const QVariantList& overrides);

  bool doNotTrack() const;
  void setDoNotTrack(bool dnt);

 Q_SIGNALS:
  void productChanged();
  void userAgentChanged();
  void dataPathChanged();
  void cachePathChanged();
  void acceptLangsChanged();
  void userScriptsChanged();
  void cookiePolicyChanged();
  void sessionCookieModeChanged();
  void popupBlockerEnabledChanged();
  void networkRequestDelegateChanged();
  void storageAccessPermissionDelegateChanged();
  void userAgentOverrideDelegateChanged();
  void devtoolsEnabledChanged();
  void devtoolsPortChanged();
  void devtoolsBindIpChanged();
  Q_REVISION(1) void hostMappingRulesChanged();
  Q_REVISION(1) void allowedExtraUrlSchemesChanged();
  Q_REVISION(2) void maxCacheSizeHintChanged();
  Q_REVISION(3) void defaultAudioCaptureDeviceIdChanged();
  Q_REVISION(3) void defaultVideoCaptureDeviceIdChanged();
  Q_REVISION(3) void userAgentOverridesChanged();
  Q_REVISION(3) void doNotTrackEnabledChanged();

 protected:
  // QQmlParserStatus implementation
  void classBegin() Q_DECL_OVERRIDE;
  void componentComplete() Q_DECL_OVERRIDE;

 private:
  Q_PRIVATE_SLOT(d_func(), void userScriptUpdated());
  Q_PRIVATE_SLOT(d_func(), void userScriptWillBeDeleted());

  QScopedPointer<OxideQQuickWebContextPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebContext)

#endif // OXIDE_QTQUICK_WEB_CONTEXT
