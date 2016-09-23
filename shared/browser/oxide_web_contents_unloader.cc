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
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"

#include "oxide_browser_context.h"
#include "oxide_web_view_contents_helper.h"

namespace oxide {

namespace {

void MaybeDestroyOffTheRecordContext(BrowserContext* context) {
  DCHECK(context->IsOffTheRecord());

  if (WebViewContentsHelper::IsContextInUse(context)) {
    return;
  }

  BrowserContext::DestroyOffTheRecordContextForContext(
      context->GetOriginalContext());
}

}

class WebContentsUnloaderObserver : public content::WebContentsObserver {
 public:
  explicit WebContentsUnloaderObserver(content::WebContents* contents)
      : content::WebContentsObserver(contents) {}
  ~WebContentsUnloaderObserver() override {}

 private:
  // content::WebContentsObserver implementation
  void RenderProcessGone(base::TerminationStatus status) override {
    web_contents()->GetDelegate()->CloseContents(web_contents());
  }

  void WebContentsDestroyed() {
    delete this;
  }
};

WebContentsUnloader::WebContentsUnloader() = default;

void WebContentsUnloader::CloseContents(content::WebContents* contents) {
  auto it = std::find_if(
      contents_unloading_.begin(),
      contents_unloading_.end(),
      [contents](const std::unique_ptr<content::WebContents>& c) {
    return contents == c.get();
  });
  DCHECK(it != contents_unloading_.end());

  contents_unloading_.erase(it);
}

WebContentsUnloader::~WebContentsUnloader() = default;

// static
WebContentsUnloader* WebContentsUnloader::GetInstance() {
  return base::Singleton<WebContentsUnloader>::get();
}

void WebContentsUnloader::Unload(
    std::unique_ptr<content::WebContents> contents) {
  content::WebContents* c = contents.get();
  contents_unloading_.push_back(std::move(contents));

  content::BrowserContext* context = c->GetBrowserContext();
  if (context->IsOffTheRecord()) {
    MaybeDestroyOffTheRecordContext(BrowserContext::FromContent(context));
  }

  if (!c->NeedToFireBeforeUnload()) {
    // Despite the name, this checks if sudden termination is allowed. If so,
    // we shouldn't fire the unload handler particularly if this was script
    // closed, else we'll never get an ACK
    CloseContents(c);
    // |c| is invalid now
    return;
  }

  // To intercept render process crashes
  new WebContentsUnloaderObserver(c);

  // So we can intercept CloseContents
  c->SetDelegate(this);

  c->ClosePage();
  // Note: |c| might be deleted at this point
}

void WebContentsUnloader::Shutdown() {
  contents_unloading_.clear();
}

bool WebContentsUnloader::IsUnloadInProgress(content::WebContents* contents) {
  return std::find_if(
      contents_unloading_.begin(),
      contents_unloading_.end(),
      [contents](const std::unique_ptr<content::WebContents>& c) {
    return contents == c.get();
  }) != contents_unloading_.end();
}

}
