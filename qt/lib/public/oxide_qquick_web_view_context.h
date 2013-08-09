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

#ifndef _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_H_
#define _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_H_

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QtQml>

#include "shared/common/oxide_export.h"

QT_USE_NAMESPACE

class OxideQQuickWebViewContextPrivate;

namespace oxide {
class BrowserContext;
}

class OXIDE_EXPORT OxideQQuickWebViewContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString product READ product WRITE setProduct NOTIFY productChanged)
  Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
  Q_PROPERTY(QString dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)
  Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
  Q_PROPERTY(QString acceptLangs READ acceptLangs WRITE setAcceptLangs NOTIFY acceptLangsChanged)

  Q_DECLARE_PRIVATE(OxideQQuickWebViewContext)

 public:
  OxideQQuickWebViewContext(QObject* parent = NULL);
  OxideQQuickWebViewContext(bool is_default,
                            QObject* parent = NULL);
  virtual ~OxideQQuickWebViewContext();

  static OxideQQuickWebViewContext* defaultContext();

  QString product() const;
  void setProduct(const QString& product);

  QString userAgent() const;
  void setUserAgent(const QString& user_agent);

  QString dataPath() const;
  void setDataPath(const QString& data_path);

  QString cachePath() const;
  void setCachePath(const QString& cache_path);

  QString acceptLangs() const;
  void setAcceptLangs(const QString& accept_langs);

 Q_SIGNALS:
  void productChanged();
  void userAgentChanged();
  void dataPathChanged();
  void cachePathChanged();
  void acceptLangsChanged();

 private:
  QScopedPointer<OxideQQuickWebViewContextPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebViewContext)

#endif // _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_H_
