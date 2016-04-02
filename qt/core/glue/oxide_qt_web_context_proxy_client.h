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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_PROXY_CLIENT_H_
#define _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_PROXY_CLIENT_H_

#include <QString>
#include <QtGlobal>

class OxideQBeforeRedirectEvent;
class OxideQBeforeSendHeadersEvent;
class OxideQBeforeURLRequestEvent;

QT_BEGIN_NAMESPACE
template <typename T> class QList;
class QNetworkAccessManager;
class QNetworkCookie;
class QUrl;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class WebContextProxyClient {
 public:
  virtual ~WebContextProxyClient() {}

  virtual void CookiesSet(int request_id,
                          const QList<QNetworkCookie>& failed_cookies) = 0;
  virtual void CookiesRetrieved(int request_id,
                                const QList<QNetworkCookie>& cookies) = 0;
  virtual void CookiesDeleted(int request_id, int num_deleted) = 0;

  virtual QNetworkAccessManager* GetCustomNetworkAccessManager() = 0;

  virtual void DefaultAudioCaptureDeviceChanged() = 0;

  virtual void DefaultVideoCaptureDeviceChanged() = 0;

  class IOClient {
   public:
    virtual ~IOClient() {}

    virtual void OnBeforeURLRequest(OxideQBeforeURLRequestEvent* event) = 0;

    virtual void OnBeforeSendHeaders(OxideQBeforeSendHeadersEvent* event) = 0;

    virtual void OnBeforeRedirect(OxideQBeforeRedirectEvent* event) = 0;

    virtual QString GetUserAgentOverride(const QUrl& url) = 0;
  };
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_PROXY_CLIENT_H_
