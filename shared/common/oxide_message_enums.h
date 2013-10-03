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

#ifndef _OXIDE_SHARED_COMMON_MESSAGE_ENUMS_H_
#define _OXIDE_SHARED_COMMON_MESSAGE_ENUMS_H_

struct OxideMsg_SendMessage_Type {
 public:
  enum Value {
    Message,
    Reply
  };

  static bool is_valid(int type) {
    return type == Message || type == Reply;
  }
};

struct OxideMsg_SendMessage_Error {
 public:
  enum Value {
    OK,

    // The frame ID or world name were invalid
    INVALID_DESTINATION,

    // The message handler threw an error
    UNCAUGHT_EXCEPTION,

    // No handler was registered for this message
    NO_HANDLER,

    // The handler reported an error
    HANDLER_REPORTED_ERROR,

    // The frame disappeared before sending a response was sent
    // (only valid for embedder to content script messages, and
    //  never actually sent across the wire)
    FRAME_DISAPPEARED,

    // The message could not be delivered
    // (only valid for content script to embedder messages)
    UNDELIVERABLE = 1000
  };
};

#endif // _OXIDE_SHARED_COMMON_MESSAGE_ENUMS_H_
