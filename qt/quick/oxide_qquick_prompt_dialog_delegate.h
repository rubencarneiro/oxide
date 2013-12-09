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

#ifndef _OXIDE_QT_QUICK_PROMPT_DIALOG_DELEGATE_H_
#define _OXIDE_QT_QUICK_PROMPT_DIALOG_DELEGATE_H_

#include <QScopedPointer>

class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
class QQmlContext;
class QString;
class QQuickItem;
class QUrl;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class JavaScriptDialogClosedCallback;
} // namespace qt

namespace qquick {

class OxideQQuickPromptDialogDelegate Q_DECL_FINAL {
 public:
  OxideQQuickPromptDialogDelegate(OxideQQuickWebView* webview);

  QQmlComponent* component() const;
  void setComponent(QQmlComponent* component);

  void Show(const QUrl& origin_url,
            const QString& accept_lang,
            const QString& message_text,
            const QString& default_prompt_text,
            oxide::qt::JavaScriptDialogClosedCallback* callback,
            bool* did_suppress_message);

  void Hide();

 private:
  OxideQQuickWebView* web_view_;
  QQmlComponent* component_;

  QScopedPointer<QQuickItem> item_;
  QScopedPointer<QQmlContext> context_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_PROMPT_DIALOG_DELEGATE_H_
