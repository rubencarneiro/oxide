// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#include "qquick_legacy_web_context_menu.h"

#include <QPointF>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QtDebug>
#include <QUrl>

#include "qt/core/glue/macros.h"
#include "qt/core/glue/web_context_menu_actions.h"
#include "qt/core/glue/web_context_menu_client.h"
#include "qt/quick/api/oxideqquickwebview.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

using qt::WebContextMenuAction;
using qt::WebContextMenuClient;
using qt::WebContextMenuParams;

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
  ContextMenuContext(WebContextMenuParams params,
                     WebContextMenuClient* client);

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

  Q_INVOKABLE void copyImage() const;
  Q_INVOKABLE void saveLink() const;
  Q_INVOKABLE void saveMedia() const;

  Q_INVOKABLE void close();

 private:
  WebContextMenuParams params_;
  WebContextMenuClient* client_;
};

ContextMenuContext::ContextMenuContext(WebContextMenuParams params,
                                       WebContextMenuClient* client)
    : params_(std::move(params)),
      client_(client) {}

OxideQQuickWebView::MediaType ContextMenuContext::mediaType() const {
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaTypeNone,
                              qt::MEDIA_TYPE_NONE);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaTypeImage,
                              qt::MEDIA_TYPE_IMAGE);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaTypeVideo,
                              qt::MEDIA_TYPE_VIDEO);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaTypeAudio,
                              qt::MEDIA_TYPE_AUDIO);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaTypeCanvas,
                              qt::MEDIA_TYPE_CANVAS);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaTypePlugin,
                              qt::MEDIA_TYPE_PLUGIN);

  return static_cast<OxideQQuickWebView::MediaType>(params_.media_type);
}

QPointF ContextMenuContext::position() const {
  return params_.position;
}

QUrl ContextMenuContext::linkUrl() const {
  return params_.link_url;
}

QString ContextMenuContext::linkText() const {
  return params_.link_text;
}

QUrl ContextMenuContext::unfilteredLinkUrl() const {
  return params_.unfiltered_link_url;
}

QUrl ContextMenuContext::srcUrl() const {
  return params_.src_url;
}

bool ContextMenuContext::hasImageContents() const {
  return params_.has_image_contents;
}

QUrl ContextMenuContext::pageUrl() const {
  return params_.page_url;
}

QUrl ContextMenuContext::frameUrl() const {
  return params_.frame_url;
}

QString ContextMenuContext::selectionText() const {
  return params_.selection_text;
}

bool ContextMenuContext::isEditable() const {
  return params_.is_editable;
}

OxideQQuickWebView::EditCapabilities ContextMenuContext::editFlags() const {
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::NoCapability,
                              qt::EDIT_CAPABILITY_NONE);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::UndoCapability,
                              qt::EDIT_CAPABILITY_UNDO);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::RedoCapability,
                              qt::EDIT_CAPABILITY_REDO);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::CutCapability,
                              qt::EDIT_CAPABILITY_CUT);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::CopyCapability,
                              qt::EDIT_CAPABILITY_COPY);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::PasteCapability,
                              qt::EDIT_CAPABILITY_PASTE);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::EraseCapability,
                              qt::EDIT_CAPABILITY_ERASE);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::SelectAllCapability,
                              qt::EDIT_CAPABILITY_SELECT_ALL);

  return static_cast<OxideQQuickWebView::EditCapabilities>(
      qt::EditCapabilityFlags::Int(params_.edit_flags));
}

OxideQQuickWebView::MediaStatus ContextMenuContext::mediaFlags() const {
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusNone,
                              qt::MEDIA_STATUS_NONE);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusInError,
                              qt::MEDIA_STATUS_IN_ERROR);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusPaused,
                              qt::MEDIA_STATUS_PAUSED);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusMuted,
                              qt::MEDIA_STATUS_MUTED);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusLoop,
                              qt::MEDIA_STATUS_LOOP);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusCanSave,
                              qt::MEDIA_STATUS_CAN_SAVE);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusHasAudio,
                              qt::MEDIA_STATUS_HAS_AUDIO);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusCanToggleControls,
                              qt::MEDIA_STATUS_CAN_TOGGLE_CONTROLS);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusControls,
                              qt::MEDIA_STATUS_CONTROLS);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusCanPrint,
                              qt::MEDIA_STATUS_CAN_PRINT);
  STATIC_ASSERT_MATCHING_ENUM(OxideQQuickWebView::MediaStatusCanRotate,
                              qt::MEDIA_STATUS_CAN_ROTATE);

  return static_cast<OxideQQuickWebView::MediaStatus>(
      qt::MediaStatusFlags::Int(params_.media_flags));
}

void ContextMenuContext::copyImage() const {
  OxideQQuickWebView::MediaType media_type = mediaType();
  if (media_type != OxideQQuickWebView::MediaTypeImage &&
      media_type != OxideQQuickWebView::MediaTypeCanvas) {
    qWarning() << "ContextMenuContext::copyImage(): not an image";
    return;
  }

  if (!hasImageContents()) {
    qWarning() << "ContextMenuContext::copyImage(): image has no contents";
    return;
  }

  client_->execCommand(WebContextMenuAction::CopyImage, false);
}

void ContextMenuContext::saveLink() const {
  if (linkUrl().isValid()) {
    client_->execCommand(WebContextMenuAction::SaveLink, false);
  } else {
    qWarning() << "ContextMenuContext::saveLink(): not a valid link";
  }
}

void ContextMenuContext::saveMedia() const {
  OxideQQuickWebView::MediaType media_type = mediaType();
  if (media_type == OxideQQuickWebView::MediaTypeImage ||
      media_type == OxideQQuickWebView::MediaTypeCanvas) {
    if (!hasImageContents()) {
      qWarning() << "ContextMenuContext::saveMedia(): image has no contents";
      return;
    }

    client_->execCommand(WebContextMenuAction::SaveImage, false);
  } else if (media_type == OxideQQuickWebView::MediaTypeVideo ||
             media_type == OxideQQuickWebView::MediaTypeAudio) {
    if (!mediaFlags().testFlag(OxideQQuickWebView::MediaStatusCanSave)) {
      qWarning() << "ContextMenuContext::saveMedia(): cannot save media source";
      return;
    }

    client_->execCommand(WebContextMenuAction::SaveMedia, false);
  } else {
    qWarning() << "ContextMenuContext::saveMedia(): invalid content";
  }
}

void ContextMenuContext::close() {
  client_->close();
}

} // namespace

void LegacyWebContextMenu::Show() {
  if (!parent_) {
    qWarning() <<
        "LegacyWebContextMenu::Show: Can't show after the view has gone";
    client_->close();
    return;
  }

  if (!component_) {
    qWarning() <<
        "LegacyWebContextMenu::Show: Content requested a context menu, but the "
        "application hasn't provided one";
    client_->close();
    return;
  }

  ContextMenuContext* contextObject = new ContextMenuContext(std::move(params_),
                                                             client_);

  QQmlContext* baseContext = component_->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(parent_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextProperty(QLatin1String("model"), contextObject);
  context_->setContextObject(contextObject);
  contextObject->setParent(context_.get());

  item_.reset(qobject_cast<QQuickItem*>(
      component_->beginCreate(context_.get())));
  if (!item_) {
    qWarning() <<
        "LegacyWebContextMenu::Show: Failed to create instance of Qml context "
        "menu component";
    client_->close();
    return;
  }

  // This is a bit hacky - not sure what we'll do here if we introduce
  // other items that can use a ContentView and which support custom UI
  // components
  OxideQQuickWebView* web_view = qobject_cast<OxideQQuickWebView*>(parent_);
  if (web_view) {
    OxideQQuickWebViewPrivate::get(web_view)
        ->addAttachedPropertyTo(item_.get());
  }
  item_->setParentItem(parent_);

  component_->completeCreate();
}

void LegacyWebContextMenu::Hide() {
  if (item_) {
    item_->setVisible(false);
  }
}

LegacyWebContextMenu::LegacyWebContextMenu(QQuickItem* parent,
                                           QQmlComponent* component,
                                           const WebContextMenuParams& params,
                                           WebContextMenuClient* client)
    : params_(params),
      client_(client),
      parent_(parent),
      component_(component) {}

LegacyWebContextMenu::~LegacyWebContextMenu() {}

} // namespace qquick
} // namespace oxide

#include "qquick_legacy_web_context_menu.moc"
