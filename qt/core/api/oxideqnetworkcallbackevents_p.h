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

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/api/oxideqglobal.h"

class OxideQBeforeRedirectEventPrivate;
class OxideQBeforeSendHeadersEventPrivate;
class OxideQBeforeURLRequestEventPrivate;
class OxideQNetworkCallbackEventPrivate;

class OXIDE_QTCORE_EXPORT OxideQNetworkCallbackEvent : public QObject {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(QString method READ method CONSTANT)
  Q_PROPERTY(bool requestCancelled READ requestCancelled)
  Q_PROPERTY(QString referrer READ referrer)
  Q_PROPERTY(bool isMainFrame READ isMainFrame)

  Q_DECLARE_PRIVATE(OxideQNetworkCallbackEvent)
  Q_DISABLE_COPY(OxideQNetworkCallbackEvent)

 public:
  ~OxideQNetworkCallbackEvent() Q_DECL_OVERRIDE;

  QUrl url() const;
  QString method() const;
  QString referrer() const;
  bool isMainFrame() const;
  bool requestCancelled() const;
  Q_INVOKABLE void cancelRequest();

 protected:
  OxideQNetworkCallbackEvent(OxideQNetworkCallbackEventPrivate& dd);

  QScopedPointer<OxideQNetworkCallbackEventPrivate> d_ptr;
};

class OXIDE_QTCORE_EXPORT OxideQBeforeURLRequestEvent
    : public OxideQNetworkCallbackEvent {
  Q_OBJECT

  Q_PROPERTY(QUrl redirectUrl READ redirectUrl WRITE setRedirectUrl)

  Q_DECLARE_PRIVATE(OxideQBeforeURLRequestEvent)
  Q_DISABLE_COPY(OxideQBeforeURLRequestEvent)

 public:
  Q_DECL_HIDDEN OxideQBeforeURLRequestEvent(const QUrl& url,
                                            const QString& method,
					                                  const QString& referrer,
					                                  bool isMainFrame);
  ~OxideQBeforeURLRequestEvent() Q_DECL_OVERRIDE;

  QUrl redirectUrl() const;
  void setRedirectUrl(const QUrl& url);
};

class OXIDE_QTCORE_EXPORT OxideQBeforeSendHeadersEvent
    : public OxideQNetworkCallbackEvent {
  Q_OBJECT

  Q_DECLARE_PRIVATE(OxideQBeforeSendHeadersEvent)
  Q_DISABLE_COPY(OxideQBeforeSendHeadersEvent)

 public:
  Q_DECL_HIDDEN OxideQBeforeSendHeadersEvent(const QUrl& url,
                                             const QString& method,
					                                   const QString& referrer,
                              					     bool isMainFrame);
  ~OxideQBeforeSendHeadersEvent() Q_DECL_OVERRIDE;

  Q_INVOKABLE bool hasHeader(const QString& header) const;
  Q_INVOKABLE QString getHeader(const QString& header) const;

  Q_INVOKABLE void setHeader(const QString& header, const QString& value);
  Q_INVOKABLE void setHeaderIfMissing(const QString& header, const QString& value);

  Q_INVOKABLE void clearHeaders();
  Q_INVOKABLE void removeHeader(const QString& header);
};

class OXIDE_QTCORE_EXPORT OxideQBeforeRedirectEvent
    : public OxideQNetworkCallbackEvent {
  Q_OBJECT

  Q_PROPERTY(QUrl originalUrl READ originalUrl)

  Q_DECLARE_PRIVATE(OxideQBeforeRedirectEvent)
  Q_DISABLE_COPY(OxideQBeforeRedirectEvent)

 public:
  Q_DECL_HIDDEN OxideQBeforeRedirectEvent(
      const QUrl& url,
      const QString& method,
      const QString& referrer,
      bool isMainFrame,
      const QUrl& originalUrl);
  ~OxideQBeforeRedirectEvent() Q_DECL_OVERRIDE;

  QUrl originalUrl() const;
};

#endif // _OXIDE_QT_CORE_API_NETWORK_CALLBACK_EVENTS_P_H_
