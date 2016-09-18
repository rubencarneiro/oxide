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

#ifndef _OXIDE_SHARED_BROWSER_DEVTOOLS_MANAGER_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_DEVTOOLS_MANAGER_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/public/browser/devtools_manager_delegate.h"

namespace oxide {

class DevToolsManagerDelegate : public content::DevToolsManagerDelegate {
 public:
  DevToolsManagerDelegate();
  ~DevToolsManagerDelegate() override;

 private:
  // DevToolsManagerelegate overrides.
  std::string GetDiscoveryPageHTML() override;
  std::string GetFrontendResource(const std::string& path) override;

  DISALLOW_COPY_AND_ASSIGN(DevToolsManagerDelegate);
};

}

#endif  // _OXIDE_SHARED_BROWSER_DEVTOOLS_MANAGER_DELEGATE_H_

