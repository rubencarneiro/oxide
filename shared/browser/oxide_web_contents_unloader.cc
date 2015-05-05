// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_web_contents_unloader.h"

#include <algorithm>

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "content/public/browser/web_contents.h"

namespace oxide {

WebContentsUnloader::WebContentsUnloader() {}

void WebContentsUnloader::CloseContents(content::WebContents* contents) {
  ScopedVector<content::WebContents>::iterator it =
      std::find(contents_unloading_.begin(),
                contents_unloading_.end(),
                contents);
  DCHECK(it != contents_unloading_.end());

  contents_unloading_.erase(it);

  if (contents_unloading_.size() != 0 || wait_loop_quit_closure_.is_null()) {
    return;
  }

  wait_loop_quit_closure_.Run();
}

WebContentsUnloader::~WebContentsUnloader() {
  DCHECK_EQ(contents_unloading_.size(), 0U);
}

// static
WebContentsUnloader* WebContentsUnloader::GetInstance() {
  return Singleton<WebContentsUnloader>::get();
}

void WebContentsUnloader::Unload(scoped_ptr<content::WebContents> contents) {
  if (!contents->NeedToFireBeforeUnload()) {
    // Despite the name, this checks if sudden termination is allowed. If so,
    // we shouldn't fire the unload handler particularly if this was script
    // closed, else we'll never get an ACK
    return;
  }

  content::WebContents* c = contents.get();

  // So we can intercept CloseContents
  contents->SetDelegate(this);
  contents_unloading_.push_back(contents.release());

  c->ClosePage();
  // Note: |c| might be deleted at this point
}

void WebContentsUnloader::WaitForPendingUnloadsToFinish() {
  CHECK(!base::MessageLoop::current()->is_running());

  if (contents_unloading_.empty()) {
    return;
  }

  base::RunLoop run_loop;
  wait_loop_quit_closure_ = run_loop.QuitClosure();

  run_loop.Run();
}

}
