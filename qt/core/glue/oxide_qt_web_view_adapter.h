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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_VIEW_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_WEB_VIEW_ADAPTER_H_

#include <QDateTime>
#include <QList>
#include <QRect>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_adapter_base.h"
#include "qt/core/glue/oxide_qt_javascript_dialog_delegate.h"

QT_BEGIN_NAMESPACE
class QKeyEvent;
class QSize;
QT_END_NAMESPACE

class OxideQGeolocationPermissionRequest;
class OxideQLoadEvent;
class OxideQNavigationRequest;
class OxideQNewViewRequest;
class OxideQWebPreferences;

namespace oxide {
namespace qt {

class FilePickerDelegate;
class ScriptMessageHandlerAdapter;
class WebContextAdapter;
class WebFrameAdapter;
class WebPopupMenuDelegate;
class WebView;

class Q_DECL_EXPORT WebViewAdapter : public AdapterBase {
 public:
  virtual ~WebViewAdapter();

  void init();

  QUrl url() const;
  void setUrl(const QUrl& url);

  QString title() const;

  bool canGoBack() const;
  bool canGoForward() const;

  bool incognito() const;
  void setIncognito(bool incognito);

  bool loading() const;

  bool fullscreen() const;
  void setFullscreen(bool fullscreen);

  WebFrameAdapter* rootFrame() const;

  WebContextAdapter* context() const;
  void setContext(WebContextAdapter* context);

  void updateSize(const QSize& size);
  void updateVisibility(bool visible);

  void goBack();
  void goForward();
  void stop();
  void reload();
  void loadHtml(const QString& html, const QUrl& baseUrl);

  QList<ScriptMessageHandlerAdapter *>& message_handlers() {
    return message_handlers_;
  }

  bool isInitialized();

  int getNavigationEntryCount() const;
  int getNavigationCurrentEntryIndex() const;
  void setNavigationCurrentEntryIndex(int index);
  int getNavigationEntryUniqueID(int index) const;
  QUrl getNavigationEntryUrl(int index) const;
  QString getNavigationEntryTitle(int index) const;
  QDateTime getNavigationEntryTimestamp(int index) const;

  OxideQWebPreferences* preferences();
  void setPreferences(OxideQWebPreferences* prefs);

  void setRequest(OxideQNewViewRequest* request);

 protected:
  WebViewAdapter(QObject* q);

 private:
  friend class WebView;

  struct ConstructProperties {
    ConstructProperties() :
        incognito(false),
        context(NULL) {}

    bool incognito;
    WebContextAdapter* context;
  };

  void Initialized();
  void WebPreferencesDestroyed();

  virtual WebPopupMenuDelegate* CreateWebPopupMenuDelegate() = 0;
  virtual JavaScriptDialogDelegate* CreateJavaScriptDialogDelegate(
      JavaScriptDialogDelegate::Type type) = 0;
  virtual JavaScriptDialogDelegate* CreateBeforeUnloadDialogDelegate() = 0;
  virtual FilePickerDelegate* CreateFilePickerDelegate() = 0;

  virtual void OnInitialized(bool orig_incognito,
                             WebContextAdapter* orig_context) = 0;

  virtual void URLChanged() = 0;
  virtual void TitleChanged() = 0;
  virtual void CommandsUpdated() = 0;

  virtual void LoadProgressChanged(double progress) = 0;

  virtual void LoadEvent(OxideQLoadEvent* event) = 0;

  virtual void NavigationEntryCommitted() = 0;
  virtual void NavigationListPruned(bool from_front, int count) = 0;
  virtual void NavigationEntryChanged(int index) = 0;

  virtual WebFrameAdapter* CreateWebFrame() = 0;

  virtual QRect GetContainerBounds() = 0;

  virtual bool IsVisible() const = 0;

  virtual void AddMessageToConsole(int level,
                                   const QString& message,
                                   int line_no,
                                   const QString& source_id) = 0;

  virtual void ToggleFullscreenMode(bool enter) = 0;

  virtual void OnWebPreferencesChanged() = 0;

  virtual void FrameAdded(WebFrameAdapter* frame) = 0;
  virtual void FrameRemoved(WebFrameAdapter* frame) = 0;

  virtual bool CanCreateWindows() const = 0;

  virtual void NavigationRequested(OxideQNavigationRequest* request) = 0;
  virtual void NewViewRequested(OxideQNewViewRequest* request) = 0;

  virtual void RequestGeolocationPermission(
      OxideQGeolocationPermissionRequest* request) = 0;

  virtual void HandleKeyboardEvent(QKeyEvent* event) = 0;

  QScopedPointer<WebView> priv;
  QList<ScriptMessageHandlerAdapter *> message_handlers_;
  QScopedPointer<ConstructProperties> construct_props_;
  bool created_with_new_view_request_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_VIEW_ADAPTER_H_
