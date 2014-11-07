// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_APP_CONTENT_MAIN_DELEGATE_H_
#define _OXIDE_SHARED_APP_CONTENT_MAIN_DELEGATE_H_

#include "base/macros.h"
#include "content/public/app/content_main_delegate.h"

namespace oxide {

class ContentClient;

class ContentMainDelegate : public content::ContentMainDelegate {
 public:
  virtual ~ContentMainDelegate();

  // content::ContentMainDelegate implementation
  bool BasicStartupComplete(int* exit_code) final;

  void PreSandboxStartup() final;

  int RunProcess(
      const std::string& process_type,
      const content::MainFunctionParams& main_function_params) final;

  void ProcessExiting(const std::string& process_type) final;

  content::ContentBrowserClient* CreateContentBrowserClient() final;
  content::ContentRendererClient* CreateContentRendererClient() final;

 protected:
  // Allow access to default constructor only from derived classes
  ContentMainDelegate();

 private:
  DISALLOW_COPY_AND_ASSIGN(ContentMainDelegate);
};

} // namespace oxide

#endif // _OXIDE_SHARED_APP_CONTENT_MAIN_DELEGATE_H_
