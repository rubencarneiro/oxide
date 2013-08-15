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

#ifndef _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_
#define _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/content_renderer_client.h"

namespace oxide {

class ProcessObserver;
class UserScriptSlave;

class ContentRendererClient FINAL : public content::ContentRendererClient {
 public:
  ContentRendererClient();
  ~ContentRendererClient();

  UserScriptSlave* user_script_slave() const {
    return user_script_slave_.get();
  }

  void RenderThreadStarted() FINAL;

  void RenderViewCreated(content::RenderView* render_view) FINAL;

 private:
  scoped_ptr<ProcessObserver> process_observer_;
  scoped_ptr<UserScriptSlave> user_script_slave_;

  DISALLOW_COPY_AND_ASSIGN(ContentRendererClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_
