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

#include "oxide_devtools_target.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/common/url_constants.h"

using content::DevToolsAgentHost;
using content::DevToolsTarget;
using content::RenderViewHost;
using content::WebContents;

namespace oxide {

// static 
DevtoolsTarget* DevtoolsTarget::CreateForRenderViewHost(
    content::RenderViewHost * rvh) {
  if (rvh) {
    return new DevtoolsTarget(rvh);
  }
  return 0;
}

DevtoolsTarget::DevtoolsTarget(
    content::RenderViewHost * rvh) : rvh_(rvh) {

  content::WebContents* web_contents = GetWebContents();
  if (web_contents) {
    agent_host_ =
      DevToolsAgentHost::GetOrCreateFor(web_contents);
  }
}

DevtoolsTarget::~DevtoolsTarget() {
}

bool DevtoolsTarget::IsValidRenderViewHost(
    content::RenderViewHost * rvh) const {
  if (!rvh) {
    return NULL;
  }
  std::vector<content::RenderViewHost*> rvhs =
    DevToolsAgentHost::GetValidRenderViewHosts();
  return std::find(rvhs.begin(),
		   rvhs.end(),
		   rvh) != rvhs.end();
}

content::WebContents* DevtoolsTarget::GetWebContents() {
  return IsValidRenderViewHost(rvh_) ?
    WebContents::FromRenderViewHost(rvh_)
    : NULL;
}

const content::WebContents*
DevtoolsTarget::GetWebContents() const {
  return IsValidRenderViewHost(rvh_) ?
    WebContents::FromRenderViewHost(rvh_)
    : NULL;
}

std::string DevtoolsTarget::GetId() const {
  return agent_host_ ? agent_host_->GetId() : std::string();
}

std::string DevtoolsTarget::GetParentId() const {
  return std::string();
}

std::string DevtoolsTarget::GetType() const {
  return std::string();
}

std::string DevtoolsTarget::GetTitle() const {
  const content::WebContents* web_contents = GetWebContents();
  if (!web_contents) {
    return std::string();
  }

  return base::UTF16ToUTF8(web_contents->GetTitle());
}

std::string DevtoolsTarget::GetDescription() const {
  return std::string();
}

GURL DevtoolsTarget::GetURL() const {
  const content::WebContents* web_contents = GetWebContents();
  if (!web_contents) {
    return GURL();
  }

  return web_contents->GetVisibleURL();
}

GURL DevtoolsTarget::GetFaviconURL() const {
  return GURL();
}

base::TimeTicks DevtoolsTarget::GetLastActivityTime() const {
  const content::WebContents* web_contents = GetWebContents();
  if (!web_contents) {
    return base::TimeTicks();
  }

  return web_contents->GetLastActiveTime();
}

bool DevtoolsTarget::IsAttached() const {
  return agent_host_->IsAttached();
}

scoped_refptr<DevToolsAgentHost> DevtoolsTarget::GetAgentHost() const {
  return agent_host_;
}

bool DevtoolsTarget::Activate() const {
  if (!rvh_) {
    return false;
  }

  content::WebContents* web_contents =
    WebContents::FromRenderViewHost(rvh_);
  if (!web_contents) {
    return false;
  }

  web_contents->GetDelegate()->ActivateContents(web_contents);
  return true;
}

bool DevtoolsTarget::Close() const {
  NOTIMPLEMENTED();
  return false;
}

}
