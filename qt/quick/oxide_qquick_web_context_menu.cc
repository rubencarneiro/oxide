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
  Q_PROPERTY(OxideQQuickWebView::MediaStatus mediaFlags READ mediaFlags CONSTANT FINAL)

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
  OxideQQuickWebView::MediaStatus mediaFlags() const;

  Q_INVOKABLE void saveLink() const;
  Q_INVOKABLE void saveMedia() const;

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
      OxideQQuickWebView::NoCapability ==
        static_cast<OxideQQuickWebView::EditCapabilityFlags>(oxide::qt::NO_CAPABILITY));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::UndoCapability ==
        static_cast<OxideQQuickWebView::EditCapabilityFlags>(oxide::qt::UNDO_CAPABILITY));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::RedoCapability ==
        static_cast<OxideQQuickWebView::EditCapabilityFlags>(oxide::qt::REDO_CAPABILITY));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CutCapability ==
        static_cast<OxideQQuickWebView::EditCapabilityFlags>(oxide::qt::CUT_CAPABILITY));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::CopyCapability ==
        static_cast<OxideQQuickWebView::EditCapabilityFlags>(oxide::qt::COPY_CAPABILITY));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::PasteCapability ==
        static_cast<OxideQQuickWebView::EditCapabilityFlags>(oxide::qt::PASTE_CAPABILITY));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::EraseCapability ==
        static_cast<OxideQQuickWebView::EditCapabilityFlags>(oxide::qt::ERASE_CAPABILITY));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::SelectAllCapability ==
        static_cast<OxideQQuickWebView::EditCapabilityFlags>(oxide::qt::SELECT_ALL_CAPABILITY));

  return static_cast<OxideQQuickWebView::EditCapabilities>(client_->editFlags());
}

OxideQQuickWebView::MediaStatus ContextMenuContext::mediaFlags() const {
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaNone ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_NONE));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaInError ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_IN_ERROR));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaPaused ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_PAUSED));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaMuted ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_MUTED));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaLoop ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_LOOP));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaCanSave ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_CAN_SAVE));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaHasAudio ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_HAS_AUDIO));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaCanToggleControls ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_CAN_TOGGLE_CONTROLS));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaControls ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_CONTROLS));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaCanPrint ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_CAN_PRINT));
  Q_STATIC_ASSERT(
      OxideQQuickWebView::MediaCanRotate ==
        static_cast<OxideQQuickWebView::MediaFlags>(oxide::qt::MEDIA_CAN_ROTATE));

  return static_cast<OxideQQuickWebView::MediaStatus>(client_->mediaFlags());
}

void ContextMenuContext::saveLink() const {
  if (linkUrl().isValid()) {
    client_->saveLink();
  } else {
    qWarning() << "ContextMenuContext::saveLink(): not a valid link";
  }
}

void ContextMenuContext::saveMedia() const {
  OxideQQuickWebView::MediaType media_type = mediaType();
  if ((media_type == OxideQQuickWebView::MediaTypeImage) ||
      (media_type == OxideQQuickWebView::MediaTypeCanvas)) {
    if (!hasImageContents()) {
      qWarning() << "ContextMenuContext::saveMedia(): image has no contents";
      return;
    }
  } else if ((media_type == OxideQQuickWebView::MediaTypeVideo) ||
      (media_type == OxideQQuickWebView::MediaTypeAudio)) {
    if (!mediaFlags().testFlag(OxideQQuickWebView::MediaCanSave)) {
      qWarning() << "ContextMenuContext::saveMedia(): cannot save media source";
      return;
    }
  } else {
    qWarning() << "ContextMenuContext::saveMedia(): invalid content";
    return;
  }

  client_->saveMedia();
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
