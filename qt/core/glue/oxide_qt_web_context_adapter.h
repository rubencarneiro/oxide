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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_ADAPTER_H_

#include <QString>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_adapter_base.h"

QT_BEGIN_NAMESPACE
template <typename T> class QList;
class QOpenGLContext;
QT_END_NAMESPACE

class OxideQBeforeSendHeadersEvent;
class OxideQBeforeURLRequestEvent;
class OxideQStoragePermissionRequest;

namespace oxide {
namespace qt {

class UserScriptAdapter;
class WebContextAdapterPrivate;

class Q_DECL_EXPORT WebContextAdapter : public AdapterBase {
 public:
  virtual ~WebContextAdapter();

  enum CookiePolicy {
    CookiePolicyAllowAll,
    CookiePolicyBlockAll,
    CookiePolicyBlockThirdParty
  };

  class IOThreadDelegate {
   public:
    virtual ~IOThreadDelegate() {}

    virtual void OnBeforeURLRequest(OxideQBeforeURLRequestEvent* event) = 0;

    virtual void OnBeforeSendHeaders(OxideQBeforeSendHeadersEvent* event) = 0;

    virtual void HandleStoragePermissionRequest(OxideQStoragePermissionRequest* req) = 0;

    virtual bool GetUserAgentOverride(const QUrl& url, QString* user_agent) = 0;
  };

  QString product() const;
  void setProduct(const QString& product);

  QString userAgent() const;
  void setUserAgent(const QString& user_agent);

  QUrl dataPath() const;
  void setDataPath(const QUrl& url);

  QUrl cachePath() const;
  void setCachePath(const QUrl& url);

  QString acceptLangs() const;
  void setAcceptLangs(const QString& langs);

  QList<UserScriptAdapter *>& user_scripts();

  void updateUserScripts();

  bool isInitialized() const;
  void init();

  static QOpenGLContext* sharedGLContext();
  static void setSharedGLContext(QOpenGLContext* context);

  static void ensureChromiumStarted();

  IOThreadDelegate* getIOThreadDelegate() const;

  CookiePolicy cookiePolicy() const;
  void setCookiePolicy(CookiePolicy policy);

  bool popupBlockerEnabled() const;
  void setPopupBlockerEnabled(bool enabled);

 protected:
  WebContextAdapter(QObject* q,
                    IOThreadDelegate* io_delegate);

 private:
  friend class WebContextAdapterPrivate;

  QList<UserScriptAdapter *> user_scripts_;
  // This is a strong-ref. We can't use scoped_refptr here, so we manage
  // it manually
  WebContextAdapterPrivate* priv;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_ADAPTER_H_
