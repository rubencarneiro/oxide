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

#ifndef _OXIDE_QT_QUICK_JAVASCRIPT_DIALOG_DELEGATE_H_
#define _OXIDE_QT_QUICK_JAVASCRIPT_DIALOG_DELEGATE_H_

#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QScopedPointer>

class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {

class OxideQQuickJavaScriptDialogDelegate : public QObject {
 public:
  OxideQQuickJavaScriptDialogDelegate(OxideQQuickWebView* webview,
                                      QQmlComponent* component);

  virtual bool Handle(bool accept, const QString& prompt_override) = 0;

 protected:
  // takes ownership of contextObject
  bool show(QObject* contextObject);

  bool isShown() const;

  OxideQQuickWebView* web_view_;
  QQmlComponent* component_;

  QScopedPointer<QQmlContext> context_;
  QScopedPointer<QQuickItem> item_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_JAVASCRIPT_DIALOG_DELEGATE_H_
