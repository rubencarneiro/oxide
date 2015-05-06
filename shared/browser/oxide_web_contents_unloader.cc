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

#include <map>
#include <algorithm>

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"

class WebContentsUnloaderObserver : public content::WebContentsObserver {
public:
  explicit WebContentsUnloaderObserver(
          WebContentsUnloaderObserver* contents_observer) {
    content::WebContents* contents = contents_observer->web_contents();
    contents_observer->Observe(nullptr);
    Observe(contents);
  }
  explicit WebContentsUnloaderObserver(
          content::WebContents* contents)
      : content::WebContentsObserver(contents) {}
  virtual ~WebContentsUnloaderObserver() {}

  void RenderProcessGone(base::TerminationStatus status) override {
    //    DLOG(ERROR) << "renderer process gone: " << status;
    if (status == base::TERMINATION_STATUS_ABNORMAL_TERMINATION
        || status == base::TERMINATION_STATUS_PROCESS_WAS_KILLED
        || status == base::TERMINATION_STATUS_PROCESS_CRASHED) {
      if (web_contents() && web_contents()->GetDelegate()) {
        web_contents()->GetDelegate()->CloseContents(web_contents());
      }
    }
    //  FILE* f = fopen("./dddd.log", "a+");
    // fprintf(f, "renderer process gone %d\n", status);
    // fclose(f);
  }
};

namespace {

typedef std::map<
  content::WebContents*,
  std::shared_ptr<WebContentsUnloaderObserver> > WebContensObserverMap;

WebContensObserverMap g_webcontents_observer_map;

WebContensObserverMap::iterator findObserverFor(
      content::WebContents* contents)
{
  WebContensObserverMap::iterator it =
    g_webcontents_observer_map.begin();
  for (; it != g_webcontents_observer_map.end(); it++) {
    if (it->first == contents) {
      break;
    }
  }
  return it;
}

void addObserverForWebContents(content::WebContents* contents)
{
  if (contents == nullptr) {
    return;
  }
  DCHECK(findObserverFor(contents) == g_webcontents_observer_map.end());
  g_webcontents_observer_map[contents] =
      std::make_shared<WebContentsUnloaderObserver>(
          new WebContentsUnloaderObserver(contents));
}
  
void removeObserverFor(content::WebContents* contents)
{
  WebContensObserverMap::iterator it =
    findObserverFor(contents);
  if (it != g_webcontents_observer_map.end()) {
    g_webcontents_observer_map.erase(it);
  }
}

}


namespace oxide {

WebContentsUnloader::WebContentsUnloader()
{}

void WebContentsUnloader::CloseContents(content::WebContents* contents) {
  ScopedVector<content::WebContents>::iterator it =
      std::find(contents_unloading_.begin(),
                contents_unloading_.end(),
                contents);
  DCHECK(it != contents_unloading_.end());

  contents_unloading_.erase(it);

  removeObserverFor(contents);

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

#include <stdio.h>

void WebContentsUnloader::Unload(scoped_ptr<content::WebContents> contents) {
  content::RenderViewHost* rvh = contents->GetRenderViewHost();
  if (!rvh) {
    return;
  }

  content::WebContents* web_contents = contents.release();

  addObserverForWebContents(web_contents);

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
