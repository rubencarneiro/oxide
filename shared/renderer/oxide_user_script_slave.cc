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

#include "oxide_user_script_slave.h"

#include <map>
#include <string>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/pickle.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/renderer/render_thread.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/platform/WebSecurityOrigin.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebURLRequest.h"
#include "third_party/WebKit/public/web/WebDataSource.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "third_party/WebKit/public/web/WebScriptSource.h"
#include "url/gurl.h"
#include "v8/include/v8.h"

#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_user_script.h"

#include "oxide_isolated_world_map.h"
#include "oxide_script_message_dispatcher_renderer.h"
#include "oxide_script_message_manager.h"

namespace oxide {

namespace {

UserScriptSlave* g_instance;

const char kIsolatedWorldCSP[] = "script-src 'self'";
const char kUserScriptHead[] = "(function (unsafeWindow) {\n";
const char kUserScriptTail[] = "\n})(window);";

}

UserScriptSlave::~UserScriptSlave() {
  CHECK(render_process_shutting_down_);
  DCHECK_EQ(g_instance, this);
  g_instance = nullptr;
}

// static
int UserScriptSlave::GetIsolatedWorldID(const GURL& url,
                                        blink::WebLocalFrame* frame) {
  int id = IsolatedWorldMap::IDFromURL(url);

  frame->setIsolatedWorldSecurityOrigin(
      id, blink::WebSecurityOrigin::createFromString(base::UTF8ToUTF16(url.spec())));
  frame->setIsolatedWorldContentSecurityPolicy(
      id, blink::WebString::fromUTF8(kIsolatedWorldCSP));

  return id;
}

void UserScriptSlave::OnUpdateUserScripts(base::SharedMemoryHandle handle) {
  user_scripts_.clear();

  scoped_ptr<base::SharedMemory> shmem(new base::SharedMemory(handle, true));

  CHECK(shmem->Map(sizeof(base::Pickle::Header)));
  base::Pickle::Header* header =
      reinterpret_cast<base::Pickle::Header *>(shmem->memory());
  int size = sizeof(base::Pickle::Header) + header->payload_size;
  shmem->Unmap();

  CHECK(shmem->Map(size));

  base::Pickle pickle(reinterpret_cast<char *>(shmem->memory()), size);
  base::PickleIterator iter(pickle);

  uint64_t num_scripts = 0;
  CHECK(iter.ReadUInt64(&num_scripts));
  for (; num_scripts > 0; --num_scripts) {
    linked_ptr<UserScript> script(new UserScript());
    user_scripts_.push_back(script);

    script->Unpickle(&iter);
  }
}

void UserScriptSlave::InjectGreaseMonkeyScriptInMainWorld(
    blink::WebLocalFrame* frame,
    const blink::WebScriptSource& script_source) {

  ScriptMessageDispatcherRenderer * dispatcher_renderer =
      ScriptMessageDispatcherRenderer::FromWebFrame(frame);
  DCHECK(dispatcher_renderer);

  linked_ptr<ScriptMessageManager> message_manager =
      dispatcher_renderer->ScriptMessageManagerForWorldId(kMainWorldId);
  if (!message_manager.get()) {
    LOG(ERROR) << "Could not get a proper message manager for frame: "
               << frame
               << " while trying to inject script in main world";
    return;
  }

  v8::Isolate* isolate = message_manager->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(message_manager->GetV8Context());

  v8::Local<v8::String> wrapped_script_head(
      v8::String::NewFromUtf8(isolate,
                              "(function(oxide) {\n"));

  v8::Local<v8::String> src(
      v8::String::NewFromUtf8(isolate,
                              script_source.code.utf8().c_str()));
  DCHECK(!src.IsEmpty() && src->Length() > 0);

  v8::Local<v8::String> wrapped_script_tail(
      v8::String::NewFromUtf8(isolate, "\n})"));

  v8::Local<v8::String> wrapped_src(
      v8::String::Concat(wrapped_script_head,
                         v8::String::Concat(src, wrapped_script_tail)));

  v8::Local<v8::Script> script(v8::Script::Compile(wrapped_src));

  v8::TryCatch try_catch;
  {
    blink::WebScopedMicrotaskSuppression mts;

    v8::Local<v8::Function> function(script->Run().As<v8::Function>());
    if (try_catch.HasCaught()) {
      LOG(ERROR) << "Caught exception when running script: "
                 << *v8::String::Utf8Value(try_catch.Message()->Get());
      return;
    }

    v8::Local<v8::Value> argv[] = {
        message_manager->GetOxideApiObject(message_manager->isolate())
    };

    frame->callFunctionEvenIfScriptDisabled(
        function,
        message_manager->GetV8Context()->Global(),
        arraysize(argv),
        argv);
  }

  if (try_catch.HasCaught()) {
    LOG(ERROR) << "Caught exception when calling script: "
               << *v8::String::Utf8Value(try_catch.Message()->Get());
    return;
  }
}

bool UserScriptSlave::OnControlMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(UserScriptSlave, message)
    IPC_MESSAGE_HANDLER(OxideMsg_UpdateUserScripts, OnUpdateUserScripts)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void UserScriptSlave::OnRenderProcessShutdown() {
  content::RenderThread::Get()->RemoveObserver(this);
  render_process_shutting_down_ = true;
  delete this;
}

// static
UserScriptSlave* UserScriptSlave::GetInstance() {
  DCHECK(g_instance);
  return g_instance;
}

UserScriptSlave::UserScriptSlave()
    : render_process_shutting_down_(false) {
  CHECK(!g_instance);
  g_instance = this;

  content::RenderThread::Get()->AddObserver(this);
}

void UserScriptSlave::InjectScripts(blink::WebLocalFrame* frame,
                                    UserScript::RunLocation location) {
  blink::WebDataSource* data_source = frame->provisionalDataSource() ?
      frame->provisionalDataSource() : frame->dataSource();
  CHECK(data_source);
  GURL data_source_url(data_source->request().url());
  if (data_source_url.is_empty()) {
    return;
  }

  for (Vector::iterator it = user_scripts_.begin();
       it != user_scripts_.end(); ++it) {
    linked_ptr<UserScript>& script = *it;

    if (script->content().empty() || !script->context().is_valid()) {
      continue;
    }

    if (script->run_location() != location) {
      continue;
    }

    if (!script->match_all_frames() &&
        !script->emulate_greasemonkey() &&
        frame->parent()) {
      continue;
    }

    if (!script->incognito_enabled() &&
        base::CommandLine::ForCurrentProcess()->HasSwitch(
            switches::kIncognito)) {
      continue;
    }

    if (!script->MatchesURL(data_source_url)) {
      continue;
    }

    std::string content = script->content();

    if (script->emulate_greasemonkey()) {
      content.insert(0, kUserScriptHead);
      content += kUserScriptTail;
    }

    blink::WebScriptSource source(blink::WebString::fromUTF8(content));

    if (script->context() == GURL(kMainWorldContextUrl)) {
      if (script->emulate_greasemonkey()) {
        InjectGreaseMonkeyScriptInMainWorld(frame, source);
      } else {
        frame->executeScript(source);
      }
      continue;
    }

    int id = GetIsolatedWorldID(script->context(), frame);
    frame->executeScriptInIsolatedWorld(id, &source, 1, 0);
  }
}

} // namespace oxide
