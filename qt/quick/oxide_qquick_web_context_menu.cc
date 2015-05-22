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

  Q_PROPERTY(OxideQQuickWebView::MediaType mediaType READ mediaType CONSTANT FINAL)
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
  Q_PROPERTY(OxideQQuickWebView::EditCapabilities editFlags READ editFlags CONSTANT FINAL)

 public:
  virtual ~ContextMenuContext() {}
  ContextMenuContext(oxide::qt::WebContextMenuProxyClient* client);

  OxideQQuickWebView::MediaType mediaType() const;
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
  OxideQQuickWebView::EditCapabilities editFlags() const;

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

OxideQQuickWebView::MediaType ContextMenuContext::mediaType() const {
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaTypeNone ==
        static_cast<OxideQQuickWebView::MediaType>(oxide::qt::MEDIA_TYPE_NONE));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaTypeImage ==
        static_cast<OxideQQuickWebView::MediaType>(oxide::qt::MEDIA_TYPE_IMAGE));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaTypeVideo ==
        static_cast<OxideQQuickWebView::MediaType>(oxide::qt::MEDIA_TYPE_VIDEO));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaTypeAudio ==
        static_cast<OxideQQuickWebView::MediaType>(oxide::qt::MEDIA_TYPE_AUDIO));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaTypeCanvas ==
        static_cast<OxideQQuickWebView::MediaType>(oxide::qt::MEDIA_TYPE_CANVAS));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaTypePlugin ==
        static_cast<OxideQQuickWebView::MediaType>(oxide::qt::MEDIA_TYPE_PLUGIN));

  return static_cast<OxideQQuickWebView::MediaType>(client_->mediaType());
}

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

OxideQQuickWebView::EditCapabilities ContextMenuContext::editFlags() const {
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CanDoNone ==
        static_cast<OxideQQuickWebView::EditFlags>(oxide::qt::EDIT_CAN_DO_NONE));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CanUndo ==
        static_cast<OxideQQuickWebView::EditFlags>(oxide::qt::EDIT_CAN_UNDO));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CanRedo ==
        static_cast<OxideQQuickWebView::EditFlags>(oxide::qt::EDIT_CAN_REDO));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CanCut ==
        static_cast<OxideQQuickWebView::EditFlags>(oxide::qt::EDIT_CAN_CUT));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CanCopy ==
        static_cast<OxideQQuickWebView::EditFlags>(oxide::qt::EDIT_CAN_COPY));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CanPaste ==
        static_cast<OxideQQuickWebView::EditFlags>(oxide::qt::EDIT_CAN_PASTE));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CanErase ==
        static_cast<OxideQQuickWebView::EditFlags>(oxide::qt::EDIT_CAN_ERASE));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CanSelectAll ==
        static_cast<OxideQQuickWebView::EditFlags>(oxide::qt::EDIT_CAN_SELECT_ALL));

  return static_cast<OxideQQuickWebView::EditCapabilities>(client_->editFlags());
}

void ContextMenuContext::undo() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::undo(): not editable";
  } else if (!editFlags().testFlag(OxideQQuickWebView::CanUndo)) {
    qWarning() << "ContextMenuContext::undo(): cannot undo";
  } else {
    client_->undo();
  }
}

void ContextMenuContext::redo() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::redo(): not editable";
  } else if (!editFlags().testFlag(OxideQQuickWebView::CanRedo)) {
    qWarning() << "ContextMenuContext::redo(): cannot redo";
  } else {
    client_->redo();
  }
}

void ContextMenuContext::cut() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::cut(): not editable";
  } else if (!editFlags().testFlag(OxideQQuickWebView::CanCut)) {
    qWarning() << "ContextMenuContext::cut(): cannot cut";
  } else {
    client_->cut();
  }
}

void ContextMenuContext::copy() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::copy(): not editable";
  } else if (!editFlags().testFlag(OxideQQuickWebView::CanCopy)) {
    qWarning() << "ContextMenuContext::copy(): cannot copy";
  } else {
    client_->copy();
  }
}

void ContextMenuContext::paste() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::paste(): not editable";
  } else if (!editFlags().testFlag(OxideQQuickWebView::CanPaste)) {
    qWarning() << "ContextMenuContext::paste(): cannot paste";
  } else {
    client_->paste();
  }
}

void ContextMenuContext::erase() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::erase(): not editable";
  } else if (!editFlags().testFlag(OxideQQuickWebView::CanErase)) {
    qWarning() << "ContextMenuContext::erase(): cannot erase";
  } else {
    client_->erase();
  }
}

void ContextMenuContext::selectAll() const {
  if (!isEditable()) {
    qWarning() << "ContextMenuContext::selectAll(): not editable";
  } else if (!editFlags().testFlag(OxideQQuickWebView::CanSelectAll)) {
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
