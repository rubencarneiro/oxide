// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#include "oxide_qquick_javascript_dialog_delegate.h"

#include <QDebug>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>

#include "qt/quick/api/oxideqquickwebview_p.h"
#include "qt/quick/api/oxideqquickwebview_p_p.h"

namespace oxide {
namespace qquick {

JavaScriptDialogDelegate::JavaScriptDialogDelegate(
    OxideQQuickWebView* webview) :
    web_view_(webview) {}

JavaScriptDialogDelegate::~JavaScriptDialogDelegate() {
  if (!item_.isNull()) {
    item_.take()->deleteLater();
    context_.take()->deleteLater();
  }
}

bool JavaScriptDialogDelegate::show(QObject* contextObject,
                                    QQmlComponent* component) {
  if (!component) {
    qWarning() << "Content requested a javascript dialog, "
                  "but the application hasn't provided one";
    delete contextObject;
    return false;
  }
  QQmlContext* baseContext = component->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(web_view_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextProperty(QLatin1String("model"), contextObject);
  context_->setContextObject(contextObject);
  contextObject->setParent(context_.data());

  item_.reset(qobject_cast<QQuickItem*>(component->beginCreate(context_.data())));
  if (!item_) {
    qWarning() << "Failed to create javascript dialog";
    context_.reset();
    return false;
  }

  OxideQQuickWebViewPrivate::get(web_view_)->addAttachedPropertyTo(item_.data());
  item_->setParentItem(web_view_);
  component->completeCreate();

  return true;
}

} // namespace qquick
} // namespace oxide
