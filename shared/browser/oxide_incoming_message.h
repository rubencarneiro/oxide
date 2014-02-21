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
#include "url/gurl.h"

namespace oxide {

class WebFrame;

class IncomingMessage FINAL {
 public:
  IncomingMessage(WebFrame* source_frame,
                  int serial,
                  const GURL& context,
                  const std::string& msg_id,
                  const std::string& args);
    
  void Reply(const std::string& args);
  void Error(const std::string& msg);

  WebFrame* source_frame() const { return source_frame_.get(); }
  int serial() const { return serial_; }
  GURL context() const { return context_; }
  std::string msg_id() const { return msg_id_; }
  std::string args() const { return args_; }

 private:
  base::WeakPtr<WebFrame> source_frame_;
  int serial_;
  GURL context_;
  std::string msg_id_;
  std::string args_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(IncomingMessage);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_INCOMING_MESSAGE_H_
