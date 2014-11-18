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

#include <limits>

#include <QSize>
#include <QtDebug>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/pickle.h"
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
#include "shared/browser/oxide_browser_process_main.h"

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

WebViewAdapter::WebViewAdapter(QObject* q) :
    AdapterBase(q),
    view_(WebView::Create(this)) {}

WebViewAdapter::~WebViewAdapter() {}

void WebViewAdapter::init(bool incognito,
                          WebContextAdapter* context,
                          OxideQNewViewRequest* new_view_request,
                          const QByteArray& restoreState) {
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

  if (!restoreState.isEmpty()) {
    std::vector<sessions::SerializedNavigationEntry> entries;
    Pickle pickle(restoreState.data(), restoreState.size());
    PickleIterator i(pickle);
    int count;
    if (!i.ReadLength(&count)) {
      qWarning() << "Failed to read initial state: invalid data";
      return;
    }
    entries.resize(count);
    for (int j = 0; j < count; ++j) {
      sessions::SerializedNavigationEntry entry;
      if (!entry.ReadFromPickle(&i)) {
        qWarning() << "Failed to read initial state: invalid data";
        return;
      }
      entries[j] = entry;
    }
    int index;
    if (!i.ReadInt(&index)) {
      qWarning() << "Failed to read initial state: invalid data";
      return;
    }
    view_->SetState(entries, index);
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

QByteArray WebViewAdapter::currentState() const {
  std::vector<sessions::SerializedNavigationEntry> entries = view_->GetState();
  if (entries.size() == 0) {
    return QByteArray();
  }
  Pickle pickle;
  pickle.WriteInt(entries.size());
  std::vector<sessions::SerializedNavigationEntry>::const_iterator i;
  static const size_t max_state_size = std::numeric_limits<uint16>::max() - 1024;
  for (i = entries.begin(); i != entries.end(); ++i) {
    i->WriteToPickle(max_state_size, &pickle);
  }
  pickle.WriteInt(view_->GetNavigationCurrentEntryIndex());
  return QByteArray(static_cast<const char*>(pickle.data()), pickle.size());
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
