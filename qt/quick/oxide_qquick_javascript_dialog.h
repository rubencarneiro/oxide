// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_JAVASCRIPT_DIALOG_H_
#define _OXIDE_QT_QUICK_JAVASCRIPT_DIALOG_H_

#include <QObject>
#include <QPointer>
#include <QQmlContext>
#include <QQuickItem>
#include <QScopedPointer>

#include "qt/core/glue/oxide_qt_javascript_dialog_proxy.h"

class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class JavaScriptDialogProxyClient;
}

namespace qquick {

class JavaScriptDialog : public oxide::qt::JavaScriptDialogProxy {
 public:
  JavaScriptDialog(OxideQQuickWebView* view,
                   oxide::qt::JavaScriptDialogProxyClient* client);

 protected:
  // oxide::qt::JavaScriptDialogProxy implementation
  void Hide() override;
  void Handle(bool accept, const QString& prompt_override) override;

  // takes ownership of contextObject
  bool run(QObject* contextObject, QQmlComponent* component);

  QPointer<OxideQQuickWebView> view_;
  oxide::qt::JavaScriptDialogProxyClient* client_;

  QScopedPointer<QQmlContext> context_;
  QScopedPointer<QQuickItem> item_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_JAVASCRIPT_DIALOG_H_
