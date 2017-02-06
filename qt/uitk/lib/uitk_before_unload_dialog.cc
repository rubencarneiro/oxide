// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "uitk_before_unload_dialog.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QtDebug>

#include "qt/core/glue/javascript_dialog_client.h"

namespace oxide {
namespace uitk {

using qt::JavaScriptDialogClient;

void BeforeUnloadDialog::OnResponse(bool proceed) {
  client_->close(proceed);
}

BeforeUnloadDialog::BeforeUnloadDialog(JavaScriptDialogClient* client)
    : client_(client) {}

bool BeforeUnloadDialog::Init(QQuickItem* parent) {
  QQmlEngine* engine = qmlEngine(parent);
  if (!engine) {
    qWarning() <<
        "uitk::BeforeUnloadDialog: Failed to create beforeunload dialog - "
        "cannot determine QQmlEngine for parent item";
    return false;
  }

  QQmlComponent component(engine);
  component.loadUrl(QUrl("qrc:///BeforeUnloadDialog.qml"));

  if (component.isError()) {
    qCritical() <<
        "uitk::BeforeUnloadDialog: Failed to initialize beforeunload dialog "
        "component because of the following errors: ";
    for (const auto& error : component.errors()) {
      qCritical() << error;
    }
    return false;
  }

  Q_ASSERT(component.isReady());

  QObject* dialog = component.beginCreate(engine->rootContext());
  if (!dialog) {
    qCritical() <<
        "uitk::BeforeUnloadDialog: Failed to create beforeunload dialog "
        "instance";
    return false;
  }

  item_.reset(qobject_cast<QQuickItem*>(dialog));
  if (!item_) {
    qCritical() <<
        "uitk::BeforeUnloadDialog: beforeunload dialog instance is not a "
        "QQuickItem";
    delete dialog;
    return false;
  }

  item_->setProperty("opener", QVariant::fromValue(parent));
  item_->setParentItem(parent);

  component.completeCreate();

  connect(item_.get(), SIGNAL(response(bool)), this, SLOT(OnResponse(bool)));

  return true;
}

void BeforeUnloadDialog::Show() {
  QMetaObject::invokeMethod(item_.get(), "show");
}

void BeforeUnloadDialog::Hide() {
  QMetaObject::invokeMethod(item_.get(), "hide");
}

QString BeforeUnloadDialog::GetCurrentPromptText() {
  return QString();
}

BeforeUnloadDialog::~BeforeUnloadDialog() = default;

// static
std::unique_ptr<BeforeUnloadDialog> BeforeUnloadDialog::Create(
    QQuickItem* parent,
    JavaScriptDialogClient* client) {
  std::unique_ptr<BeforeUnloadDialog> dialog(new BeforeUnloadDialog(client));
  if (!dialog->Init(parent)) {
    return nullptr;
  }
  return std::move(dialog);
}

} // namespace uitk
} // namespace oxide
