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

#ifndef _OXIDE_SHARED_BROWSER_INCOMING_MESSAGE_H_
#define _OXIDE_SHARED_BROWSER_INCOMING_MESSAGE_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"

#include "shared/browser/oxide_message_dispatcher_browser.h"

namespace oxide {

class WebFrame;

class IncomingMessage FINAL {
 public:
  IncomingMessage(const MessageDispatcherBrowser::V8Message& message);
    
  void Reply(const std::string& args);
  void Error(const std::string& msg);

  WebFrame* frame() const { return frame_.get(); }
  std::string world_id() const { return world_id_; }
  std::string args() const { return args_; }

 private:
  base::WeakPtr<WebFrame> frame_;
  std::string world_id_;
  int serial_;
  std::string args_;
  base::WeakPtr<MessageDispatcherBrowser> source_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(IncomingMessage);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_INCOMING_MESSAGE_H_
