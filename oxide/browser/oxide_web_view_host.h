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

#ifndef _OXIDE_BROWSER_WEB_VIEW_HOST_H_
#define _OXIDE_BROWSER_WEB_VIEW_HOST_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"

#include "common/oxide_core_export.h"

class GURL;

namespace gfx {
class Size;
}

namespace content {

struct OpenURLParams;
class WebContents;

} // namespace content

namespace oxide {

class OXIDE_CORE_EXPORT WebViewHost :
    public content::WebContentsDelegate,
    public content::WebContentsObserver {
 public:
  virtual ~WebViewHost();

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
  void WasShown();
  void WasHidden();

  content::WebContents* web_contents() const;

 protected:
  WebViewHost();
  bool Init(bool incognito, const gfx::Size& initial_size);

 private:
  void NavigationStateChanged(const content::WebContents* source,
                              unsigned changed_flags) FINAL;

  virtual void OnURLChanged();
  virtual void OnTitleChanged();
  virtual void OnLoadingChanged();
  virtual void OnCommandsUpdated();

  scoped_ptr<content::WebContents> web_contents_;

  DISALLOW_COPY_AND_ASSIGN(WebViewHost);
};

} // namespace oxide

#endif // _OXIDE_BROWSER_WEB_VIEW_HOST_H_
