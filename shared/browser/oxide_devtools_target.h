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

#ifndef _OXIDE_SHARED_BROWSER_DEVTOOLS_TARGET_H_
#define _OXIDE_SHARED_BROWSER_DEVTOOLS_TARGET_H_

#include "base/basictypes.h"
#include "base/macros.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {
class DevToolsAgentHost;
class WebContents;
}

namespace oxide {

class DevToolsTarget
    : public content::DevToolsTarget,
      public content::WebContentsObserver {
 public:
  DevToolsTarget(content::WebContents* contents);
  virtual ~DevToolsTarget();

  // content::DevToolsTarget overrides.
  std::string GetId() const FINAL;
  std::string GetParentId() const FINAL;
  std::string GetType() const FINAL;
  std::string GetTitle() const FINAL;
  std::string GetDescription() const FINAL;
  GURL GetURL() const FINAL;
  GURL GetFaviconURL() const FINAL;
  base::TimeTicks GetLastActivityTime() const FINAL;
  bool IsAttached() const FINAL;
  scoped_refptr<content::DevToolsAgentHost> GetAgentHost() const FINAL;
  bool Activate() const FINAL;
  bool Close() const FINAL;

 private:
  scoped_refptr<content::DevToolsAgentHost> agent_host_;

  DISALLOW_COPY_AND_ASSIGN(DevToolsTarget);
};

}

#endif  // _OXIDE_SHARED_BROWSER_DEVTOOLS_HTTP_HANDLER_DELEGATE_H_

