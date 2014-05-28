// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2012 The Chromium Authors
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

#include "content/browser/renderer_host/render_sandbox_host_linux.h"

#include <sys/socket.h>
#include <unistd.h>

#include <utility>

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/path_service.h"
#include "base/posix/eintr_wrapper.h"
#include "base/process/launch.h"
#include "content/public/common/content_switches.h"

#include "shared/common/oxide_constants.h"

namespace content {

RenderSandboxHostLinux::RenderSandboxHostLinux() :
    initialized_(false),
    renderer_socket_(0),
    childs_lifeline_fd_(0),
    pid_(-1) {}

RenderSandboxHostLinux::~RenderSandboxHostLinux() {
  if (initialized_) {
    if (IGNORE_EINTR(close(renderer_socket_)) < 0)
      PLOG(ERROR) << "close";
    if (IGNORE_EINTR(close(childs_lifeline_fd_)) < 0)
      PLOG(ERROR) << "close";
  }
}

// static
RenderSandboxHostLinux* RenderSandboxHostLinux::GetInstance() {
  return Singleton<RenderSandboxHostLinux>::get();
}

void RenderSandboxHostLinux::Init(const std::string& sandbox_path) {
  DCHECK(!initialized_);
  initialized_ = true;

  int fds[2];
  CHECK(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fds) == 0);
  renderer_socket_ = fds[0];

  int pipefds[2];
  CHECK(0 == pipe(pipefds));
  childs_lifeline_fd_ = pipefds[1];

  base::FilePath exe;
  CHECK(PathService::Get(base::FILE_EXE, &exe));
  base::CommandLine cmd_line(exe);

  cmd_line.AppendSwitchASCII(switches::kProcessType, switches::kSandboxIPCProcess);
  if (!sandbox_path.empty()) {
    cmd_line.AppendSwitchASCII(switches::kSandboxExe, sandbox_path.c_str());
  }

  base::FileHandleMappingVector fds_to_map;
  fds_to_map.push_back(std::make_pair(fds[1], oxide::kSandboxIPCSocketPairFd));
  fds_to_map.push_back(std::make_pair(pipefds[0], oxide::kSandboxIPCLifelinePipeFd));

  base::LaunchOptions options;
  // allow_new_privs defaults to false, which causes LaunchProcess to call
  // prctl(PR_SET_NO_NEW_PRIVS...) in the child. However, the Sandbox IPC
  // process relies on executing the suid sandbox for mapping socket inode
  // numbers to PID's on behalf of the zygote, and PR_SET_NO_NEW_PRIVS breaks
  // this
  options.allow_new_privs = true;
  options.fds_to_remap = &fds_to_map;
  base::LaunchProcess(cmd_line.argv(), options, &pid_);
  CHECK(pid_ != -1);
}

} // namespace content
