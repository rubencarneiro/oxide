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

#ifndef _OXIDE_SHARED_BROWSER_SCRIPT_EXECUTOR_HOST_H_
#define _OXIDE_SHARED_BROWSER_SCRIPT_EXECUTOR_HOST_H_

#include <map>
#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/web_contents_observer.h"

namespace base {
class ListValue;
}

namespace content {
class WebContents;
}

namespace oxide {

class ScriptExecutorHost FINAL : public content::WebContentsObserver {
 public:
  typedef base::Callback<void(bool, const std::string&)> ExecuteScriptCallback;

  ScriptExecutorHost();

  void BeginObserving(content::WebContents* web_contents);

  void ExecuteScript(
      const std::string& code,
      bool all_frames,
      int run_at,
      bool in_main_world,
      const std::string& isolated_world_name,
      const ExecuteScriptCallback& callback);

  bool OnMessageReceived(const IPC::Message& message) FINAL;

 private:
  typedef std::map<int, ExecuteScriptCallback> PendingRequestMap;

  void OnExecuteScriptFinished(int request_id,
                               const std::string& error,
                               const base::ListValue& results);

  content::WebContents* web_contents_;
  PendingRequestMap pending_requests_;

  DISALLOW_COPY_AND_ASSIGN(ScriptExecutorHost);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SCRIPT_EXECUTOR_HOST_H_
