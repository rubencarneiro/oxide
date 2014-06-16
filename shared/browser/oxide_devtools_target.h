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

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/devtools_target.h"

namespace content {
  class DevToolsTarget;
  class DevToolsAgentHost;
  class RenderViewHost;
  class WebContents;
}

namespace oxide {

class DevtoolsTarget
    : public content::DevToolsTarget {
 public:

  virtual ~DevtoolsTarget();

  static DevtoolsTarget * CreateForRenderViewHost(content::RenderViewHost *);

  // DevToolsHttpProtocolHandler::Delegate overrides.
  virtual std::string GetId() const OVERRIDE;
  virtual std::string GetParentId() const OVERRIDE;
  virtual std::string GetType() const OVERRIDE;
  virtual std::string GetTitle() const OVERRIDE;
  virtual std::string GetDescription() const OVERRIDE;
  virtual GURL GetURL() const OVERRIDE;
  virtual GURL GetFaviconURL() const OVERRIDE;
  virtual base::TimeTicks GetLastActivityTime() const OVERRIDE;
  virtual bool IsAttached() const OVERRIDE;
  virtual scoped_refptr<content::DevToolsAgentHost> GetAgentHost() const OVERRIDE;
  virtual bool Activate() const OVERRIDE;
  virtual bool Close() const OVERRIDE;

 private:

  content::WebContents* GetWebContents();
  const content::WebContents* GetWebContents() const;
  bool IsValidRenderViewHost(content::RenderViewHost * rvh) const;

  scoped_refptr<content::DevToolsAgentHost> agent_host_;
  content::RenderViewHost* rvh_;

  DevtoolsTarget(content::RenderViewHost * rvh);
  DISALLOW_COPY_AND_ASSIGN(DevtoolsTarget);
};

}

#endif  // _OXIDE_SHARED_BROWSER_DEVTOOLS_HTTP_HANDLER_DELEGATE_H_

