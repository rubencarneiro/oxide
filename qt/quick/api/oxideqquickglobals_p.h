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

#ifndef _OXIDE_QT_QUICK_API_GLOBALS_P_H_
#define _OXIDE_QT_QUICK_API_GLOBALS_P_H_

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

class OxideQQuickGlobalsPrivate;

class OxideQQuickGlobals : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString product READ product WRITE setProduct NOTIFY productChanged)
  Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
  Q_PROPERTY(QUrl dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)
  Q_PROPERTY(QUrl cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
  Q_PROPERTY(QString acceptLangs READ acceptLangs WRITE setAcceptLangs NOTIFY acceptLangsChanged)

  Q_DECLARE_PRIVATE(OxideQQuickGlobals)
  Q_DISABLE_COPY(OxideQQuickGlobals)

 public:
  static OxideQQuickGlobals* instance();
  virtual ~OxideQQuickGlobals();

  QString product() const;
  void setProduct(const QString& product);

  QString userAgent() const;
  void setUserAgent(const QString& user_agent);

  QUrl dataPath() const;
  void setDataPath(const QUrl& data_path);

  QUrl cachePath() const;
  void setCachePath(const QUrl& cache_path);

  QString acceptLangs() const;
  void setAcceptLangs(const QString& accept_langs);

 Q_SIGNALS:
  void productChanged();
  void userAgentChanged();
  void dataPathChanged();
  void cachePathChanged();
  void acceptLangsChanged();

 private:
  OxideQQuickGlobals();

  Q_PRIVATE_SLOT(d_func(), void defaultContextDestroyed());
  Q_PRIVATE_SLOT(d_func(), void defaultContextProductChanged());
  Q_PRIVATE_SLOT(d_func(), void defaultContextUserAgentChanged());
  Q_PRIVATE_SLOT(d_func(), void defaultContextDataPathChanged());
  Q_PRIVATE_SLOT(d_func(), void defaultContextCachePathChanged());
  Q_PRIVATE_SLOT(d_func(), void defaultContextAcceptLangsChanged());

  QScopedPointer<OxideQQuickGlobalsPrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_GLOBALS_P_H_
