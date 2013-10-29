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

#ifndef _OXIDE_QT_CORE_GLUE_PRIVATE_OUTGOING_MESSAGE_REQUEST_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_PRIVATE_OUTGOING_MESSAGE_REQUEST_ADAPTER_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"

#include "shared/browser/oxide_outgoing_message_request.h"

namespace oxide {
namespace qt {

class OutgoingMessageRequestAdapter;
class WebFrameAdapterPrivate;

class OutgoingMessageRequestAdapterPrivate FINAL {
 public:
  static OutgoingMessageRequestAdapterPrivate* Create(
      OutgoingMessageRequestAdapter* adapter);

  oxide::OutgoingMessageRequest& request() {
    return request_;
  }

  void RemoveFromOwner();

  base::WeakPtr<OutgoingMessageRequestAdapterPrivate> GetWeakPtr();

  static OutgoingMessageRequestAdapterPrivate* get(
      OutgoingMessageRequestAdapter* adapter);

 private:
  friend class OutgoingMessageRequestAdapter;
  friend class WebFrameAdapterPrivate;

  OutgoingMessageRequestAdapterPrivate(OutgoingMessageRequestAdapter* adapter);
  void ReceiveReplyCallback(const std::string& args);
  void ReceiveErrorCallback(int error, const std::string& msg);

  oxide::OutgoingMessageRequest request_;
  OutgoingMessageRequestAdapter* a;
  WebFrameAdapterPrivate* frame;
  base::WeakPtrFactory<OutgoingMessageRequestAdapterPrivate> weak_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(OutgoingMessageRequestAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_OUTGOING_MESSAGE_REQUEST_ADAPTER_H_
