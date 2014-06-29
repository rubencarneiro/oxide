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

#include "oxideqquickwebview_p.h"
#include "oxideqquickwebview_p_p.h"

#include <QEvent>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QImage>
#include <QKeyEvent>
#include <QMetaMethod>
#include <QMouseEvent>
#include <QPointF>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QRect>
#include <QRectF>
#include <QSGNode>
#include <QSizeF>
#include <QSize>
#include <QTouchEvent>
#include <QtQml>
#include <Qt>

#include "qt/core/api/oxideqpermissionrequest.h"
#if defined(ENABLE_COMPOSITING)
#include "qt/quick/oxide_qquick_accelerated_frame_node.h"
#endif
#include "qt/quick/oxide_qquick_alert_dialog_delegate.h"
#include "qt/quick/oxide_qquick_before_unload_dialog_delegate.h"
#include "qt/quick/oxide_qquick_confirm_dialog_delegate.h"
#include "qt/quick/oxide_qquick_file_picker_delegate.h"
#include "qt/quick/oxide_qquick_prompt_dialog_delegate.h"
#include "qt/quick/oxide_qquick_software_frame_node.h"
#include "qt/quick/oxide_qquick_web_popup_menu_delegate.h"

#include "oxideqquickscriptmessagehandler_p.h"
#include "oxideqquickscriptmessagehandler_p_p.h"
#include "oxideqquickwebcontext_p.h"
#include "oxideqquickwebcontext_p_p.h"
#include "oxideqquickwebframe_p.h"
#include "oxideqquickwebframe_p_p.h"

QT_USE_NAMESPACE

using oxide::qquick::AcceleratedFrameNode;
using oxide::qquick::SoftwareFrameNode;

namespace oxide {
namespace qquick {

class WebViewInputArea : public QQuickItem {
 public:
  WebViewInputArea(OxideQQuickWebView* webview, OxideQQuickWebViewPrivate* d)
      : QQuickItem(webview),
        d_(d) {
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
  }

  virtual ~WebViewInputArea() {}

 private:
  void focusInEvent(QFocusEvent* event) Q_DECL_FINAL;
  void focusOutEvent(QFocusEvent* event) Q_DECL_FINAL;

  void hoverMoveEvent(QHoverEvent* event) Q_DECL_FINAL;

  void inputMethodEvent(QInputMethodEvent* event) Q_DECL_FINAL;
  QVariant inputMethodQuery(Qt::InputMethodQuery query) const Q_DECL_FINAL;

  void keyPressEvent(QKeyEvent* event) Q_DECL_FINAL;
  void keyReleaseEvent(QKeyEvent* event) Q_DECL_FINAL;

  void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_FINAL;
  void mouseMoveEvent(QMouseEvent* event) Q_DECL_FINAL;
  void mousePressEvent(QMouseEvent* event) Q_DECL_FINAL;
  void mouseReleaseEvent(QMouseEvent* event) Q_DECL_FINAL;

  void touchEvent(QTouchEvent* event) Q_DECL_FINAL;

  void wheelEvent(QWheelEvent* event) Q_DECL_FINAL;

  OxideQQuickWebViewPrivate* d_;
};

void WebViewInputArea::focusInEvent(QFocusEvent* event) {
  d_->handleFocusEvent(event);
}

void WebViewInputArea::focusOutEvent(QFocusEvent* event) {
  d_->handleFocusEvent(event);
}

void WebViewInputArea::hoverMoveEvent(QHoverEvent* event) {
  // QtQuick gives us a hover event unless we have a grab (which
  // happens implicitly on button press). As Chromium doesn't
  // distinguish between the 2, just give it a mouse event
  QPointF window_pos = mapToScene(event->posF());
  QMouseEvent me(QEvent::MouseMove,
                 event->posF(),
                 window_pos,
                 window_pos + window()->position(),
                 Qt::NoButton,
                 Qt::NoButton,
                 event->modifiers());
  me.accept();

  d_->handleMouseEvent(&me);

  event->setAccepted(me.isAccepted());
}

void WebViewInputArea::inputMethodEvent(QInputMethodEvent* event) {
  d_->handleInputMethodEvent(event);
}

QVariant WebViewInputArea::inputMethodQuery(
    Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImEnabled:
      return (flags() & QQuickItem::ItemAcceptsInputMethod) != 0;
    default:
      return d_->inputMethodQuery(query);
  }
}

void WebViewInputArea::keyPressEvent(QKeyEvent* event) {
  d_->handleKeyEvent(event);
}

void WebViewInputArea::keyReleaseEvent(QKeyEvent* event) {
  d_->handleKeyEvent(event);
}

void WebViewInputArea::mouseDoubleClickEvent(QMouseEvent* event) {
  d_->handleMouseEvent(event);
}

void WebViewInputArea::mouseMoveEvent(QMouseEvent* event) {
  d_->handleMouseEvent(event);
}

void WebViewInputArea::mousePressEvent(QMouseEvent* event) {
  forceActiveFocus();
  d_->handleMouseEvent(event);
}

void WebViewInputArea::mouseReleaseEvent(QMouseEvent* event) {
  d_->handleMouseEvent(event);
}

void WebViewInputArea::touchEvent(QTouchEvent* event) {
  if (event->type() == QEvent::TouchBegin) {
    forceActiveFocus();
  }
  d_->handleTouchEvent(event);
}

void WebViewInputArea::wheelEvent(QWheelEvent* event) {
  d_->handleWheelEvent(event);
}

} // namespace qquick
} // namespace oxide

class UpdatePaintNodeScope {
 public:
  UpdatePaintNodeScope(OxideQQuickWebViewPrivate* d)
      : d_(d) {}

  virtual ~UpdatePaintNodeScope() {
    d_->didUpdatePaintNode();
  }

 private:
  OxideQQuickWebViewPrivate* d_;
};

OxideQQuickWebViewAttached::OxideQQuickWebViewAttached(QObject* parent) :
    QObject(parent),
    view_(NULL) {}

OxideQQuickWebViewAttached::~OxideQQuickWebViewAttached() {}

OxideQQuickWebView* OxideQQuickWebViewAttached::view() const {
  return view_;
}

void OxideQQuickWebViewAttached::setView(OxideQQuickWebView* view) {
  view_ = view;
}

OxideQQuickWebViewPrivate::OxideQQuickWebViewPrivate(
    OxideQQuickWebView* view) :
    oxide::qt::WebViewAdapter(view),
    load_progress_(0),
    constructed_(false),
    navigation_history_(view),
    popup_menu_(NULL),
    alert_dialog_(NULL),
    confirm_dialog_(NULL),
    prompt_dialog_(NULL),
    before_unload_dialog_(NULL),
    file_picker_(NULL),
    input_area_(NULL),
    received_new_compositor_frame_(false),
    frame_evicted_(false),
    last_composited_frame_type_(oxide::qt::CompositorFrameHandle::TYPE_INVALID) {}

oxide::qt::WebPopupMenuDelegate*
OxideQQuickWebViewPrivate::CreateWebPopupMenuDelegate() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::WebPopupMenuDelegate(q);
}

oxide::qt::JavaScriptDialogDelegate*
OxideQQuickWebViewPrivate::CreateJavaScriptDialogDelegate(
    oxide::qt::JavaScriptDialogDelegate::Type type) {
  Q_Q(OxideQQuickWebView);

  switch (type) {
  case oxide::qt::JavaScriptDialogDelegate::TypeAlert:
    return new oxide::qquick::AlertDialogDelegate(q);
  case oxide::qt::JavaScriptDialogDelegate::TypeConfirm:
    return new oxide::qquick::ConfirmDialogDelegate(q);
  case oxide::qt::JavaScriptDialogDelegate::TypePrompt:
    return new oxide::qquick::PromptDialogDelegate(q);
  default:
    Q_UNREACHABLE();
  }
}

oxide::qt::JavaScriptDialogDelegate*
OxideQQuickWebViewPrivate::CreateBeforeUnloadDialogDelegate() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::BeforeUnloadDialogDelegate(q);
}

oxide::qt::FilePickerDelegate*
OxideQQuickWebViewPrivate::CreateFilePickerDelegate() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::FilePickerDelegate(q);
}

void OxideQQuickWebViewPrivate::OnInitialized(
    bool orig_incognito,
    oxide::qt::WebContextAdapter* orig_context) {
  Q_Q(OxideQQuickWebView);

  // Make the webview the QObject parent of the new root frame,
  // to stop Qml from collecting the frame tree
  q->rootFrame()->setParent(q);

  // Initialization created the root frame. This is the only time
  // this is emitted
  emit q->rootFrameChanged();

  if (orig_incognito != incognito()) {
    emit q->incognitoChanged();
  }
  if (orig_context != context()) {
    detachContextSignals(static_cast<OxideQQuickWebContextPrivate *>(orig_context));
    attachContextSignals(static_cast<OxideQQuickWebContextPrivate *>(context()));

    emit q->contextChanged();
  }
}

void OxideQQuickWebViewPrivate::URLChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->urlChanged();
}

void OxideQQuickWebViewPrivate::TitleChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->titleChanged();
}

void OxideQQuickWebViewPrivate::IconChanged(QUrl icon) {
  Q_Q(OxideQQuickWebView);

  if (icon != icon_) {
    icon_ = icon;
    emit q->iconChanged();
  }
}

void OxideQQuickWebViewPrivate::CommandsUpdated() {
  Q_Q(OxideQQuickWebView);

  emit q->navigationHistoryChanged();
}

void OxideQQuickWebViewPrivate::LoadProgressChanged(double progress) {
  Q_Q(OxideQQuickWebView);

  load_progress_ = progress * 100;
  emit q->loadProgressChanged();
}

void OxideQQuickWebViewPrivate::LoadEvent(OxideQLoadEvent* event) {
  Q_Q(OxideQQuickWebView);

  emit q->loadingChanged(event);
}

void OxideQQuickWebViewPrivate::NavigationEntryCommitted() {
  navigation_history_.onNavigationEntryCommitted();
}

void OxideQQuickWebViewPrivate::NavigationListPruned(bool from_front, int count) {
  navigation_history_.onNavigationListPruned(from_front, count);
}

void OxideQQuickWebViewPrivate::NavigationEntryChanged(int index) {
  navigation_history_.onNavigationEntryChanged(index);
}

oxide::qt::WebFrameAdapter* OxideQQuickWebViewPrivate::CreateWebFrame() {
  OxideQQuickWebFrame* frame = new OxideQQuickWebFrame();
  QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);
  return OxideQQuickWebFramePrivate::get(frame);
}

QScreen* OxideQQuickWebViewPrivate::GetScreen() const {
  Q_Q(const OxideQQuickWebView);

  if (!q->window()) {
    return NULL;
  }

  return q->window()->screen();
}

QRect OxideQQuickWebViewPrivate::GetContainerBoundsPix() const {
  Q_Q(const OxideQQuickWebView);

  if (!q->window()) {
    return QRect();
  }

  QPointF pos(q->mapToScene(QPointF(0, 0)) + q->window()->position());

  return QRect(qRound(pos.x()), qRound(pos.y()),
               qRound(q->width()), qRound(q->height()));
}

bool OxideQQuickWebViewPrivate::IsVisible() const {
  Q_Q(const OxideQQuickWebView);

  return q->isVisible();
}

bool OxideQQuickWebViewPrivate::HasFocus() const {
  return input_area_->hasActiveFocus();
}

void OxideQQuickWebViewPrivate::AddMessageToConsole(
    int level,
    const QString& message,
    int line_no,
    const QString& source_id) {
  Q_Q(OxideQQuickWebView);

  OxideQQuickWebView::LogMessageSeverityLevel oxideLevel =
    OxideQQuickWebView::LogSeverityInfo;
  if (level >= 0 && level <= OxideQQuickWebView::LogSeverityFatal) {
    oxideLevel = static_cast<OxideQQuickWebView::LogMessageSeverityLevel>(level);
  }
  emit q->javaScriptConsoleMessage(
      oxideLevel,
      message,
      line_no,
      source_id);
}

void OxideQQuickWebViewPrivate::ToggleFullscreenMode(bool enter) {
  Q_Q(OxideQQuickWebView);

  emit q->fullscreenRequested(enter);
}

void OxideQQuickWebViewPrivate::OnWebPreferencesChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->preferencesChanged();
}

void OxideQQuickWebViewPrivate::FrameAdded(
    oxide::qt::WebFrameAdapter* frame) {
  Q_Q(OxideQQuickWebView);

  emit q->frameAdded(adapterToQObject<OxideQQuickWebFrame>(frame));
}

void OxideQQuickWebViewPrivate::FrameRemoved(
    oxide::qt::WebFrameAdapter* frame) {
  Q_Q(OxideQQuickWebView);

  emit q->frameRemoved(adapterToQObject<OxideQQuickWebFrame>(frame));
}

bool OxideQQuickWebViewPrivate::CanCreateWindows() const {
  Q_Q(const OxideQQuickWebView);

  // QObject::isSignalConnected doesn't work from here (it still indicates
  // true during the last disconnect)
  return q->receivers(SIGNAL(newViewRequested(OxideQNewViewRequest*))) > 0;
}

void OxideQQuickWebViewPrivate::UpdateCursor(const QCursor& cursor) {
  Q_Q(OxideQQuickWebView);

  q->setCursor(cursor);
}

void OxideQQuickWebViewPrivate::NavigationRequested(
    OxideQNavigationRequest* request) {
  Q_Q(OxideQQuickWebView);

  emit q->navigationRequested(request);
}

void OxideQQuickWebViewPrivate::NewViewRequested(
    OxideQNewViewRequest* request) {
  Q_Q(OxideQQuickWebView);

  emit q->newViewRequested(request);
}

void OxideQQuickWebViewPrivate::RequestGeolocationPermission(
    OxideQGeolocationPermissionRequest* request) {
  Q_Q(OxideQQuickWebView);

  QQmlEngine::setObjectOwnership(request, QQmlEngine::JavaScriptOwnership);
  emit q->geolocationPermissionRequested(request);
}

void OxideQQuickWebViewPrivate::HandleUnhandledKeyboardEvent(
    QKeyEvent* event) {
  Q_Q(OxideQQuickWebView);

  QQuickWindow* w = q->window();
  if (!w) {
    return;
  }

  w->sendEvent(q, event);
}

inline oxide::qt::FrameMetadataChangeFlags operator&(
    oxide::qt::FrameMetadataChangeFlags a,
    oxide::qt::FrameMetadataChangeFlags b) {
  return static_cast<oxide::qt::FrameMetadataChangeFlags>(
      static_cast<int>(a) & static_cast<int>(b));
}

void OxideQQuickWebViewPrivate::FrameMetadataUpdated(
    oxide::qt::FrameMetadataChangeFlags flags) {
  Q_Q(OxideQQuickWebView);

#define IS_SET(flag) flags & flag
  if (IS_SET(oxide::qt::FRAME_METADATA_CHANGE_DEVICE_SCALE) ||
      IS_SET(oxide::qt::FRAME_METADATA_CHANGE_PAGE_SCALE)) {
    emit q->contentXChanged();
    emit q->contentYChanged();
    emit q->contentWidthChanged();
    emit q->contentHeightChanged();
    emit q->viewportWidthChanged();
    emit q->viewportHeightChanged();
  }
  if (IS_SET(oxide::qt::FRAME_METADATA_CHANGE_SCROLL_OFFSET_X)) {
    emit q->contentXChanged();
  }
  if (IS_SET(oxide::qt::FRAME_METADATA_CHANGE_SCROLL_OFFSET_Y)) {
    emit q->contentYChanged();
  }
  if (IS_SET(oxide::qt::FRAME_METADATA_CHANGE_CONTENT_WIDTH)) {
    emit q->contentWidthChanged();
  }
  if (IS_SET(oxide::qt::FRAME_METADATA_CHANGE_CONTENT_HEIGHT)) {
    emit q->contentHeightChanged();
  }
  if (IS_SET(oxide::qt::FRAME_METADATA_CHANGE_VIEWPORT_WIDTH)) {
    emit q->viewportWidthChanged();
  }
  if (IS_SET(oxide::qt::FRAME_METADATA_CHANGE_VIEWPORT_HEIGHT)) {
    emit q->viewportHeightChanged();
  }
#undef IS_SET
}

void OxideQQuickWebViewPrivate::ScheduleUpdate() {
  Q_Q(OxideQQuickWebView);

  frame_evicted_ = false;
  received_new_compositor_frame_ = true;

  q->update();
  q->polish();
}

void OxideQQuickWebViewPrivate::EvictCurrentFrame() {
  Q_Q(OxideQQuickWebView);

  frame_evicted_ = true;
  received_new_compositor_frame_ = false;

  Q_ASSERT(!compositor_frame_handle_);

  q->update();
}

void OxideQQuickWebViewPrivate::SetInputMethodEnabled(bool enabled) {
  Q_Q(OxideQQuickWebView);

  input_area_->setFlag(QQuickItem::ItemAcceptsInputMethod, enabled);
  QGuiApplication::inputMethod()->update(Qt::ImEnabled);
}

void OxideQQuickWebViewPrivate::completeConstruction() {
  Q_Q(OxideQQuickWebView);

  if (!context()) {
    OxideQQuickWebContext* c = OxideQQuickWebContext::defaultContext(true);
    setContext(OxideQQuickWebContextPrivate::get(c));
  }

  init();
}

// static
void OxideQQuickWebViewPrivate::messageHandler_append(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    OxideQQuickScriptMessageHandler* message_handler) {
  if (!message_handler) {
    return;
  }

  OxideQQuickWebView* web_view =
      static_cast<OxideQQuickWebView* >(prop->object);

  web_view->addMessageHandler(message_handler);
}

// static
int OxideQQuickWebViewPrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(
        static_cast<OxideQQuickWebView *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickScriptMessageHandler* OxideQQuickWebViewPrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    int index) {
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(
        static_cast<OxideQQuickWebView *>(prop->object));

  if (index >= p->message_handlers().size()) {
    return NULL;
  }

  return adapterToQObject<OxideQQuickScriptMessageHandler>(
      p->message_handlers().at(index));
}

// static
void OxideQQuickWebViewPrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  OxideQQuickWebView* web_view =
      static_cast<OxideQQuickWebView *>(prop->object);
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(web_view);

  while (p->message_handlers().size() > 0) {
    web_view->removeMessageHandler(
        adapterToQObject<OxideQQuickScriptMessageHandler>(
          p->message_handlers().at(0)));
  }
}

void OxideQQuickWebViewPrivate::contextConstructed() {
  if (constructed_) {
    completeConstruction();
  }
}

void OxideQQuickWebViewPrivate::contextWillBeDestroyed() {
  Q_Q(OxideQQuickWebView);

  // XXX: Our underlying BrowserContext lives on, so we're left in a
  // bit of a weird state here (WebView.context will return no context,
  // which is a lie)
  emit q->contextChanged();
}

void OxideQQuickWebViewPrivate::attachContextSignals(
    OxideQQuickWebContextPrivate* context) {
  Q_Q(OxideQQuickWebView);

  if (!context) {
    return;
  }

  QObject::connect(context, SIGNAL(willBeDestroyed()),
                   q, SLOT(contextWillBeDestroyed()));
  QObject::connect(context, SIGNAL(constructed()),
                   q, SLOT(contextConstructed()));
}

void OxideQQuickWebViewPrivate::detachContextSignals(
    OxideQQuickWebContextPrivate* context) {
  Q_Q(OxideQQuickWebView);

  if (!context) {
    return;
  }

  QObject::disconnect(context, SIGNAL(constructed()),
                      q, SLOT(contextConstructed()));
  QObject::disconnect(context, SIGNAL(willBeDestroyed()),
                      q, SLOT(contextWillBeDestroyed()));
}

void OxideQQuickWebViewPrivate::didUpdatePaintNode() {
  if (received_new_compositor_frame_) {
    received_new_compositor_frame_ = false;
    didCommitCompositorFrame();
  }
  compositor_frame_handle_.reset();
}

void OxideQQuickWebViewPrivate::onWindowChanged(QQuickWindow* window) {
  if (!window) {
    return;
  }

  wasResized();
}

OxideQQuickWebViewPrivate::~OxideQQuickWebViewPrivate() {}

// static
OxideQQuickWebViewPrivate* OxideQQuickWebViewPrivate::get(
    OxideQQuickWebView* view) {
  return view->d_func();
}

void OxideQQuickWebViewPrivate::addAttachedPropertyTo(QObject* object) {
  Q_Q(OxideQQuickWebView);

  OxideQQuickWebViewAttached* attached =
      qobject_cast<OxideQQuickWebViewAttached *>(
        qmlAttachedPropertiesObject<OxideQQuickWebView>(object));
  attached->setView(q);
}

void OxideQQuickWebView::connectNotify(const QMetaMethod& signal) {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(thread() == QThread::currentThread());

  if (signal == QMetaMethod::fromSignal(
          &OxideQQuickWebView::newViewRequested)) {
    d->updateWebPreferences();
  }
}

void OxideQQuickWebView::disconnectNotify(const QMetaMethod& signal) {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(thread() == QThread::currentThread());

  if (signal == QMetaMethod::fromSignal(
          &OxideQQuickWebView::newViewRequested) ||
      !signal.isValid()) {
    d->updateWebPreferences();
  }
}

void OxideQQuickWebView::geometryChanged(const QRectF& newGeometry,
                                         const QRectF& oldGeometry) {
  Q_D(OxideQQuickWebView);

  QQuickItem::geometryChanged(newGeometry, oldGeometry);

  d->input_area_->setWidth(newGeometry.width());
  d->input_area_->setHeight(newGeometry.height());

  if (d->isInitialized() && window()) {
    d->wasResized();
  }
}

void OxideQQuickWebView::itemChange(QQuickItem::ItemChange change,
                                    const QQuickItem::ItemChangeData& value) {
  Q_D(OxideQQuickWebView);

  QQuickItem::itemChange(change, value);

  if (!d->isInitialized()) {
    return;
  }

  if (change == QQuickItem::ItemVisibleHasChanged) {
    d->visibilityChanged();
  }
}

QSGNode* OxideQQuickWebView::updatePaintNode(
    QSGNode* oldNode,
    UpdatePaintNodeData * updatePaintNodeData) {
  Q_UNUSED(updatePaintNodeData);
  Q_D(OxideQQuickWebView);

  UpdatePaintNodeScope scope(d);

  oxide::qt::CompositorFrameHandle::Type type =
      oxide::qt::CompositorFrameHandle::TYPE_INVALID;

  if (d->received_new_compositor_frame_) {
    Q_ASSERT(d->compositor_frame_handle_);
    Q_ASSERT(!d->frame_evicted_);
    type = d->compositor_frame_handle_->GetType();
  } else if (!d->frame_evicted_) {
    Q_ASSERT(!d->compositor_frame_handle_);
    type = d->last_composited_frame_type_;
  }

  if (type != d->last_composited_frame_type_) {
    delete oldNode;
    oldNode = NULL;
  }

  d->last_composited_frame_type_ = type;

  if (d->frame_evicted_) {
    delete oldNode;
    return NULL;
  }

#if defined(ENABLE_COMPOSITING)
  if (type == oxide::qt::CompositorFrameHandle::TYPE_ACCELERATED) {
    AcceleratedFrameNode* node = static_cast<AcceleratedFrameNode *>(oldNode);
    if (!node) {
      node = new AcceleratedFrameNode(this);
    }

    if (d->received_new_compositor_frame_ || !oldNode) {
      node->updateNode(d->compositor_frame_handle_);
    }

    return node;
  }
#else
  Q_ASSERT(type != oxide::qt::CompositorFrameHandle::TYPE_ACCELERATED);
#endif

  if (type == oxide::qt::CompositorFrameHandle::TYPE_SOFTWARE) {
    SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(oldNode);
    if (!node) {
      node = new SoftwareFrameNode(this);
    }

    if (d->received_new_compositor_frame_ || !oldNode) {
      node->updateNode(d->compositor_frame_handle_);
    }

    return node;
  }

  Q_ASSERT(type == oxide::qt::CompositorFrameHandle::TYPE_INVALID);

  SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(oldNode);
  if (!node) {
    node = new SoftwareFrameNode(this);
  }

  QRectF rect(QPointF(0, 0), QSizeF(width(), height()));

  if (!oldNode || rect != node->rect()) {
    QImage blank(qRound(rect.width()),
                 qRound(rect.height()),
                 QImage::Format_ARGB32);
    blank.fill(Qt::white);
    node->setImage(blank);
  }

  return node;
}

void OxideQQuickWebView::updatePolish() {
  Q_D(OxideQQuickWebView);

  d->compositor_frame_handle_ = d->compositorFrameHandle();
}

OxideQQuickWebView::OxideQQuickWebView(QQuickItem* parent) :
    QQuickItem(parent) {
  // WebView instantiates NotificationRegistrar, which starts
  // NotificationService, which uses LazyInstance. Start Chromium now
  // else we'll crash
  OxideQQuickWebContextPrivate::ensureChromiumStarted();
  d_ptr.reset(new OxideQQuickWebViewPrivate(this));

  Q_D(OxideQQuickWebView);

  setFlags(QQuickItem::ItemClipsChildrenToShape |
           QQuickItem::ItemHasContents |
           QQuickItem::ItemIsFocusScope);

  connect(this, SIGNAL(windowChanged(QQuickWindow*)),
          this, SLOT(onWindowChanged(QQuickWindow*)));

  // We have an input area QQuickItem for receiving input events, so
  // that we have a way of bubbling unhandled key events back to the
  // WebView
  d->input_area_ = new oxide::qquick::WebViewInputArea(this, d);
  d->input_area_->setX(0.0f);
  d->input_area_->setY(0.0f);
  d->input_area_->setWidth(width());
  d->input_area_->setHeight(height());
  d->input_area_->setFocus(true);
}

OxideQQuickWebView::~OxideQQuickWebView() {
  Q_D(OxideQQuickWebView);

  d->detachContextSignals(
      static_cast<OxideQQuickWebContextPrivate *>(d->context()));

  // Do this before our d_ptr is cleared, as these call back in to us
  // when they are deleted
  while (d->message_handlers().size() > 0) {
    delete adapterToQObject<OxideQQuickScriptMessageHandler>(
        d->message_handlers().at(0));
  }

  // Delete this now as it can get a focusOutEvent after our destructor
  // runs, calling back in to our deleted oxide::WebView
  delete d->input_area_;
}

void OxideQQuickWebView::componentComplete() {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(!d->constructed_);
  d->constructed_ = true;

  QQuickItem::componentComplete();

  if (!d->context() ||
      static_cast<OxideQQuickWebContextPrivate *>(d->context())->isConstructed()) {
    d->completeConstruction();
  }
}

QUrl OxideQQuickWebView::url() const {
  Q_D(const OxideQQuickWebView);

  return d->url();
}

void OxideQQuickWebView::setUrl(const QUrl& url) {
  Q_D(OxideQQuickWebView);

  d->setUrl(url);
}

QString OxideQQuickWebView::title() const {
  Q_D(const OxideQQuickWebView);

  return d->title();
}

QUrl OxideQQuickWebView::icon() const {
  Q_D(const OxideQQuickWebView);

  return d->icon_;
}

bool OxideQQuickWebView::canGoBack() const {
  Q_D(const OxideQQuickWebView);

  return d->canGoBack();
}

bool OxideQQuickWebView::canGoForward() const {
  Q_D(const OxideQQuickWebView);

  return d->canGoForward();
}

bool OxideQQuickWebView::incognito() const {
  Q_D(const OxideQQuickWebView);

  return d->incognito();
}

void OxideQQuickWebView::setIncognito(bool incognito) {
  Q_D(OxideQQuickWebView);

  if (incognito == d->incognito()) {
    return;
  }

  d->setIncognito(incognito);
  emit incognitoChanged();
}

bool OxideQQuickWebView::loading() const {
  Q_D(const OxideQQuickWebView);

  return d->loading();
}

bool OxideQQuickWebView::fullscreen() const {
  Q_D(const OxideQQuickWebView);

  return d->fullscreen();
}

void OxideQQuickWebView::setFullscreen(bool fullscreen) {
  Q_D(OxideQQuickWebView);

  if (fullscreen == d->fullscreen()) {
    return;
  }

  d->setFullscreen(fullscreen);
  emit fullscreenChanged();
}

int OxideQQuickWebView::loadProgress() const {
  Q_D(const OxideQQuickWebView);

  return d->load_progress_;
}

OxideQQuickWebFrame* OxideQQuickWebView::rootFrame() const {
  Q_D(const OxideQQuickWebView);

  return adapterToQObject<OxideQQuickWebFrame>(d->rootFrame());
}

QQmlListProperty<OxideQQuickScriptMessageHandler>
OxideQQuickWebView::messageHandlers() {
  return QQmlListProperty<OxideQQuickScriptMessageHandler>(
      this, NULL,
      OxideQQuickWebViewPrivate::messageHandler_append,
      OxideQQuickWebViewPrivate::messageHandler_count,
      OxideQQuickWebViewPrivate::messageHandler_at,
      OxideQQuickWebViewPrivate::messageHandler_clear);
}

void OxideQQuickWebView::addMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebView);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (hd->isActive() && handler->parent() != this) {
    qWarning() << "MessageHandler can't be added to more than one message target";
    return;
  }

  if (d->message_handlers().contains(hd)) {
    d->message_handlers().removeOne(hd);
  }

  handler->setParent(this);
  d->message_handlers().append(hd);

  emit messageHandlersChanged();
}

void OxideQQuickWebView::removeMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebView);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (!d->message_handlers().contains(hd)) {
    return;
  }

  handler->setParent(NULL);
  d->message_handlers().removeOne(hd);

  emit messageHandlersChanged();
}

qreal OxideQQuickWebView::viewportWidth() const {
  Q_D(const OxideQQuickWebView);

  return d->compositorFrameViewportSize().width()
      * d->compositorFrameDeviceScaleFactor()
      * d->compositorFramePageScaleFactor();
}

qreal OxideQQuickWebView::viewportHeight() const {
  Q_D(const OxideQQuickWebView);

  return d->compositorFrameViewportSize().height()
      * d->compositorFrameDeviceScaleFactor()
      * d->compositorFramePageScaleFactor();
}

qreal OxideQQuickWebView::contentWidth() const {
  Q_D(const OxideQQuickWebView);

  return d->compositorFrameLayerSize().width()
      * d->compositorFrameDeviceScaleFactor()
      * d->compositorFramePageScaleFactor();
}

qreal OxideQQuickWebView::contentHeight() const {
  Q_D(const OxideQQuickWebView);

  return d->compositorFrameLayerSize().height()
      * d->compositorFrameDeviceScaleFactor()
      * d->compositorFramePageScaleFactor();
}

qreal OxideQQuickWebView::contentX() const {
  Q_D(const OxideQQuickWebView);

  return d->compositorFrameScrollOffset().x()
      * d->compositorFrameDeviceScaleFactor()
      * d->compositorFramePageScaleFactor();
}

qreal OxideQQuickWebView::contentY() const {
  Q_D(const OxideQQuickWebView);

  return d->compositorFrameScrollOffset().y()
      * d->compositorFrameDeviceScaleFactor()
      * d->compositorFramePageScaleFactor();
}

QQmlComponent* OxideQQuickWebView::popupMenu() const {
  Q_D(const OxideQQuickWebView);

  return d->popup_menu_;
}

void OxideQQuickWebView::setPopupMenu(QQmlComponent* popup_menu) {
  Q_D(OxideQQuickWebView);

  if (d->popup_menu_ == popup_menu) {
    return;
  }

  d->popup_menu_ = popup_menu;
  emit popupMenuChanged();
}

QQmlComponent* OxideQQuickWebView::alertDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->alert_dialog_;
}

void OxideQQuickWebView::setAlertDialog(QQmlComponent* alert_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->alert_dialog_ == alert_dialog) {
    return;
  }

  d->alert_dialog_ = alert_dialog;
  emit alertDialogChanged();
}

QQmlComponent* OxideQQuickWebView::confirmDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->confirm_dialog_;
}

void OxideQQuickWebView::setConfirmDialog(QQmlComponent* confirm_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->confirm_dialog_ == confirm_dialog) {
    return;
  }

  d->confirm_dialog_ = confirm_dialog;
  emit confirmDialogChanged();
}

QQmlComponent* OxideQQuickWebView::promptDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->prompt_dialog_;
}

void OxideQQuickWebView::setPromptDialog(QQmlComponent* prompt_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->prompt_dialog_ == prompt_dialog) {
    return;
  }

  d->prompt_dialog_ = prompt_dialog;
  emit promptDialogChanged();
}

QQmlComponent* OxideQQuickWebView::beforeUnloadDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->before_unload_dialog_;
}

void OxideQQuickWebView::setBeforeUnloadDialog(
    QQmlComponent* before_unload_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->before_unload_dialog_ == before_unload_dialog) {
    return;
  }

  d->before_unload_dialog_ = before_unload_dialog;
  emit beforeUnloadDialogChanged();
}

QQmlComponent* OxideQQuickWebView::filePicker() const {
  Q_D(const OxideQQuickWebView);

  return d->file_picker_;
}

void OxideQQuickWebView::setFilePicker(QQmlComponent* file_picker) {
  Q_D(OxideQQuickWebView);

  if (d->file_picker_ == file_picker) {
    return;
  }

  d->file_picker_ = file_picker;
  emit filePickerChanged();
}

OxideQQuickWebContext* OxideQQuickWebView::context() const {
  Q_D(const OxideQQuickWebView);

  OxideQQuickWebContext* c =
      adapterToQObject<OxideQQuickWebContext>(d->context());
  if (c == OxideQQuickWebContext::defaultContext(false)) {
    return NULL;
  }

  return c;
}

void OxideQQuickWebView::setContext(OxideQQuickWebContext* context) {
  Q_D(OxideQQuickWebView);

  if (d->isInitialized()) {
    qWarning() << "WebView context must be set during construction";
    return;
  }

  if (context == OxideQQuickWebContext::defaultContext(false)) {
    qWarning() <<
        "Setting WebView context to default context is unnecessary. WebView "
        "will automatically use the default context if it is created without "
        "one";
    return;
  }

  oxide::qt::WebContextAdapter* old = d->context();
  if (context == adapterToQObject<OxideQQuickWebContext>(old)) {
    return;
  }
  d->detachContextSignals(static_cast<OxideQQuickWebContextPrivate *>(old));

  OxideQQuickWebContextPrivate* cd = NULL;
  if (context) {
    cd = OxideQQuickWebContextPrivate::get(context);
  }
  d->attachContextSignals(cd);
  d->setContext(cd);

  emit contextChanged();
}

OxideQWebPreferences* OxideQQuickWebView::preferences() {
  Q_D(OxideQQuickWebView);

  return d->preferences();
}

void OxideQQuickWebView::setPreferences(OxideQWebPreferences* prefs) {
  Q_D(OxideQQuickWebView);

  if (prefs == d->preferences()) {
    return;
  }

  d->setPreferences(prefs);

  emit preferencesChanged();
}

OxideQQuickNavigationHistory* OxideQQuickWebView::navigationHistory() {
  Q_D(OxideQQuickWebView);

  return &d->navigation_history_;
}

// This exists purely to remove a moc warning. We don't store this request
// anywhere, it's only a transient object and I can't think of any possible
// reason why anybody would want to read it back
OxideQNewViewRequest* OxideQQuickWebView::request() const {
  return NULL;
}

void OxideQQuickWebView::setRequest(OxideQNewViewRequest* request) {
  Q_D(OxideQQuickWebView);

  d->setRequest(request);
}

// static
OxideQQuickWebViewAttached* OxideQQuickWebView::qmlAttachedProperties(
    QObject* object) {
  return new OxideQQuickWebViewAttached(object);
}

void OxideQQuickWebView::goBack() {
  Q_D(OxideQQuickWebView);

  d->goBack();
}

void OxideQQuickWebView::goForward() {
  Q_D(OxideQQuickWebView);

  d->goForward();
}

void OxideQQuickWebView::stop() {
  Q_D(OxideQQuickWebView);

  d->stop();
}

void OxideQQuickWebView::reload() {
  Q_D(OxideQQuickWebView);

  d->reload();
}

void OxideQQuickWebView::loadHtml(const QString& html, const QUrl& baseUrl) {
  Q_D(OxideQQuickWebView);

  d->loadHtml(html, baseUrl);
}

#include "moc_oxideqquickwebview_p.cpp"
