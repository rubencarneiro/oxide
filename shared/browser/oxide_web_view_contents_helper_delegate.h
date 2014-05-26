// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_WEB_VIEW_CONTENTS_HELPER_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_WEB_VIEW_CONTENTS_HELPER_DELEGATE_H_

#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "ui/base/window_open_disposition.h"

class GURL;

namespace content {
struct FileChooserParams;
struct OpenURLParams;
class WebContents;
}

namespace gfx {
class Rect;
}

namespace oxide {

class WebViewContentsHelperDelegate {
 public:
  virtual ~WebViewContentsHelperDelegate() {}

  virtual content::WebContents* OpenURL(const content::OpenURLParams& params) = 0;

  virtual void NavigationStateChanged(unsigned flags) = 0;

  virtual bool ShouldCreateWebContents(const GURL& target_url,
                                       WindowOpenDisposition disposition,
                                       bool user_gesture) = 0;

  virtual bool CreateNewViewAndAdoptWebContents(
      scoped_ptr<content::WebContents> contents,
      WindowOpenDisposition disposition,
      const gfx::Rect& initial_pos) = 0;

  virtual void LoadProgressChanged(double progress) = 0;

  virtual void AddMessageToConsole(int32 level,
                                   const base::string16& message,
                                   int32 line_no,
                                   const base::string16& source_id) = 0;

  virtual bool RunFileChooser(const content::FileChooserParams& params) = 0;

  virtual void ToggleFullscreenMode(bool enter) = 0;

  virtual void NotifyWebPreferencesDestroyed() = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_CONTENTS_HELPER_DELEGATE_H_
