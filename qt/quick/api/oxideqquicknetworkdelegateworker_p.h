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

#ifndef _OXIDE_QT_QUICK_API_NETWORK_DELEGATE_WORKER_P_H_
#define _OXIDE_QT_QUICK_API_NETWORK_DELEGATE_WORKER_P_H_

#include <QObject>
#include <QQmlParserStatus>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QString;
class QVariant;
QT_END_NAMESPACE

class OxideQQuickNetworkDelegateWorkerPrivate;

class OxideQQuickNetworkDelegateWorker : public QObject,
                                         public QQmlParserStatus {
  Q_OBJECT

  Q_PROPERTY(QUrl source READ source WRITE setSource)

  Q_DECLARE_PRIVATE(OxideQQuickNetworkDelegateWorker)
  Q_DISABLE_COPY(OxideQQuickNetworkDelegateWorker)

  Q_INTERFACES(QQmlParserStatus)

 public:
  OxideQQuickNetworkDelegateWorker();
  virtual ~OxideQQuickNetworkDelegateWorker();

  virtual void classBegin();
  virtual void componentComplete();

  QUrl source() const;
  void setSource(const QUrl& url);

 public Q_SLOTS:
  void sendMessage(const QVariant& message);

 Q_SIGNALS:
  void message(const QVariant& message);
  void error(const QString& error);

 private:
  QScopedPointer<OxideQQuickNetworkDelegateWorkerPrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_NETWORK_DELEGATE_WORKER_P_H_
