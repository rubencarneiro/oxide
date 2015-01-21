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

#include <string>

#include "oxide_devtools_manager_delegate.h"

#include "oxide_devtools_target.h"
#include "oxide_web_view.h"

namespace oxide {

DevToolsManagerDelegate::DevToolsManagerDelegate() {}

void DevToolsManagerDelegate::Inspect(
    content::BrowserContext* browser_context,
    content::DevToolsAgentHost* agent_host) {}

void DevToolsManagerDelegate::DevToolsAgentStateChanged(
    content::DevToolsAgentHost* agent_host,
    bool attached) {}

base::DictionaryValue* DevToolsManagerDelegate::HandleCommand(
    content::DevToolsAgentHost* agent_host,
    base::DictionaryValue* command) {
  return nullptr;
}

scoped_ptr<content::DevToolsTarget> DevToolsManagerDelegate::CreateNewTarget(
    const GURL& url) {
  return scoped_ptr<content::DevToolsTarget>();
}

void DevToolsManagerDelegate::EnumerateTargets(TargetCallback callback) {
  // There is currently a DevToolsHttpHandler per context, but we don't
  // know which one is our caller. Return all WebView's for now, but it
  // would be nice to be able to return only the ones belonging to the
  // context that owns the DevToolsHttpHandler again

  TargetList target_list;

  WebViewIterator iter = WebView::GetAllWebViews();
  while (iter.HasMore()) {
    target_list.push_back(
        new DevToolsTarget(iter.GetNext()->GetWebContents()));
  }

  callback.Run(target_list);
}

std::string DevToolsManagerDelegate::GetPageThumbnailData(const GURL& url) {
  return std::string();
}

} // namespace oxide
