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

#include "uitk_javascript_dialog.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QtDebug>

#include "qt/core/glue/javascript_dialog_client.h"

namespace oxide {
namespace uitk {

using qt::JavaScriptDialogClient;
using qt::JavaScriptDialogType;

namespace {

QString TypeToString(JavaScriptDialogType type) {
  switch (type) {
    case JavaScriptDialogType::Alert:
      return "alert";
    case JavaScriptDialogType::Confirm:
      return "confirm";
    case JavaScriptDialogType::Prompt:
      return "prompt";
  }

  Q_UNREACHABLE();
}

}

void JavaScriptDialog::OnResponse(bool success, const QString& user_input) {
  client_->close(success, user_input);
}

JavaScriptDialog::JavaScriptDialog(JavaScriptDialogClient* client)
    : client_(client) {}

bool JavaScriptDialog::Init(QQuickItem* parent,
                            JavaScriptDialogType type,
                            const QString& message_text,
                            const QString& default_prompt_text) {
  QQmlEngine* engine = qmlEngine(parent);
  if (!engine) {
    qWarning() <<
        "uitk::JavaScriptDialog: Failed to create JS dialog - cannot determine "
        "QQmlEngine for parent item";
    return false;
  }

  QQmlComponent component(engine);
  component.loadUrl(QUrl("qrc:///JavaScriptDialog.qml"));

  if (component.isError()) {
    qCritical() <<
        "uitk::JavaScriptDialog: Failed to initialize JS dialog component "
        "because of the following errors: ";
    for (const auto& error : component.errors()) {
      qCritical() << error;
    }
    return false;
  }

  Q_ASSERT(component.isReady());

  QObject* dialog = component.beginCreate(engine->rootContext());
  if (!dialog) {
    qCritical() <<
        "uitk::JavaScriptDialog: Failed to create JS dialog instance";
    return false;
  }

  item_.reset(qobject_cast<QQuickItem*>(dialog));
  if (!item_) {
    qCritical() <<
        "uitk::JavaScriptDialog: JS dialog instance is not a QQuickItem";
    delete dialog;
    return false;
  }

  item_->setProperty("type", TypeToString(type));
  item_->setProperty("text", message_text);
  item_->setProperty("defaultPromptValue", default_prompt_text);
  item_->setProperty("opener", QVariant::fromValue(parent));
  item_->setParentItem(parent);

  component.completeCreate();

  connect(item_.get(), SIGNAL(response(bool, const QString&)),
          this, SLOT(OnResponse(bool, const QString&)));

  return true;
}

void JavaScriptDialog::Show() {
  QMetaObject::invokeMethod(item_.get(), "show");
}

void JavaScriptDialog::Hide() {
  QMetaObject::invokeMethod(item_.get(), "hide");
}

QString JavaScriptDialog::GetCurrentPromptText() {
  return item_->property("currentPromptValue").toString();
}

JavaScriptDialog::~JavaScriptDialog() = default;

// static
std::unique_ptr<JavaScriptDialog> JavaScriptDialog::Create(
    QQuickItem* parent,
    JavaScriptDialogType type,
    const QString& message_text,
    const QString& default_prompt_text,
    JavaScriptDialogClient* client) {
  std::unique_ptr<JavaScriptDialog> dialog(new JavaScriptDialog(client));
  if (!dialog->Init(parent, type, message_text, default_prompt_text)) {
    return nullptr;
  }
  return std::move(dialog);
}

} // namespace uitk
} // namespace oxide
