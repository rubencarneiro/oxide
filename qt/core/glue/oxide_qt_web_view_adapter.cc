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

  CompositorFrameHandle::Type GetType() Q_DECL_FINAL {
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

  const QSize& GetSize() const Q_DECL_FINAL {
    return size_;
  }

  QImage GetSoftwareFrame() Q_DECL_FINAL {
    DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_SOFTWARE);
    return QImage(
        static_cast<uchar *>(frame_->software_frame_data()->pixels()),
        frame_->size_in_pixels().width(),
        frame_->size_in_pixels().height(),
        QImage::Format_ARGB32);
  }

  AcceleratedFrameData GetAcceleratedFrame() Q_DECL_FINAL {
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

void WebViewAdapter::WebPreferencesDestroyed() {
  OnWebPreferencesChanged();
}

WebViewAdapter::WebViewAdapter(QObject* q) :
    AdapterBase(q),
    priv(WebView::Create(this)),
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

  priv->Init(&params);
}

QUrl WebViewAdapter::url() const {
  return QUrl(QString::fromStdString(priv->GetURL().spec()));
}

void WebViewAdapter::setUrl(const QUrl& url) {
  priv->SetURL(GURL(url.toString().toStdString()));
}

QString WebViewAdapter::title() const {
  return QString::fromStdString(priv->GetTitle());
}

bool WebViewAdapter::canGoBack() const {
  return priv->CanGoBack();
}

bool WebViewAdapter::canGoForward() const {
  return priv->CanGoForward();
}

bool WebViewAdapter::incognito() const {
  if (construct_props_) {
    return construct_props_->incognito;
  }

  return priv->IsIncognito();  
}

void WebViewAdapter::setIncognito(bool incognito) {
  if (!construct_props_) {
    qWarning() << "Cannot change incognito mode after WebView is initialized";
    return;
  }

  construct_props_->incognito = incognito;
}

bool WebViewAdapter::loading() const {
  return priv->IsLoading();
}

bool WebViewAdapter::fullscreen() const {
  return priv->IsFullscreen();
}

void WebViewAdapter::setFullscreen(bool fullscreen) {
  priv->SetIsFullscreen(fullscreen);
}

WebFrameAdapter* WebViewAdapter::rootFrame() const {
  WebFrame* frame = static_cast<WebFrame *>(priv->GetRootFrame());
  if (!frame) {
    return NULL;
  }

  return frame->adapter();
}

WebContextAdapter* WebViewAdapter::context() const {
  if (construct_props_) {
    return construct_props_->context;
  }

  WebContext* context =
      WebContext::FromBrowserContext(priv->GetBrowserContext());
  if (!context) {
    return NULL;
  }

  return WebContextAdapter::FromWebContext(context);
}

void WebViewAdapter::setContext(WebContextAdapter* context) {
  DCHECK(construct_props_);
  construct_props_->context = context;
}

void WebViewAdapter::wasResized() {
  priv->WasResized();
}

void WebViewAdapter::visibilityChanged() {
  priv->VisibilityChanged();
}

void WebViewAdapter::handleFocusEvent(QFocusEvent* event) {
  priv->HandleFocusEvent(event);
}

void WebViewAdapter::handleInputMethodEvent(QInputMethodEvent* event) {
  priv->HandleInputMethodEvent(event);
}

void WebViewAdapter::handleKeyEvent(QKeyEvent* event) {
  priv->HandleKeyEvent(event);
}

void WebViewAdapter::handleMouseEvent(QMouseEvent* event) {
  priv->HandleMouseEvent(event);
}

void WebViewAdapter::handleTouchEvent(QTouchEvent* event) {
  priv->HandleTouchEvent(event);
}

void WebViewAdapter::handleWheelEvent(QWheelEvent* event) {
  priv->HandleWheelEvent(event);
}

QVariant WebViewAdapter::inputMethodQuery(Qt::InputMethodQuery query) const {
  return priv->InputMethodQuery(query);
}

void WebViewAdapter::goBack() {
  priv->GoBack();
}

void WebViewAdapter::goForward() {
  priv->GoForward();
}

void WebViewAdapter::stop() {
  priv->Stop();
}

void WebViewAdapter::reload() {
  priv->Reload();
}

void WebViewAdapter::loadHtml(const QString& html, const QUrl& baseUrl) {
  QByteArray encodedData = html.toUtf8().toPercentEncoding();
  priv->LoadData(std::string(encodedData.constData(), encodedData.length()),
                 "text/html;charset=UTF-8",
                 GURL(baseUrl.toString().toStdString()));
}

bool WebViewAdapter::isInitialized() {
  return priv->GetWebContents() != NULL;
}

int WebViewAdapter::getNavigationEntryCount() const {
  return priv->GetNavigationEntryCount();
}

int WebViewAdapter::getNavigationCurrentEntryIndex() const {
  return priv->GetNavigationCurrentEntryIndex();
}

void WebViewAdapter::setNavigationCurrentEntryIndex(int index) {
  priv->SetNavigationCurrentEntryIndex(index);
}

int WebViewAdapter::getNavigationEntryUniqueID(int index) const {
  return priv->GetNavigationEntryUniqueID(index);
}

QUrl WebViewAdapter::getNavigationEntryUrl(int index) const {
  return QUrl(QString::fromStdString(
      priv->GetNavigationEntryUrl(index).spec()));
}

QString WebViewAdapter::getNavigationEntryTitle(int index) const {
  return QString::fromStdString(priv->GetNavigationEntryTitle(index));
}

QDateTime WebViewAdapter::getNavigationEntryTimestamp(int index) const {
  return QDateTime::fromMSecsSinceEpoch(
      priv->GetNavigationEntryTimestamp(index).ToJsTime());
}

OxideQWebPreferences* WebViewAdapter::preferences() {
  WebPreferences* prefs =
      static_cast<WebPreferences *>(priv->GetWebPreferences());
  if (!prefs) {
    OxideQWebPreferences* p = new OxideQWebPreferences(adapterToQObject(this));
    prefs = OxideQWebPreferencesPrivate::get(p)->preferences();
    priv->SetWebPreferences(prefs);
  } else if (!prefs->api_handle()) {
    OxideQWebPreferencesPrivate::Adopt(prefs, adapterToQObject(this));
  }

  return prefs->api_handle();
}

void WebViewAdapter::setPreferences(OxideQWebPreferences* prefs) {
  OxideQWebPreferences* old = NULL;
  if (WebPreferences* o =
      static_cast<WebPreferences *>(priv->GetWebPreferences())) {
    old = o->api_handle();
  }
 
  if (prefs && !prefs->parent()) { 
    prefs->setParent(adapterToQObject(this));
  }

  WebPreferences* p = NULL;
  if (prefs) {
    p = OxideQWebPreferencesPrivate::get(prefs)->preferences();
  }
  priv->SetWebPreferences(p);

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

  rd->view = priv->AsWeakPtr();
  created_with_new_view_request_ = true;
}

void WebViewAdapter::updateWebPreferences() {
  priv->UpdateWebPreferences();
}

float WebViewAdapter::compositorFrameDeviceScaleFactor() const {
  return priv->compositor_frame_metadata().device_scale_factor;
}

float WebViewAdapter::compositorFramePageScaleFactor() const {
  return priv->compositor_frame_metadata().page_scale_factor;
}

QPointF WebViewAdapter::compositorFrameScrollOffset() const {
  const gfx::Vector2dF& offset =
      priv->compositor_frame_metadata().root_scroll_offset;
  return QPointF(offset.x(), offset.y());
}

QSizeF WebViewAdapter::compositorFrameLayerSize() const {
  const gfx::SizeF& size = priv->compositor_frame_metadata().root_layer_size;
  return QSizeF(size.width(), size.height());
}

QSizeF WebViewAdapter::compositorFrameViewportSize() const {
  const gfx::SizeF& size =
      priv->compositor_frame_metadata().scrollable_viewport_size;
  return QSizeF(size.width(), size.height());
}

QSharedPointer<CompositorFrameHandle> WebViewAdapter::compositorFrameHandle() {
  QSharedPointer<CompositorFrameHandle> handle(
      new CompositorFrameHandleImpl(priv->GetCompositorFrameHandle()));
  return handle;
}

void WebViewAdapter::didCommitCompositorFrame() {
  priv->DidCommitCompositorFrame();
}

void WebViewAdapter::setCanTemporarilyDisplayInsecureContent(bool allow) {
  if (!(priv->blocked_content() & oxide::CONTENT_TYPE_MIXED_DISPLAY) &&
      allow) {
    qWarning() << "Can only set webview to temporarily display insecure "
                  "content when the content has been blocked";
    return;
  }

  priv->SetCanTemporarilyDisplayInsecureContent(allow);
}

void WebViewAdapter::setCanTemporarilyRunInsecureContent(bool allow) {
  if (!(priv->blocked_content() & oxide::CONTENT_TYPE_MIXED_SCRIPT) &&
      allow) {
    qWarning() << "Can only set webview to temporarily run insecure "
                  "content when the content has been blocked";
    return;
  }

  priv->SetCanTemporarilyRunInsecureContent(allow);
}

OxideQSecurityStatus* WebViewAdapter::securityStatus() {
  return priv->qsecurity_status();
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

  return static_cast<ContentTypeFlags>(priv->blocked_content());
}

} // namespace qt
} // namespace oxide
