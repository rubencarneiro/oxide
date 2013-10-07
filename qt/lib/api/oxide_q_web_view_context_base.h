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

#ifndef OXIDE_Q_WEB_VIEW_CONTEXT_BASE_H
#define OXIDE_Q_WEB_VIEW_CONTEXT_BASE_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QUrl>

QT_USE_NAMESPACE

namespace oxide {
namespace qt {
class QWebViewContextBasePrivate;
}
}

class Q_DECL_EXPORT OxideQWebViewContextBase : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString product READ product WRITE setProduct NOTIFY productChanged)
  Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
  Q_PROPERTY(QUrl dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)
  Q_PROPERTY(QUrl cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
  Q_PROPERTY(QString acceptLangs READ acceptLangs WRITE setAcceptLangs NOTIFY acceptLangsChanged)

  Q_DECLARE_PRIVATE(oxide::qt::QWebViewContextBase)

 public:
  virtual ~OxideQWebViewContextBase();

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

 Q_SIGNALS:
  void productChanged();
  void userAgentChanged();
  void dataPathChanged();
  void cachePathChanged();
  void acceptLangsChanged();

 protected:
  OxideQWebViewContextBase(oxide::qt::QWebViewContextBasePrivate& dd,
                           QObject* parent = NULL);

  QScopedPointer<oxide::qt::QWebViewContextBasePrivate> d_ptr;

 private Q_SLOTS:
  void scriptUpdated();
};

#endif // OXIDE_Q_WEB_VIEW_CONTEXT_BASE_H
