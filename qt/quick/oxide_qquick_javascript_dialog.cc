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

#include "oxide_qquick_javascript_dialog.h"

#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>

#include "qt/core/glue/oxide_qt_javascript_dialog_proxy_client.h"
#include "qt/quick/api/oxideqquickwebview_p.h"
#include "qt/quick/api/oxideqquickwebview_p_p.h"

namespace oxide {
namespace qquick {

void JavaScriptDialog::Hide() {
  if (item_) {
    item_->setVisible(false);
  }
}

void JavaScriptDialog::Handle(bool accept, const QString& prompt_override) {
  Q_UNUSED(prompt_override);
  client_->close(accept);
}

bool JavaScriptDialog::run(QObject* contextObject,
                           QQmlComponent* component) {
  if (!component) {
    qWarning() <<
        "JavaScriptDialog::run: Content requested a javascript dialog, but "
        "the application hasn't provided one";
    delete contextObject;
    return false;
  }

  QQmlContext* baseContext = component->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(view_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextProperty(QLatin1String("model"), contextObject);
  context_->setContextObject(contextObject);
  contextObject->setParent(context_.data());

  item_.reset(qobject_cast<QQuickItem*>(component->beginCreate(context_.data())));
  if (!item_) {
    qWarning() <<
        "JavaScriptDialog::run: Failed to create instance of Qml JS dialog "
        "component";
    context_.reset();
    return false;
  }

  OxideQQuickWebViewPrivate::get(view_)->addAttachedPropertyTo(item_.data());
  item_->setParentItem(view_);
  component->completeCreate();

  return true;
}

JavaScriptDialog::JavaScriptDialog(
    OxideQQuickWebView* view,
    oxide::qt::JavaScriptDialogProxyClient* client)
    : view_(view),
      client_(client) {}

} // namespace qquick
} // namespace oxide
