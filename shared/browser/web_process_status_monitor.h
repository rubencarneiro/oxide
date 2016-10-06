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

#ifndef _OXIDE_SHARED_BROWSER_WEB_PROCESS_STATUS_MONITOR_H_
#define _OXIDE_SHARED_BROWSER_WEB_PROCESS_STATUS_MONITOR_H_

#include <memory>

#include "base/callback.h"
#include "base/callback_list.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

// A helper class for tracking web process status. This gets its own class to
// avoid duplicating the same logic in a couple of places
class OXIDE_SHARED_EXPORT WebProcessStatusMonitor
    : public content::WebContentsUserData<WebProcessStatusMonitor>,
      public content::WebContentsObserver {
 public:
  static WebProcessStatusMonitor* FromWebContents(
      content::WebContents* contents);

  ~WebProcessStatusMonitor() override;

  enum class Status {
    Running,
    Killed,
    Crashed,
    Unresponsive
  };

  Status GetStatus() const;

  using Subscription = base::CallbackList<void()>::Subscription;
  std::unique_ptr<Subscription> AddChangeCallback(const base::Closure& cb);

  void RendererIsResponsive();
  void RendererIsUnresponsive();

 private:
  friend class content::WebContentsUserData<WebProcessStatusMonitor>;
  WebProcessStatusMonitor(content::WebContents* contents);

  void StatusUpdated();

  // content::WebContentsObserver
  void RenderViewReady() override;
  void RenderProcessGone(base::TerminationStatus status) override;

  bool renderer_is_unresponsive_;

  Status last_status_;

  base::CallbackList<void()> callback_list_;

  DISALLOW_COPY_AND_ASSIGN(WebProcessStatusMonitor);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_PROCESS_STATUS_MONITOR_H_
