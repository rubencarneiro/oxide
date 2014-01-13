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

#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

QT_BEGIN_NAMESPACE
template <typename T> class QList;
class QOpenGLContext;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class UserScriptAdapter;
class WebContextAdapterPrivate;

class Q_DECL_EXPORT WebContextAdapter {
 public:
  virtual ~WebContextAdapter();

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

  bool constructed() const;
  void completeConstruction();

  static QOpenGLContext* sharedGLContext();
  static void setSharedGLContext(QOpenGLContext* context);

 protected:
  WebContextAdapter();

 private:
  friend class WebContextAdapterPrivate;

  QScopedPointer<WebContextAdapterPrivate> priv;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_ADAPTER_H_
