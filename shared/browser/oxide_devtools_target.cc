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
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"

using content::DevToolsAgentHost;

namespace oxide {

namespace {

const char kTargetTypePage[] = "page";
const char kTargetTypeIframe[] = "iframe";
const char kTargetTypeOther[] = "other";

}


DevToolsTarget::DevToolsTarget(content::WebContents* contents)
    : content::WebContentsObserver(contents),
      agent_host_(DevToolsAgentHost::GetOrCreateFor(contents)) {
  DCHECK(contents);
}

DevToolsTarget::~DevToolsTarget() {}

std::string DevToolsTarget::GetId() const {
  return agent_host_.get() ? agent_host_->GetId() : std::string();
}

std::string DevToolsTarget::GetParentId() const {
  return std::string();
}

std::string DevToolsTarget::GetType() const {
  return kTargetTypePage;
}

std::string DevToolsTarget::GetTitle() const {
  const content::WebContents* wc = web_contents();
  if (!wc) {
    return std::string();
  }

  return base::UTF16ToUTF8(wc->GetTitle());
}

std::string DevToolsTarget::GetDescription() const {
  return std::string();
}

GURL DevToolsTarget::GetURL() const {
  const content::WebContents* wc = web_contents();
  if (!wc) {
    return GURL();
  }

  return wc->GetLastCommittedURL();
}

GURL DevToolsTarget::GetFaviconURL() const {
  return GURL();
}

base::TimeTicks DevToolsTarget::GetLastActivityTime() const {
  const content::WebContents* wc = web_contents();
  if (!wc) {
    return base::TimeTicks();
  }

  return wc->GetLastActiveTime();
}

bool DevToolsTarget::IsAttached() const {
  return agent_host_->IsAttached();
}

scoped_refptr<DevToolsAgentHost> DevToolsTarget::GetAgentHost() const {
  return agent_host_;
}

bool DevToolsTarget::Activate() const {
  content::WebContents* wc = web_contents();
  if (!wc) {
    return false;
  }

  wc->GetDelegate()->ActivateContents(wc);
  return true;
}

bool DevToolsTarget::Close() const {
  NOTIMPLEMENTED();
  return false;
}

}
