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

#include "oxide_qt_web_view_adapter.h"

#include <QPointF>
#include <QSizeF>
#include <QtDebug>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "cc/output/compositor_frame_metadata.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/geometry/size_f.h"
#include "ui/gfx/geometry/vector2d_f.h"
#include "url/gurl.h"

#include "qt/core/api/oxideqnewviewrequest_p.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/core/api/oxideqwebpreferences_p.h"
#include "qt/core/browser/oxide_qt_web_context.h"
#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/browser/oxide_qt_web_preferences.h"
#include "qt/core/browser/oxide_qt_web_view.h"
#include "shared/base/oxide_enum_flags.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/browser/oxide_content_types.h"
#include "shared/browser/oxide_browser_process_main.h"

#include "oxide_qt_web_context_adapter.h"
#include "oxide_qt_web_frame_adapter.h"

namespace oxide {
namespace qt {

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(FrameMetadataChangeFlags)

class CompositorFrameHandleImpl : public CompositorFrameHandle {
 public:
  CompositorFrameHandleImpl(oxide::CompositorFrameHandle* frame,
                            int location_bar_content_offset)
      : frame_(frame) {
    if (frame_.get()) {
      rect_ = QRect(0, location_bar_content_offset,
                    frame_->size_in_pixels().width(),
                    frame_->size_in_pixels().height());
    }
  }

  virtual ~CompositorFrameHandleImpl() {}

  CompositorFrameHandle::Type GetType() final {
    if (!frame_.get()) {
      return CompositorFrameHandle::TYPE_INVALID;
    }
    if (frame_->gl_frame_data()) {
      return CompositorFrameHandle::TYPE_ACCELERATED;
    }
    if (frame_->software_frame_data()) {
      return CompositorFrameHandle::TYPE_SOFTWARE;
    }

    NOTREACHED();
    return CompositorFrameHandle::TYPE_INVALID;
  }

  const QRect& GetRect() const final {
    return rect_;
  }

  QImage GetSoftwareFrame() final {
    DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_SOFTWARE);
    return QImage(
        static_cast<uchar *>(frame_->software_frame_data()->pixels()),
        frame_->size_in_pixels().width(),
        frame_->size_in_pixels().height(),
        QImage::Format_ARGB32);
  }

  AcceleratedFrameData GetAcceleratedFrame() final {
    DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_ACCELERATED);
    return AcceleratedFrameData(frame_->gl_frame_data()->texture_id());
  }

 private:
  scoped_refptr<oxide::CompositorFrameHandle> frame_;
  QRect rect_;
};

void WebViewAdapter::EnsurePreferences() {
  if (view_->GetWebPreferences()) {
    return;
  }

  OxideQWebPreferences* p = new OxideQWebPreferences(adapterToQObject(this));
  view_->SetWebPreferences(
      OxideQWebPreferencesPrivate::get(p)->preferences());
}

void WebViewAdapter::Initialized() {
  DCHECK(isInitialized());

  OxideQWebPreferences* p =
      static_cast<WebPreferences*>(view_->GetWebPreferences())->api_handle();
  if (!p->parent()) {
    // This will happen for a WebView created by newViewRequested, as
    // we clone the openers preferences before the WebView is created
    p->setParent(adapterToQObject(this));
  }

  OnInitialized();
}

void WebViewAdapter::WebPreferencesDestroyed() {
  OxideQWebPreferences* p = new OxideQWebPreferences(adapterToQObject(this));
  view_->SetWebPreferences(
      OxideQWebPreferencesPrivate::get(p)->preferences());

  OnWebPreferencesReplaced();
}

void WebViewAdapter::FrameMetadataUpdated(FrameMetadataChangeFlags flags) {
  if (flags & FRAME_METADATA_CHANGE_DEVICE_SCALE ||
      flags & FRAME_METADATA_CHANGE_PAGE_SCALE) {
    flags |= FRAME_METADATA_CHANGE_SCROLL_OFFSET;
    flags |= FRAME_METADATA_CHANGE_CONTENT;
    flags |= FRAME_METADATA_CHANGE_VIEWPORT;
  }

  if (flags & FRAME_METADATA_CHANGE_VIEWPORT) {
    flags |= FRAME_METADATA_CHANGE_SCROLL_OFFSET;
    flags |= FRAME_METADATA_CHANGE_CONTENT;
  }

  frame_metadata_dirty_flags_ |= flags;

  OnFrameMetadataUpdated(flags);
}

void WebViewAdapter::ScheduleUpdate() {
  compositor_frame_.reset();
  OnScheduleUpdate();
}

void WebViewAdapter::EvictCurrentFrame() {
  compositor_frame_.reset();
  OnEvictCurrentFrame();
}

float WebViewAdapter::GetFrameMetadataScaleToPix() {
  if (frame_metadata_dirty_flags_ & FRAME_METADATA_CHANGE_DEVICE_SCALE ||
      frame_metadata_dirty_flags_ & FRAME_METADATA_CHANGE_PAGE_SCALE) {
    frame_metadata_dirty_flags_ &= ~FRAME_METADATA_CHANGE_DEVICE_SCALE;
    frame_metadata_dirty_flags_ &= ~FRAME_METADATA_CHANGE_PAGE_SCALE;

    const cc::CompositorFrameMetadata& metadata =
        view_->compositor_frame_metadata();
    frame_metadata_scale_to_pix_ =
      metadata.device_scale_factor * metadata.page_scale_factor;
  }

  return frame_metadata_scale_to_pix_;
}

WebViewAdapter::WebViewAdapter(QObject* q) :
    AdapterBase(q),
    view_(WebView::Create(this)),
    frame_metadata_dirty_flags_(FrameMetadataChangeFlags(-1)),
    frame_metadata_scale_to_pix_(0.0f),
    location_bar_offset_(0),
    location_bar_content_offset_(0) {}

WebViewAdapter::~WebViewAdapter() {}

void WebViewAdapter::init(bool incognito,
                          WebContextAdapter* context,
                          OxideQNewViewRequest* new_view_request,
                          int location_bar_height) {
  DCHECK(!isInitialized());

  bool script_opened = false;

  if (new_view_request) {
    OxideQNewViewRequestPrivate* rd =
        OxideQNewViewRequestPrivate::get(new_view_request);
    if (rd->view) {
      qWarning() << "Cannot assign NewViewRequest to more than one WebView";
    } else {
      rd->view = view_->AsWeakPtr();
      script_opened = true;
    }
  }

  if (script_opened) {
    // Script opened webviews get initialized via another path
    return;
  }

  CHECK(context) <<
      "No context available for WebView. If you see this when running in "
      "single-process mode, it is possible that the default WebContext has "
      "been deleted by the application. In single-process mode, there is only "
      "one WebContext, and this has to live for the life of the application";

  WebContext* c = WebContext::FromAdapter(context);

  if (oxide::BrowserProcessMain::GetInstance()->GetProcessModel() ==
          oxide::PROCESS_MODEL_SINGLE_PROCESS) {
    DCHECK(!incognito);
    DCHECK_EQ(c, WebContext::GetDefault());
  }

  EnsurePreferences();

  oxide::WebView::Params params;
  params.context = c->GetContext();
  params.incognito = incognito;
  params.location_bar_height = location_bar_height;

  view_->Init(&params);
}

QUrl WebViewAdapter::url() const {
  return QUrl(QString::fromStdString(view_->GetURL().spec()));
}

void WebViewAdapter::setUrl(const QUrl& url) {
  view_->SetURL(GURL(url.toString().toStdString()));
}

QString WebViewAdapter::title() const {
  return QString::fromStdString(view_->GetTitle());
}

bool WebViewAdapter::canGoBack() const {
  return view_->CanGoBack();
}

bool WebViewAdapter::canGoForward() const {
  return view_->CanGoForward();
}

bool WebViewAdapter::incognito() const {
  return view_->IsIncognito();  
}

bool WebViewAdapter::loading() const {
  return view_->IsLoading();
}

bool WebViewAdapter::fullscreen() const {
  return view_->IsFullscreen();
}

void WebViewAdapter::setFullscreen(bool fullscreen) {
  view_->SetIsFullscreen(fullscreen);
}

WebFrameAdapter* WebViewAdapter::rootFrame() const {
  return WebFrameAdapter::FromWebFrame(
      static_cast<WebFrame *>(view_->GetRootFrame()));
}

WebContextAdapter* WebViewAdapter::context() const {
  WebContext* c = view_->GetContext();
  if (!c) {
    return NULL;
  }

  return WebContextAdapter::FromWebContext(c);
}

void WebViewAdapter::wasResized() {
  view_->WasResized();
}

void WebViewAdapter::visibilityChanged() {
  view_->VisibilityChanged();
}

void WebViewAdapter::handleFocusEvent(QFocusEvent* event) {
  view_->HandleFocusEvent(event);
}

void WebViewAdapter::handleInputMethodEvent(QInputMethodEvent* event) {
  view_->HandleInputMethodEvent(event);
}

void WebViewAdapter::handleKeyEvent(QKeyEvent* event) {
  view_->HandleKeyEvent(event);
}

void WebViewAdapter::handleMouseEvent(QMouseEvent* event) {
  view_->HandleMouseEvent(event);
}

void WebViewAdapter::handleTouchEvent(QTouchEvent* event) {
  view_->HandleTouchEvent(event);
}

void WebViewAdapter::handleWheelEvent(QWheelEvent* event) {
  view_->HandleWheelEvent(event);
}

QVariant WebViewAdapter::inputMethodQuery(Qt::InputMethodQuery query) const {
  return view_->InputMethodQuery(query);
}

void WebViewAdapter::goBack() {
  view_->GoBack();
}

void WebViewAdapter::goForward() {
  view_->GoForward();
}

void WebViewAdapter::stop() {
  view_->Stop();
}

void WebViewAdapter::reload() {
  view_->Reload();
}

void WebViewAdapter::loadHtml(const QString& html, const QUrl& baseUrl) {
  QByteArray encodedData = html.toUtf8().toPercentEncoding();
  view_->LoadData(std::string(encodedData.constData(), encodedData.length()),
                  "text/html;charset=UTF-8",
                  GURL(baseUrl.toString().toStdString()));
}

QList<ScriptMessageHandlerAdapter*>& WebViewAdapter::messageHandlers() {
  return message_handlers_;
}

bool WebViewAdapter::isInitialized() const {
  return view_->GetWebContents() != NULL;
}

int WebViewAdapter::getNavigationEntryCount() const {
  return view_->GetNavigationEntryCount();
}

int WebViewAdapter::getNavigationCurrentEntryIndex() const {
  return view_->GetNavigationCurrentEntryIndex();
}

void WebViewAdapter::setNavigationCurrentEntryIndex(int index) {
  view_->SetNavigationCurrentEntryIndex(index);
}

int WebViewAdapter::getNavigationEntryUniqueID(int index) const {
  return view_->GetNavigationEntryUniqueID(index);
}

QUrl WebViewAdapter::getNavigationEntryUrl(int index) const {
  return QUrl(QString::fromStdString(
      view_->GetNavigationEntryUrl(index).spec()));
}

QString WebViewAdapter::getNavigationEntryTitle(int index) const {
  return QString::fromStdString(view_->GetNavigationEntryTitle(index));
}

QDateTime WebViewAdapter::getNavigationEntryTimestamp(int index) const {
  return QDateTime::fromMSecsSinceEpoch(
      view_->GetNavigationEntryTimestamp(index).ToJsTime());
}

OxideQWebPreferences* WebViewAdapter::preferences() {
  EnsurePreferences();
  return static_cast<WebPreferences*>(
      view_->GetWebPreferences())->api_handle();
}

void WebViewAdapter::setPreferences(OxideQWebPreferences* prefs) {
  OxideQWebPreferences* old = NULL;
  if (WebPreferences* o =
      static_cast<WebPreferences *>(view_->GetWebPreferences())) {
    old = o->api_handle();
  }

  if (!prefs) {
    prefs = new OxideQWebPreferences(adapterToQObject(this));
  } else if (!prefs->parent()) {
    prefs->setParent(adapterToQObject(this));
  }
  view_->SetWebPreferences(
      OxideQWebPreferencesPrivate::get(prefs)->preferences());

  if (!old) {
    return;
  }

  if (old->parent() == adapterToQObject(this)) {
    delete old;
  }
}

void WebViewAdapter::updateWebPreferences() {
  view_->UpdateWebPreferences();
}

float WebViewAdapter::compositorFrameDeviceScaleFactor() const {
  return view_->compositor_frame_metadata().device_scale_factor;
}

float WebViewAdapter::compositorFramePageScaleFactor() const {
  return view_->compositor_frame_metadata().page_scale_factor;
}

QPoint WebViewAdapter::compositorFrameScrollOffsetPix() {
  if (frame_metadata_dirty_flags_ & FRAME_METADATA_CHANGE_SCROLL_OFFSET) {
    frame_metadata_dirty_flags_ &= ~FRAME_METADATA_CHANGE_SCROLL_OFFSET;

    // See https://launchpad.net/bugs/1336730
    const gfx::SizeF& viewport_size =
        view_->compositor_frame_metadata().scrollable_viewport_size;
    float x_scale = GetFrameMetadataScaleToPix() *
                    viewport_size.width() / qRound(viewport_size.width());
    float y_scale = GetFrameMetadataScaleToPix() *
                    viewport_size.height() / qRound(viewport_size.height());

    gfx::Vector2dF offset =
        gfx::ScaleVector2d(view_->compositor_frame_metadata().root_scroll_offset,
                           x_scale, y_scale);
    frame_scroll_offset_ = QPointF(offset.x(), offset.y()).toPoint();
  }

  return frame_scroll_offset_;
}

QSize WebViewAdapter::compositorFrameContentSizePix() {
  if (frame_metadata_dirty_flags_ & FRAME_METADATA_CHANGE_CONTENT) {
    frame_metadata_dirty_flags_ &= ~FRAME_METADATA_CHANGE_CONTENT;

    // See https://launchpad.net/bugs/1336730
    const gfx::SizeF& viewport_size =
        view_->compositor_frame_metadata().scrollable_viewport_size;
    float x_scale = GetFrameMetadataScaleToPix() *
                    viewport_size.width() / qRound(viewport_size.width());
    float y_scale = GetFrameMetadataScaleToPix() *
                    viewport_size.height() / qRound(viewport_size.height());

    gfx::SizeF size =
        gfx::ScaleSize(view_->compositor_frame_metadata().root_layer_size,
                       x_scale, y_scale);
    frame_content_size_ = QSizeF(size.width(), size.height()).toSize();
  }

  return frame_content_size_;
}

QSize WebViewAdapter::compositorFrameViewportSizePix() {
  if (frame_metadata_dirty_flags_ & FRAME_METADATA_CHANGE_VIEWPORT) {
    frame_metadata_dirty_flags_ &= ~FRAME_METADATA_CHANGE_VIEWPORT;
    gfx::SizeF size =
        gfx::ScaleSize(view_->compositor_frame_metadata().scrollable_viewport_size,
                       GetFrameMetadataScaleToPix());
    frame_viewport_size_ = QSizeF(size.width(), size.height()).toSize();
  }

  return frame_viewport_size_;
}

QSharedPointer<CompositorFrameHandle> WebViewAdapter::compositorFrameHandle() {
  if (!compositor_frame_) {
    const cc::CompositorFrameMetadata& metadata =
        view_->compositor_frame_metadata();
    compositor_frame_ =
        QSharedPointer<CompositorFrameHandle>(new CompositorFrameHandleImpl(
          view_->GetCompositorFrameHandle(),
          metadata.device_scale_factor *
            metadata.location_bar_content_translation.y()));
  }

  return compositor_frame_;
}

void WebViewAdapter::didCommitCompositorFrame() {
  view_->DidCommitCompositorFrame();
}

void WebViewAdapter::setCanTemporarilyDisplayInsecureContent(bool allow) {
  view_->SetCanTemporarilyDisplayInsecureContent(allow);
}

void WebViewAdapter::setCanTemporarilyRunInsecureContent(bool allow) {
  view_->SetCanTemporarilyRunInsecureContent(allow);
}

OxideQSecurityStatus* WebViewAdapter::securityStatus() {
  return view_->qsecurity_status();
}

ContentTypeFlags WebViewAdapter::blockedContent() const {
  COMPILE_ASSERT(
      CONTENT_TYPE_NONE ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_NONE),
      content_type_flags_none_doesnt_match);
  COMPILE_ASSERT(
      CONTENT_TYPE_MIXED_DISPLAY ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_MIXED_DISPLAY),
      content_type_flags_mixed_display_doesnt_match);
  COMPILE_ASSERT(
      CONTENT_TYPE_MIXED_SCRIPT ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_MIXED_SCRIPT),
      content_type_flags_mixed_script_doesnt_match);

  return static_cast<ContentTypeFlags>(view_->blocked_content());
}

void WebViewAdapter::prepareToClose() {
  view_->PrepareToClose();
}

int WebViewAdapter::locationBarMaxHeight() {
  return qRound(view_->GetLocationBarMaxHeightDip() *
                view_->GetDeviceScaleFactor());
}

int WebViewAdapter::locationBarOffsetPix() {
  if (frame_metadata_dirty_flags_ & FRAME_METADATA_CHANGE_CONTROLS_OFFSET) {
    frame_metadata_dirty_flags_ &= ~FRAME_METADATA_CHANGE_CONTROLS_OFFSET;
    const cc::CompositorFrameMetadata& metadata =
        view_->compositor_frame_metadata();
    location_bar_offset_ =
        qRound(metadata.location_bar_offset.y() *
               metadata.device_scale_factor);
  }

  return location_bar_offset_;
}

int WebViewAdapter::locationBarContentOffsetPix() {
  if (frame_metadata_dirty_flags_ & FRAME_METADATA_CHANGE_CONTENT_OFFSET) {
    frame_metadata_dirty_flags_ &= ~FRAME_METADATA_CHANGE_CONTENT_OFFSET;
    const cc::CompositorFrameMetadata& metadata =
        view_->compositor_frame_metadata();
    location_bar_content_offset_ =
        qRound(metadata.location_bar_content_translation.y() *
               metadata.device_scale_factor);
  }

  return location_bar_content_offset_;
}

} // namespace qt
} // namespace oxide
