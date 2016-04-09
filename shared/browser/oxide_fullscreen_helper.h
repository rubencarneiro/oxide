// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_FULLSCREEN_HELPER_H_
#define _OXIDE_SHARED_BROWSER_FULLSCREEN_HELPER_H_

#include "base/macros.h"
#include "content/public/browser/web_contents_user_data.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class FullscreenHelperClient;

class OXIDE_SHARED_EXPORT FullscreenHelper
    : public content::WebContentsUserData<FullscreenHelper> {
 public:
  ~FullscreenHelper() override;

  static FullscreenHelper* FromWebContents(content::WebContents* contents);

  void set_client(FullscreenHelperClient* client) {
    client_ = client;
  }

  bool fullscreen_granted() const { return fullscreen_granted_; }

  // Set whether fullscreen has been granted by the client
  void SetFullscreenGranted(bool granted);

  // Whether the tab is fullscreen - ie, fullscreen has been requested and
  // granted
  bool IsFullscreen() const;

  // Request the client to enter fullscreen
  void EnterFullscreenMode(const GURL& origin);

  // Request the client to exit fullscreen
  void ExitFullscreenMode();

 private:
  friend class content::WebContentsUserData<FullscreenHelper>;

  FullscreenHelper(content::WebContents* contents);

  content::WebContents* web_contents_;

  FullscreenHelperClient* client_;

  // Whether fullscreen has been requested by content
  bool fullscreen_requested_;

  // Whether fullscreen has been granted by the client
  bool fullscreen_granted_;

  DISALLOW_COPY_AND_ASSIGN(FullscreenHelper);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_FULLSCREEN_HELPER_H_
