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

#include "oxide_user_script_scheduler.h"

#include "base/bind.h"
#include "base/threading/thread_task_runner_handle.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"

#include "grit/oxide_resources.h"

#include "shared/common/oxide_user_script.h"

#include "oxide_user_script_slave.h"

namespace oxide {

void UserScriptScheduler::DoIdleInject() {
  idle_posted_ = false;

  for (std::set<blink::WebLocalFrame *>::const_iterator it =
           pending_idle_frames_.begin();
       it != pending_idle_frames_.end(); ++it) {
    blink::WebLocalFrame* frame = *it;
    UserScriptSlave::GetInstance()->InjectScripts(frame,
                                                  UserScript::DOCUMENT_IDLE);
  }

  pending_idle_frames_.clear();
}

UserScriptScheduler::UserScriptScheduler(content::RenderView* render_view) :
    content::RenderViewObserver(render_view),
    idle_posted_(false),
    weak_factory_(this) {}

void UserScriptScheduler::OnDestruct() {
  delete this;
}

void UserScriptScheduler::DidFinishLoad(blink::WebLocalFrame* frame) {
  pending_idle_frames_.insert(frame);

  if (idle_posted_) {
    return;
  }

  idle_posted_ = true;

  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&UserScriptScheduler::DoIdleInject,
                 weak_factory_.GetWeakPtr()));
}

void UserScriptScheduler::FrameDetached(blink::WebFrame* frame) {
  if (!idle_posted_) {
    return;
  }

  blink::WebLocalFrame* local_frame = frame->toWebLocalFrame();
  if (local_frame) {
    pending_idle_frames_.erase(local_frame);
  }
}

} // namespace oxide
