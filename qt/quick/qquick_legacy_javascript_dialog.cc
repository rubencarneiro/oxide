// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "qquick_legacy_javascript_dialog.h"

#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>

#include "qt/core/glue/javascript_dialog_client.h"
#include "qt/quick/api/oxideqquickwebview.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

using qt::JavaScriptDialogClient;

void LegacyJavaScriptDialog::Hide() {
  if (item_) {
    item_->setVisible(false);
  }
}

QString LegacyJavaScriptDialog::GetCurrentPromptText() {
  Q_UNREACHABLE();
}

void LegacyJavaScriptDialog::run(QObject* contextObject) {
  if (!parent_) {
    qWarning() <<
        "LegacyJavaScriptDialog:run: Can't show after the parent item has gone";
    delete contextObject;
    client_->close(false);
    return;
  }

  if (!component_) {
    qWarning() <<
        "LegacyJavaScriptDialog::run: Content requested a javascript dialog, "
        "but the application hasn't provided one";
    delete contextObject;
    client_->close(false);
    return;
  }

  QQmlContext* baseContext = component_->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(parent_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextProperty(QLatin1String("model"), contextObject);
  context_->setContextObject(contextObject);
  contextObject->setParent(context_.data());

  item_.reset(qobject_cast<QQuickItem*>(component_->beginCreate(context_.data())));
  if (!item_) {
    qWarning() <<
        "LegacyJavaScriptDialog::run: Failed to create instance of Qml JS "
        "dialog component";
    context_.reset();
    client_->close(false);
    return;
  }

  OxideQQuickWebView* web_view = qobject_cast<OxideQQuickWebView*>(parent_);
  if (web_view) {
    OxideQQuickWebViewPrivate::get(web_view)
        ->addAttachedPropertyTo(item_.data());
  }

  item_->setParentItem(parent_);
  component_->completeCreate();
}

LegacyJavaScriptDialog::LegacyJavaScriptDialog(QQuickItem* parent,
                                               QQmlComponent* component,
                                               JavaScriptDialogClient* client)
    : parent_(parent),
      component_(component),
      client_(client) {}

} // namespace qquick
} // namespace oxide
