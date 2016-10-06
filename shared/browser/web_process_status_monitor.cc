// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "web_process_status_monitor.h"

#include "base/process/kill.h"
#include "content/public/browser/web_contents.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(WebProcessStatusMonitor);

WebProcessStatusMonitor::WebProcessStatusMonitor(content::WebContents* contents)
    : content::WebContentsObserver(contents),
      renderer_is_unresponsive_(false),
      last_status_(GetStatus()) {}

void WebProcessStatusMonitor::StatusUpdated() {
  Status status = GetStatus();
  if (last_status_ == status) {
    return;
  }

  last_status_ = status;
  callback_list_.Notify();
}

void WebProcessStatusMonitor::RenderViewReady() {
  StatusUpdated();
}

void WebProcessStatusMonitor::RenderProcessGone(base::TerminationStatus status) {
  StatusUpdated();
}

// static
WebProcessStatusMonitor* WebProcessStatusMonitor::FromWebContents(
    content::WebContents* contents) {
  return content::WebContentsUserData<WebProcessStatusMonitor>::FromWebContents(contents);
}

WebProcessStatusMonitor::~WebProcessStatusMonitor() = default;

WebProcessStatusMonitor::Status WebProcessStatusMonitor::GetStatus() const {
  if (web_contents()->IsCrashed()) {
    switch (web_contents()->GetCrashedStatus()) {
      case base::TERMINATION_STATUS_PROCESS_WAS_KILLED:
        return Status::Killed;
      default:
        // Map all other termination statuses to crashed. This is
        // consistent with how the sad tab helper works in Chrome.
        return Status::Crashed;
    }
  }

  if (renderer_is_unresponsive_) {
    return Status::Unresponsive;
  }

  return Status::Running;
}

std::unique_ptr<WebProcessStatusMonitor::Subscription>
WebProcessStatusMonitor::AddChangeCallback(const base::Closure& cb) {
  return callback_list_.Add(cb);
}

void WebProcessStatusMonitor::RendererIsResponsive() {
  renderer_is_unresponsive_ = false;
  StatusUpdated();
}

void WebProcessStatusMonitor::RendererIsUnresponsive() {
  renderer_is_unresponsive_ = true;
  StatusUpdated();
}

} // namespace oxide
