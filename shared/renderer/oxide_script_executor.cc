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

#include "oxide_script_executor.h"

#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/v8_value_converter.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebVector.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScriptSource.h"
#include "third_party/WebKit/public/web/WebSecurityOrigin.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "v8/include/v8.h"

namespace oxide {

namespace {
// FIXME: Need to clean this up on shutdown
std::map<std::string, int>* g_isolated_world_ids;

const char kIsolatedWorldCSP[] = "script-src 'self'";
const char kIsolatedWorldOriginProtocol[] = "oxide-world://";
}

// static
int ScriptExecutor::GetIsolatedWorldID(const std::string& name,
                                       WebKit::WebFrame* frame) {
  static int next_isolated_world_id = 1;

  if (!g_isolated_world_ids) {
    g_isolated_world_ids = new std::map<std::string, int>();
  }

  std::map<std::string, int>& isolated_world_ids = *g_isolated_world_ids;

  std::string url(kIsolatedWorldOriginProtocol);
  url.append(name);

  std::map<std::string, int>::iterator it = isolated_world_ids.find(name);
  if (it != isolated_world_ids.end()) {
    frame->setIsolatedWorldSecurityOrigin(
        it->second,
        WebKit::WebSecurityOrigin::createFromString(base::UTF8ToUTF16(url)));
    frame->setIsolatedWorldContentSecurityPolicy(
        it->second,
        WebKit::WebString::fromUTF8(kIsolatedWorldCSP));
    return it->second;
  }

  int new_id = next_isolated_world_id++;
  isolated_world_ids[name] = new_id;

  frame->setIsolatedWorldSecurityOrigin(
      new_id,
      WebKit::WebSecurityOrigin::createFromString(base::UTF8ToUTF16(url)));
  frame->setIsolatedWorldContentSecurityPolicy(
      new_id,
      WebKit::WebString::fromUTF8(kIsolatedWorldCSP));

  return new_id;
}

void ScriptExecutor::OnExecuteScript(
    const OxideMsg_ExecuteScript_Params& params) {
  UserScript::RunLocation run_at =
      static_cast<UserScript::RunLocation>(params.run_at);
  if (run_at > current_location_) {
    pending_execution_map_[run_at].push(OxideMsg_ExecuteScript_Params(params));
    return;
  }

  ExecuteScript(params);
}

void ScriptExecutor::MaybeRun() {
  for (int i = UserScript::DOCUMENT_START; i <= current_location_; ++i) {
    UserScript::RunLocation run_at = static_cast<UserScript::RunLocation>(i);
    ExecutionQueue& q = pending_execution_map_[run_at];
    while (!q.empty()) {
      ExecuteScript(q.front());
      q.pop();
    }
  }
}

void ScriptExecutor::ExecuteScript(
    const OxideMsg_ExecuteScript_Params& params) {
  if (render_view()->GetPageId() != params.page_id) {
    render_view()->Send(new OxideHostMsg_ExecuteScriptFinished(
        render_view()->GetRoutingID(), params.request_id,
        "Page ID not current", base::ListValue()));
    return;
  }

  WebKit::WebFrame* frame =
      render_view()->GetWebView() ?
        render_view()->GetWebView()->mainFrame() : NULL;
  if (!frame) {
    render_view()->Send(new OxideHostMsg_ExecuteScriptFinished(
        render_view()->GetRoutingID(), params.request_id,
        "No main frame", base::ListValue()));
    return;
  }

  std::vector<WebKit::WebFrame *> frames;
  frames.push_back(frame);

  if (params.all_frames) {
    GetChildFrames(frame, frames);
  }

  WebKit::WebScriptSource source(WebKit::WebString::fromUTF8(params.code));
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope scope(isolate);

  scoped_ptr<content::V8ValueConverter> v8vc(
      content::V8ValueConverter::create());
  base::ListValue rv;

  for (std::vector<WebKit::WebFrame *>::iterator it = frames.begin();
       it != frames.end(); ++it) {
    WebKit::WebFrame* f = *it;

    v8::Handle<v8::Value> script_rv;

    if (params.in_main_world) {
      script_rv = f->executeScriptAndReturnValue(source);
    } else {
      WebKit::WebVector<v8::Local<v8::Value> > results;
      int id = GetIsolatedWorldID(params.isolated_world_name, f);
      f->executeScriptInIsolatedWorld(id, &source, 1, 0, &results);
      if (results.size() == 1 && !results[0].IsEmpty()) {
        script_rv = results[0];
      }
    }

    if (!script_rv.IsEmpty()) {
      v8::Local<v8::Context> context(v8::Context::New(isolate));
      base::Value* val = v8vc->FromV8Value(script_rv, context);
      rv.Append(val ? val : base::Value::CreateNullValue());
    } else {
      rv.Append(base::Value::CreateNullValue());
    }
  }

  render_view()->Send(new OxideHostMsg_ExecuteScriptFinished(
    render_view()->GetRoutingID(), params.request_id, std::string(), rv));
}

void ScriptExecutor::GetChildFrames(WebKit::WebFrame* frame,
                                    std::vector<WebKit::WebFrame *>& frames) {
  for (WebKit::WebFrame* child = frame->firstChild(); child;
       child = child->nextSibling()) {
    frames.push_back(child);
    GetChildFrames(child, frames);
  }
}

ScriptExecutor::ScriptExecutor(content::RenderView* render_view) :
    content::RenderViewObserver(render_view),
    current_location_(static_cast<UserScript::RunLocation>(-1)),
    weak_factory_(this) {
  for (int i = UserScript::DOCUMENT_START;
       i < UserScript::RUN_LOCATION_LAST;
       ++i) {
    pending_execution_map_[static_cast<UserScript::RunLocation>(i)] =
        ExecutionQueue();
  }
}

void ScriptExecutor::DidFinishDocumentLoad(WebKit::WebFrame* frame) {
  current_location_ = UserScript::DOCUMENT_END;
  MaybeRun();
}

void ScriptExecutor::DidFinishLoad(WebKit::WebFrame* frame) {
  current_location_ = UserScript::DOCUMENT_IDLE;

  base::MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(&ScriptExecutor::MaybeRun, weak_factory_.GetWeakPtr()));
}

void ScriptExecutor::DidStartProvisionalLoad(WebKit::WebFrame* frame) {
  current_location_ = static_cast<UserScript::RunLocation>(-1);
  for (ExecutionMap::iterator it = pending_execution_map_.begin();
       it != pending_execution_map_.end();
       ++it) {
    while (!it->second.empty()) {
      it->second.pop();
    }
  }
}

void ScriptExecutor::DidCreateDocumentElement(WebKit::WebFrame* frame) {
  current_location_ = UserScript::DOCUMENT_START;
  MaybeRun();
}

bool ScriptExecutor::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ScriptExecutor, message)
    IPC_MESSAGE_HANDLER(OxideMsg_ExecuteScript, OnExecuteScript)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

} // namespace oxide
