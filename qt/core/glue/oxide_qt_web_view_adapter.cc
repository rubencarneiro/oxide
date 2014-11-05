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

#include <QSize>
#include <QtDebug>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "cc/output/compositor_frame_metadata.h"
#include "ui/gfx/size.h"
#include "url/gurl.h"

#include "qt/core/api/oxideqnewviewrequest_p.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/core/api/oxideqwebpreferences_p.h"
#include "qt/core/browser/oxide_qt_web_context.h"
#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/browser/oxide_qt_web_preferences.h"
#include "qt/core/browser/oxide_qt_web_view.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/browser/oxide_content_types.h"

#include "oxide_qt_web_context_adapter.h"
#include "oxide_qt_web_frame_adapter.h"

namespace oxide {
namespace qt {

class CompositorFrameHandleImpl : public CompositorFrameHandle {
 public:
  CompositorFrameHandleImpl(oxide::CompositorFrameHandle* frame)
      : frame_(frame) {
    if (frame_.get()) {
      size_ = QSize(frame_->size_in_pixels().width(),
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

  const QSize& GetSize() const final {
    return size_;
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
  QSize size_;
};

void WebViewAdapter::Initialized() {
  DCHECK(isInitialized());

  OnInitialized(construct_props_->incognito,
                construct_props_->context);
  construct_props_.reset();
}

WebViewAdapter::WebViewAdapter(QObject* q) :
    AdapterBase(q),
    view_(WebView::Create(this)),
    construct_props_(new ConstructProperties()),
    created_with_new_view_request_(false) {}

WebViewAdapter::~WebViewAdapter() {}

void WebViewAdapter::init() {
  if (created_with_new_view_request_ || isInitialized()) {
    return;
  }

  oxide::WebView::Params params;
  params.context =
      WebContext::FromAdapter(construct_props_->context)->GetContext();
  params.incognito = construct_props_->incognito;

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
  if (construct_props_) {
    return construct_props_->incognito;
  }

  return view_->IsIncognito();  
}

void WebViewAdapter::setIncognito(bool incognito) {
  if (!construct_props_) {
    qWarning() << "Cannot change incognito mode after WebView is initialized";
    return;
  }

  construct_props_->incognito = incognito;
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
  if (construct_props_) {
    return construct_props_->context;
  }

  return WebContextAdapter::FromWebContext(view_->GetContext());
}

void WebViewAdapter::setContext(WebContextAdapter* context) {
  DCHECK(construct_props_);
  construct_props_->context = context;
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

bool WebViewAdapter::isInitialized() {
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
  WebPreferences* prefs =
      static_cast<WebPreferences *>(view_->GetWebPreferences());
  if (!prefs) {
    OxideQWebPreferences* p = new OxideQWebPreferences(adapterToQObject(this));
    prefs = OxideQWebPreferencesPrivate::get(p)->preferences();
    view_->SetWebPreferences(prefs);
  } else if (!prefs->api_handle()) {
    OxideQWebPreferencesPrivate::Adopt(prefs, adapterToQObject(this));
  }

  return prefs->api_handle();
}

void WebViewAdapter::setPreferences(OxideQWebPreferences* prefs) {
  OxideQWebPreferences* old = NULL;
  if (WebPreferences* o =
      static_cast<WebPreferences *>(view_->GetWebPreferences())) {
    old = o->api_handle();
  }
 
  if (prefs && !prefs->parent()) { 
    prefs->setParent(adapterToQObject(this));
  }

  WebPreferences* p = NULL;
  if (prefs) {
    p = OxideQWebPreferencesPrivate::get(prefs)->preferences();
  }
  view_->SetWebPreferences(p);

  if (!old) {
    return;
  }

  if (old->parent() == adapterToQObject(this)) {
    delete old;
  }
}

void WebViewAdapter::setRequest(OxideQNewViewRequest* request) {
  if (isInitialized()) {
    qWarning() << "Cannot assign NewViewRequest to an already constructed WebView";
    return;
  }

  if (created_with_new_view_request_) {
    return;
  }

  OxideQNewViewRequestPrivate* rd = OxideQNewViewRequestPrivate::get(request);
  if (rd->view) {
    qWarning() << "Cannot assign NewViewRequest to more than one WebView";
    return;
  }

  rd->view = view_->AsWeakPtr();
  created_with_new_view_request_ = true;
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

QPointF WebViewAdapter::compositorFrameScrollOffset() const {
  const gfx::Vector2dF& offset =
      view_->compositor_frame_metadata().root_scroll_offset;
  return QPointF(offset.x(), offset.y());
}

QSizeF WebViewAdapter::compositorFrameLayerSize() const {
  const gfx::SizeF& size = view_->compositor_frame_metadata().root_layer_size;
  return QSizeF(size.width(), size.height());
}

QSizeF WebViewAdapter::compositorFrameViewportSize() const {
  const gfx::SizeF& size =
      view_->compositor_frame_metadata().scrollable_viewport_size;
  return QSizeF(size.width(), size.height());
}

QSharedPointer<CompositorFrameHandle> WebViewAdapter::compositorFrameHandle() {
  QSharedPointer<CompositorFrameHandle> handle(
      new CompositorFrameHandleImpl(view_->GetCompositorFrameHandle()));
  return handle;
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

} // namespace qt
} // namespace oxide
