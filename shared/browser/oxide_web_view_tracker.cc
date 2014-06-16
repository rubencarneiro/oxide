// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#include "oxide_web_view_tracker.h"

#include "base/memory/singleton.h"
#include "shared/browser/oxide_web_view.h"

namespace oxide {

WebViewTracker* WebViewTracker::GetInstance() {
  return Singleton<WebViewTracker>::get();
}

void WebViewTracker::add(
    BrowserContext* context, WebView* view) {
  if (!context || !view)
    return;
  if (!exists(context)) {
    WebViewList wvl;
    wvl.insert(view);
    web_view_per_context_[context] = wvl;
  }
  else {
    web_view_per_context_[context].insert(view);
  }  
}

WebViewTracker::WebViewList WebViewTracker::get(
    BrowserContext* context) {
  if (!exists(context)) {
    return WebViewList();
  }
  return web_view_per_context_.find(context)->second;
}

void WebViewTracker::remove(BrowserContext* context, WebView* view) {
  if (!exists(context)) {
    return;
  }
  WebViewList wvl = get(context);
  WebViewList::iterator it = wvl.find(view);
  if (wvl.end() == it) {
    return;
  }
  wvl.erase(it);
  web_view_per_context_[context] = wvl;
}

void WebViewTracker::remove(BrowserContext* context) {
  if (!exists(context)) {
    return;
  }
  web_view_per_context_.erase(
    web_view_per_context_.find(context));
}

bool WebViewTracker::exists(BrowserContext* context) const {
  return web_view_per_context_.find(context) != web_view_per_context_.end();
}

}
