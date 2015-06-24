// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_REQUEST_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_REQUEST_H_

#include <string>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "v8/include/v8.h"

#include "shared/common/oxide_script_message_request.h"
#include "shared/renderer/oxide_script_referenced_object.h"
#include "shared/renderer/oxide_v8_scoped_persistent.h"

namespace oxide {

class ScriptMessageManager;

class ScriptMessageRequestImplRenderer
    : public base::RefCounted<ScriptMessageRequestImplRenderer>,
      public ScriptMessageRequest,
      public ScriptReferencedObject<ScriptMessageRequestImplRenderer> {
 public:
  ScriptMessageRequestImplRenderer(ScriptMessageManager* mm,
                                   int serial,
                                   const v8::Handle<v8::Object>& handle);

  v8::Handle<v8::Function> GetOnReplyCallback(v8::Isolate* isolate) const;
  void SetOnReplyCallback(v8::Isolate* isolate,
                          v8::Handle<v8::Function> callback);

  v8::Handle<v8::Function> GetOnErrorCallback(v8::Isolate* isolate) const;
  void SetOnErrorCallback(v8::Isolate* isolate,
                          v8::Handle<v8::Function> callback);

 private:
  friend class base::RefCounted<ScriptMessageRequestImplRenderer>;
  ~ScriptMessageRequestImplRenderer();

  void DispatchResponse(v8::Handle<v8::Function> function,
                        int argc,
                        v8::Local<v8::Value> argv[]);

  // ScriptMessageRequest implementation
  void OnReply(const base::Value& payload) override;
  void OnError(ScriptMessageParams::Error error,
               const base::Value& payload) override;

  ScopedPersistent<v8::Function> reply_callback_;
  ScopedPersistent<v8::Function> error_callback_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageRequestImplRenderer);
};

} // namespace

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_REQUEST_H_
