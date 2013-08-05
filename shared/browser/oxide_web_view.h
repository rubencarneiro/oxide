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

#ifndef _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
#define _OXIDE_SHARED_BROWSER_WEB_VIEW_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_delegate.h"

#include "shared/browser/oxide_script_executor_host.h"

class GURL;

namespace gfx {
class Size;
}

namespace content {

struct OpenURLParams;
class WebContents;

} // namespace content

namespace oxide {

class WebContentsViewDelegate;

// This is the main webview class
class WebView : public content::WebContentsDelegate {
 public:
  virtual ~WebView();

  const GURL& GetURL() const;
  void SetURL(const GURL& url);

  std::string GetTitle() const;

  bool CanGoBack() const;
  bool CanGoForward() const;

  void GoBack();
  void GoForward();

  void Stop();
  void Reload();

  bool IsIncognito() const;

  bool IsLoading() const;

  void UpdateSize(const gfx::Size& size);
  void Shown();
  void Hidden();

  void ExecuteScript(
      const std::string& code,
      bool all_frames,
      int run_at,
      bool in_main_world,
      const std::string& isolated_world_name,
      const ScriptExecutorHost::ExecuteScriptCallback& callback);

  void SetWebContentsViewDelegate(WebContentsViewDelegate* delegate);

 protected:
  WebView(WebContentsViewDelegate* delegate);
  bool Init(bool incognito, const gfx::Size& initial_size);

  content::WebContents* web_contents() const {
    return web_contents_.get();
  }

 private:
  void NavigationStateChanged(const content::WebContents* source,
                              unsigned changed_flags) FINAL;

  virtual void OnURLChanged();
  virtual void OnTitleChanged();
  virtual void OnLoadingChanged();
  virtual void OnCommandsUpdated();

  scoped_ptr<content::WebContents> web_contents_;
  WebContentsViewDelegate* web_contents_view_delegate_;
  ScriptExecutorHost script_executor_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
