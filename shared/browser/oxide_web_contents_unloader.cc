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

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/singleton.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/supports_user_data.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"

class WebContentsUnloaderObserver : public content::WebContentsObserver,
                                    public base::SupportsUserData::Data {
public:
  explicit WebContentsUnloaderObserver(
          content::WebContents* contents)
      : content::WebContentsObserver(contents) {}
  virtual ~WebContentsUnloaderObserver() {
    DLOG(ERROR) << "********* Deleting";
  }

  void RenderProcessGone(base::TerminationStatus status) override {
    web_contents()->GetDelegate()->CloseContents(web_contents());
  }
};

namespace {

const char kWebContentsUnloaderObserverKey[] =
  "oxide_web_contents_unloader_observer_data";

}


namespace oxide {

WebContentsUnloader::WebContentsUnloader() {}

void WebContentsUnloader::CloseContents(content::WebContents* contents) {
  ScopedVector<content::WebContents>::iterator it =
      std::find(contents_unloading_.begin(),
                contents_unloading_.end(),
                contents);
  DCHECK(it != contents_unloading_.end());

  WebContentsUnloaderObserver* observer =
    static_cast<WebContentsUnloaderObserver*> (
      contents->GetUserData(kWebContentsUnloaderObserverKey));
  if (observer) {
    delete observer;
  }

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
  content::RenderViewHost* rvh = contents->GetRenderViewHost();
  if (!rvh) {
    return;
  }

  content::WebContents* web_contents = contents.release();

  web_contents->SetUserData(
      kWebContentsUnloaderObserverKey,
      new WebContentsUnloaderObserver(web_contents));

  // So we can intercept CloseContents
  contents->SetDelegate(this);

  contents_unloading_.push_back(web_contents);

  rvh->ClosePage();
  // Note: |rvh| might be deleted at this point
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
