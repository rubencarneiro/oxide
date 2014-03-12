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

#ifndef _OXIDE_QT_CORE_API_NETWORK_CALLBACK_EVENTS_P_H_
#define _OXIDE_QT_CORE_API_NETWORK_CALLBACK_EVENTS_P_H_

#include <QtGlobal>
#include <QUrl>

class GURL;

namespace net {
class HttpRequestHeaders;
}

class OxideQNetworkCallbackEvent;
class OxideQBeforeSendHeadersEvent;
class OxideQBeforeURLRequestEvent;

class OxideQNetworkCallbackEventPrivate {
 public:
  virtual ~OxideQNetworkCallbackEventPrivate();

  bool *request_cancelled;

 protected:
  OxideQNetworkCallbackEventPrivate();
};

class OxideQBeforeURLRequestEventPrivate Q_DECL_FINAL :
    public OxideQNetworkCallbackEventPrivate {
 public:
  ~OxideQBeforeURLRequestEventPrivate();

  static OxideQBeforeURLRequestEventPrivate* get(OxideQBeforeURLRequestEvent* q);

  QUrl current_url;
  GURL* new_url;

 private:
  friend class OxideQBeforeURLRequestEvent;

  OxideQBeforeURLRequestEventPrivate();
};

class OxideQBeforeSendHeadersEventPrivate Q_DECL_FINAL :
    public OxideQNetworkCallbackEventPrivate {
 public:
  ~OxideQBeforeSendHeadersEventPrivate();

  static OxideQBeforeSendHeadersEventPrivate* get(OxideQBeforeSendHeadersEvent* q);

  net::HttpRequestHeaders* headers;

 private:
  friend class OxideQBeforeSendHeadersEvent;

  OxideQBeforeSendHeadersEventPrivate();
};

#endif // _OXIDE_QT_CORE_API_NETWORK_CALLBACK_EVENTS_P_H_
