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

#ifndef _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_H_
#define _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_H_

#include <QObject>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <QUrl>

QT_USE_NAMESPACE

class OxideQQuickCookieManager;
class OxideQQuickWebContextDelegateWorker;
class OxideQQuickUserScript;
class OxideQQuickWebContextPrivate;

class OxideQQuickWebContext : public QObject,
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

  OxideQQuickWebContext(QObject* parent = NULL);
  virtual ~OxideQQuickWebContext();

  void classBegin();
  void componentComplete();

  static OxideQQuickWebContext* defaultContext(bool create);

  QString product() const;
  void setProduct(const QString& product);

  QString userAgent() const;
  void setUserAgent(const QString& user_agent);

  QUrl dataPath() const;
  void setDataPath(const QUrl& data_url);

  QUrl cachePath() const;
  void setCachePath(const QUrl& cache_url);

  QString acceptLangs() const;
  void setAcceptLangs(const QString& accept_langs);

  QQmlListProperty<OxideQQuickUserScript> userScripts();
  Q_INVOKABLE void addUserScript(OxideQQuickUserScript* user_script);
  Q_INVOKABLE void removeUserScript(OxideQQuickUserScript* user_script);

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

  OxideQQuickCookieManager* cookieManager() const;

  QString devtoolsBindIp() const;
  void setDevtoolsBindIp(const QString& bindIp);

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

 private:
  Q_PRIVATE_SLOT(d_func(), void userScriptUpdated());
  Q_PRIVATE_SLOT(d_func(), void userScriptWillBeDeleted());

 private:
  QScopedPointer<OxideQQuickWebContextPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebContext)

#endif // _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_H_
