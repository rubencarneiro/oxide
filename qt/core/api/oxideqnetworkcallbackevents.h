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

#ifndef OXIDE_Q_NETWORK_CALLBACK_EVENTS
#define OXIDE_Q_NETWORK_CALLBACK_EVENTS

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

class OxideQBeforeSendHeadersEventPrivate;
class OxideQBeforeURLRequestEventPrivate;
class OxideQNetworkCallbackEventPrivate;

class Q_DECL_EXPORT OxideQNetworkCallbackEvent : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool requestCancelled READ requestCancelled)

  Q_DECLARE_PRIVATE(OxideQNetworkCallbackEvent)
  Q_DISABLE_COPY(OxideQNetworkCallbackEvent)

 public:
  virtual ~OxideQNetworkCallbackEvent();

  bool requestCancelled() const;
  Q_INVOKABLE void cancelRequest();

 protected:
  OxideQNetworkCallbackEvent(OxideQNetworkCallbackEventPrivate& dd);

  QScopedPointer<OxideQNetworkCallbackEventPrivate> d_ptr;
};

class Q_DECL_EXPORT OxideQBeforeURLRequestEvent : public OxideQNetworkCallbackEvent {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url WRITE setUrl)

  Q_DECLARE_PRIVATE(OxideQBeforeURLRequestEvent)
  Q_DISABLE_COPY(OxideQBeforeURLRequestEvent)

 public:
  Q_DECL_HIDDEN OxideQBeforeURLRequestEvent();
  virtual ~OxideQBeforeURLRequestEvent();

  QUrl url() const;
  void setUrl(const QUrl& url);
};

class Q_DECL_EXPORT OxideQBeforeSendHeadersEvent : public OxideQNetworkCallbackEvent {
  Q_OBJECT

  Q_DECLARE_PRIVATE(OxideQBeforeSendHeadersEvent)
  Q_DISABLE_COPY(OxideQBeforeSendHeadersEvent)

 public:
  Q_DECL_HIDDEN OxideQBeforeSendHeadersEvent();
  virtual ~OxideQBeforeSendHeadersEvent();

  Q_INVOKABLE bool hasHeader(const QString& header) const;
  Q_INVOKABLE QString getHeader(const QString& header) const;

  Q_INVOKABLE void setHeader(const QString& header, const QString& value);
  Q_INVOKABLE void setHeaderIfMissing(const QString& header, const QString& value);

  Q_INVOKABLE void clearHeaders();
  Q_INVOKABLE void removeHeader(const QString& header);
};

#endif // OXIDE_Q_NETWORK_CALLBACK_EVENTS
