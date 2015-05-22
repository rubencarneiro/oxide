// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_qquick_web_context_menu.h"

#include <QPointF>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QtDebug>
#include <QUrl>

#include "qt/core/glue/oxide_qt_web_context_menu_proxy_client.h"
#include "qt/quick/api/oxideqquickwebview_p.h"
#include "qt/quick/api/oxideqquickwebview_p_p.h"

namespace oxide {
namespace qquick {

namespace {

class ContextMenuContext : public QObject {
  Q_OBJECT

  Q_PROPERTY(QPointF position READ position CONSTANT FINAL)
  Q_PROPERTY(QUrl linkUrl READ linkUrl CONSTANT FINAL)
  Q_PROPERTY(QString linkText READ linkText CONSTANT FINAL)
  Q_PROPERTY(QUrl unfilteredLinkUrl READ unfilteredLinkUrl CONSTANT FINAL)
  Q_PROPERTY(QUrl srcUrl READ srcUrl CONSTANT FINAL)
  Q_PROPERTY(bool hasImageContents READ hasImageContents CONSTANT FINAL)
  Q_PROPERTY(QUrl pageUrl READ pageUrl CONSTANT FINAL)
  Q_PROPERTY(QUrl frameUrl READ frameUrl CONSTANT FINAL)
  Q_PROPERTY(QString selectionText READ selectionText CONSTANT FINAL)
  Q_PROPERTY(bool isEditable READ isEditable CONSTANT FINAL)
  Q_PROPERTY(bool canUndo READ canUndo CONSTANT FINAL)
  Q_PROPERTY(bool canRedo READ canRedo CONSTANT FINAL)
  Q_PROPERTY(bool canCut READ canCut CONSTANT FINAL)
  Q_PROPERTY(bool canCopy READ canCopy CONSTANT FINAL)
  Q_PROPERTY(bool canPaste READ canPaste CONSTANT FINAL)
  Q_PROPERTY(bool canErase READ canErase CONSTANT FINAL)
  Q_PROPERTY(bool canSelectAll READ canSelectAll CONSTANT FINAL)

 public:
  virtual ~ContextMenuContext() {}
  ContextMenuContext(oxide::qt::WebContextMenuProxyClient* client);

  QPointF position() const;
  QUrl linkUrl() const;
  QString linkText() const;
  QUrl unfilteredLinkUrl() const;
  QUrl srcUrl() const;
  bool hasImageContents() const;
  QUrl pageUrl() const;
  QUrl frameUrl() const;
  QString selectionText() const;
  bool isEditable() const;

  bool canUndo() const;
  bool canRedo() const;
  bool canCut() const;
  bool canCopy() const;
  bool canPaste() const;
  bool canErase() const;
  bool canSelectAll() const;

  Q_INVOKABLE void undo() const;
  Q_INVOKABLE void redo() const;
  Q_INVOKABLE void cut() const;
  Q_INVOKABLE void copy() const;
  Q_INVOKABLE void paste() const;
  Q_INVOKABLE void erase() const;
  Q_INVOKABLE void selectAll() const;
  Q_INVOKABLE void saveLink() const;
  Q_INVOKABLE void saveImage() const;

  Q_INVOKABLE void close();

 private:
  oxide::qt::WebContextMenuProxyClient* client_;
};

ContextMenuContext::ContextMenuContext(
    oxide::qt::WebContextMenuProxyClient* client) :
    client_(client) {}

QPointF ContextMenuContext::position() const {
  return client_->position();
}

QUrl ContextMenuContext::linkUrl() const {
  return client_->linkUrl();
}

QString ContextMenuContext::linkText() const {
  return client_->linkText();
}

QUrl ContextMenuContext::unfilteredLinkUrl() const {
  return client_->unfilteredLinkUrl();
}

QUrl ContextMenuContext::srcUrl() const {
  return client_->srcUrl();
}

bool ContextMenuContext::hasImageContents() const {
  return client_->hasImageContents();
}

QUrl ContextMenuContext::pageUrl() const {
  return client_->pageUrl();
}

QUrl ContextMenuContext::frameUrl() const {
  return client_->frameUrl();
}

QString ContextMenuContext::selectionText() const {
  return client_->selectionText();
}

bool ContextMenuContext::isEditable() const {
  return client_->isEditable();
}

bool ContextMenuContext::canUndo() const {
  return client_->canUndo();
}

void ContextMenuContext::undo() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::undo(): not editable";
  } else if (!canUndo()) {
    qWarning() << "ContextMenuContext::undo(): cannot undo";
  } else {
    client_->undo();
  }
}

bool ContextMenuContext::canRedo() const {
  return client_->canRedo();
}

void ContextMenuContext::redo() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::redo(): not editable";
  } else if (!canRedo()) {
    qWarning() << "ContextMenuContext::redo(): cannot redo";
  } else {
    client_->redo();
  }
}

bool ContextMenuContext::canCut() const {
  return client_->canCut();
}

void ContextMenuContext::cut() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::cut(): not editable";
  } else if (!canCut()) {
    qWarning() << "ContextMenuContext::cut(): cannot cut";
  } else {
    client_->cut();
  }
}

bool ContextMenuContext::canCopy() const {
  return client_->canCopy();
}

void ContextMenuContext::copy() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::copy(): not editable";
  } else if (!canCopy()) {
    qWarning() << "ContextMenuContext::copy(): cannot copy";
  } else {
    client_->copy();
  }
}

bool ContextMenuContext::canPaste() const {
  return client_->canPaste();
}

void ContextMenuContext::paste() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::paste(): not editable";
  } else if (!canPaste()) {
    qWarning() << "ContextMenuContext::paste(): cannot paste";
  } else {
    client_->paste();
  }
}

bool ContextMenuContext::canErase() const {
  return client_->canErase();
}

void ContextMenuContext::erase() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::erase(): not editable";
  } else if (!canErase()) {
    qWarning() << "ContextMenuContext::erase(): cannot erase";
  } else {
    client_->erase();
  }
}

bool ContextMenuContext::canSelectAll() const {
  return client_->canSelectAll();
}

void ContextMenuContext::selectAll() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::selectAll(): not editable";
  } else if (!canSelectAll()) {
    qWarning() << "ContextMenuContext::selectAll(): cannot select all";
  } else {
    client_->selectAll();
  }
}

void ContextMenuContext::saveLink() const {
  if (linkUrl().isValid()) {
    client_->saveLink();
  } else {
    qWarning() << "ContextMenuContext::saveLink(): not a valid link";
  }
}

void ContextMenuContext::saveImage() const {
  if (hasImageContents()) {
    client_->saveImage();
  } else {
    qWarning() << "ContextMenuContext::saveImage(): not a valid image";
  }
}

void ContextMenuContext::close() {
  client_->cancel();
}

} // namespace

void WebContextMenu::Show() {
  if (!view_) {
    qWarning() << "WebContextMenu::Show: Can't show after the view has gone";
    client_->cancel();
    return;
  }

  QQmlComponent* component = view_->contextMenu();
  if (!component) {
    qWarning() <<
        "WebContextMenu::Show: Content requested a context menu, but the "
        "application hasn't provided one";
    client_->cancel();
    return;
  }

  ContextMenuContext* contextObject = new ContextMenuContext(client_);

  QQmlContext* baseContext = component->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(view_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextProperty(QLatin1String("model"), contextObject);
  context_->setContextObject(contextObject);
  contextObject->setParent(context_.data());

  item_.reset(qobject_cast<QQuickItem*>(
      component->beginCreate(context_.data())));
  if (!item_) {
    qWarning() <<
        "WebContextMenu::Show: Failed to create instance of Qml context menu component";
    client_->cancel();
    return;
  }

  OxideQQuickWebViewPrivate::get(view_)->addAttachedPropertyTo(item_.data());
  item_->setParentItem(view_);

  component->completeCreate();
}

void WebContextMenu::Hide() {
  if (item_) {
    item_->setVisible(false);
  }
}

WebContextMenu::WebContextMenu(OxideQQuickWebView* view,
                               oxide::qt::WebContextMenuProxyClient* client)
    : client_(client),
      view_(view) {}

WebContextMenu::~WebContextMenu() {}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_web_context_menu.moc"
