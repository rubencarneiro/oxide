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

#include <QQmlListProperty>
#include <QtGlobal>
#include <QtQml>

#include "shared/common/oxide_export.h"

#include "oxide_qt_web_view_context.h"

QT_USE_NAMESPACE

class OxideQQuickWebViewContextPrivate;
class OxideQUserScript;

class OXIDE_EXPORT OxideQQuickWebViewContext : public oxide::qt::WebViewContext {
  Q_OBJECT
  Q_PROPERTY(QQmlListProperty<OxideQUserScript> userScripts READ userScripts)

  Q_DECLARE_PRIVATE(OxideQQuickWebViewContext)

 public:
  OxideQQuickWebViewContext(QObject* parent = NULL);
  virtual ~OxideQQuickWebViewContext();

  static OxideQQuickWebViewContext* defaultContext();

  QQmlListProperty<OxideQUserScript> userScripts();

 private:
  OxideQQuickWebViewContext(bool is_default);
};

QML_DECLARE_TYPE(OxideQQuickWebViewContext)

#endif // _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_H_
