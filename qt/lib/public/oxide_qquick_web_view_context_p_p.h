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

#ifndef _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_P_H_
#define _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_P_H_

#include <QtGlobal>

#include "oxide_qt_qweb_view_context_p.h"

class OxideQQuickWebViewContext;
class OxideQUserScript;

QT_BEGIN_NAMESPACE
template<typename T> class QQmlListProperty;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class OxideQQuickWebViewContextPrivate FINAL :
    public oxide::qt::QWebViewContextPrivate {
  Q_DECLARE_PUBLIC(OxideQQuickWebViewContext)

 public:
  OxideQQuickWebViewContextPrivate(OxideQQuickWebViewContext* q);
  ~OxideQQuickWebViewContextPrivate();

  static OxideQQuickWebViewContextPrivate* get(
      OxideQQuickWebViewContext* context);

  static void userScript_append(QQmlListProperty<OxideQUserScript>* prop,
                                OxideQUserScript* user_script);
  static int userScript_count(QQmlListProperty<OxideQUserScript>* prop);
  static OxideQUserScript* userScript_at(
      QQmlListProperty<OxideQUserScript>* prop,
      int index);
  static void userScript_clear(QQmlListProperty<OxideQUserScript>* prop);

 private:
  DISALLOW_COPY_AND_ASSIGN(OxideQQuickWebViewContextPrivate);
};

#endif // _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_P_H_
