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

#include "web_contents_id_tracker.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "content/public/browser/web_contents_observer.h"

namespace oxide {
namespace qt {

class WebContentsIDTracker::WebContentsObserver
    : public content::WebContentsObserver {
 public:
  WebContentsObserver(content::WebContents* contents)
      : content::WebContentsObserver(contents) {}

 private:
  // content::WebContentsObserver implementation
  void WebContentsDestroyed() override;
};

void WebContentsIDTracker::WebContentsObserver::WebContentsDestroyed() {
  WebContentsIDTracker::GetInstance()->RemoveWebContents(web_contents());
}

WebContentsIDTracker::WebContentsIDTracker() = default;

void WebContentsIDTracker::AddWebContents(content::WebContents* contents) {
  DCHECK(web_contents_set_.find(reinterpret_cast<WebContentsID>(contents)) ==
         web_contents_set_.end());
  web_contents_set_.insert(reinterpret_cast<WebContentsID>(contents));

  new WebContentsObserver(contents);
}

void WebContentsIDTracker::RemoveWebContents(content::WebContents* contents) {
  DCHECK(web_contents_set_.find(reinterpret_cast<WebContentsID>(contents)) !=
         web_contents_set_.end());
  size_t erased =
      web_contents_set_.erase(reinterpret_cast<WebContentsID>(contents));
  DCHECK_GT(erased, 0U);
}

// static
WebContentsIDTracker* WebContentsIDTracker::GetInstance() {
  return base::Singleton<WebContentsIDTracker>::get();
}

WebContentsID WebContentsIDTracker::GetIDForWebContents(
    content::WebContents* contents) {
  WebContentsID id = reinterpret_cast<WebContentsID>(contents);

  if (web_contents_set_.find(id) == web_contents_set_.end()) {
    AddWebContents(contents);
  }

  return id;
}

content::WebContents* WebContentsIDTracker::GetWebContentsFromID(
    WebContentsID id) {
  if (web_contents_set_.find(id) == web_contents_set_.end()) {
    return nullptr;
  }

  return reinterpret_cast<content::WebContents*>(id);
}

} // namespace qt
} // namespace oxide
