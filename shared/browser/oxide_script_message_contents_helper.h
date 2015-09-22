// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_CONTENTS_HELPER_H_
#define _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_CONTENTS_HELPER_H_

#include "base/macros.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace oxide {

class ScriptMessageContentsHelper
    : public content::WebContentsObserver,
      public content::WebContentsUserData<ScriptMessageContentsHelper> {
 public:
  ~ScriptMessageContentsHelper() override;

 private:
  friend class content::WebContentsUserData<ScriptMessageContentsHelper>;

  ScriptMessageContentsHelper(content::WebContents* web_contents);

  void OnReceiveScriptMessage(const IPC::Message& message,
                              content::RenderFrameHost* render_frame_observer);

  // content::WebContentsObserver implementation
  bool OnMessageReceived(const IPC::Message& message,
                         content::RenderFrameHost* render_frame_host) override;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageContentsHelper);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_CONTENTS_HELPER_H_
