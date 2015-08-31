// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#include "oxide_render_process_initializer.h"

#include "content/public/browser/browser_context.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"

#include "oxide_user_agent_settings.h"
#include "oxide_user_script_master.h"

namespace oxide {

void RenderProcessInitializer::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  DCHECK_EQ(type, content::NOTIFICATION_RENDERER_PROCESS_CREATED);

  content::RenderProcessHost* process =
      content::Source<content::RenderProcessHost>(source).ptr();
  content::BrowserContext* context = process->GetBrowserContext();

  UserAgentSettings::Get(context)->RenderProcessCreated(process);
  UserScriptMaster::Get(context)->RenderProcessCreated(process);
}

RenderProcessInitializer::RenderProcessInitializer() {
  registrar_.Add(this, content::NOTIFICATION_RENDERER_PROCESS_CREATED,
                 content::NotificationService::AllBrowserContextsAndSources());
}

RenderProcessInitializer::~RenderProcessInitializer() {}

} // namespace oxide
