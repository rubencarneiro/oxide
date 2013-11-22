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

#include "base/memory/shared_memory.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_start.h"

#include "shared/common/oxide_message_enums.h"

IPC_ENUM_TRAITS(OxideMsg_SendMessage_Type::Value)
IPC_ENUM_TRAITS(OxideMsg_SendMessage_Error::Value)

IPC_STRUCT_BEGIN(OxideMsg_SendMessage_Params)
  IPC_STRUCT_MEMBER(long long, frame_id)
  IPC_STRUCT_MEMBER(std::string, world_id)
  IPC_STRUCT_MEMBER(int, serial)
  IPC_STRUCT_MEMBER(OxideMsg_SendMessage_Type::Value, type)
  IPC_STRUCT_MEMBER(OxideMsg_SendMessage_Error::Value, error)
  IPC_STRUCT_MEMBER(std::string, msg_id)
  IPC_STRUCT_MEMBER(std::string, args)
IPC_STRUCT_END()

#define IPC_MESSAGE_START OxideMsgStart

IPC_MESSAGE_CONTROL1(OxideMsg_SetIsIncognitoProcess,
                     bool)

IPC_MESSAGE_CONTROL1(OxideMsg_UpdateUserScripts,
                     base::SharedMemoryHandle)

IPC_MESSAGE_ROUTED1(OxideHostMsg_SendMessage,
                    OxideMsg_SendMessage_Params)

IPC_MESSAGE_ROUTED1(OxideMsg_SendMessage,
                    OxideMsg_SendMessage_Params)

IPC_MESSAGE_ROUTED2(OxideHostMsg_FrameCreated,
                    long long /* parent */,
                    long long /* frame */)
