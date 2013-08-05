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

#include <string>

#include "base/values.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_start.h"

#define IPC_MESSAGE_START OxideMsgStart

IPC_STRUCT_BEGIN(OxideMsg_ExecuteScript_Params)
  // Unique ID. Note that this is only unique within a RenderView
  IPC_STRUCT_MEMBER(int, request_id)

  // The current page ID (for avoiding races)
  IPC_STRUCT_MEMBER(int, page_id)

  // The actual JS code to inject
  IPC_STRUCT_MEMBER(std::string, code)

  // Inject the code in to all frames rather than just the root frame
  IPC_STRUCT_MEMBER(bool, all_frames)

  // When to inject the code
  IPC_STRUCT_MEMBER(int, run_at)

  // Execute the script in the main world, rather than isolated world
  IPC_STRUCT_MEMBER(bool, in_main_world)

  // A name for the isolated world in which to execute the script
  IPC_STRUCT_MEMBER(std::string, isolated_world_name)
IPC_STRUCT_END()

IPC_MESSAGE_ROUTED1(OxideMsg_ExecuteScript,
                    OxideMsg_ExecuteScript_Params)

IPC_MESSAGE_ROUTED3(OxideHostMsg_ExecuteScriptFinished,
                    int, /* Request ID */
                    std::string, /* Error string */
                    base::ListValue /* Script results */)
