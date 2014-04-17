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

#include "base/pickle.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/renderer/render_thread.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebURLRequest.h"
#include "third_party/WebKit/public/web/WebDataSource.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebScriptSource.h"
#include "third_party/WebKit/public/web/WebSecurityOrigin.h"
#include "url/gurl.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_user_script.h"

#include "oxide_isolated_world_map.h"
#include "oxide_process_observer.h"

namespace oxide {

namespace {
const char kIsolatedWorldCSP[] = "script-src 'self'";
const char kUserScriptHead[] = "(function (unsafeWindow) {\n";
const char kUserScriptTail[] = "\n})(window);";
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

  CHECK(shmem->Map(sizeof(Pickle::Header)));
  Pickle::Header* header = reinterpret_cast<Pickle::Header *>(shmem->memory());
  int size = sizeof(Pickle::Header) + header->payload_size;
  shmem->Unmap();

  CHECK(shmem->Map(size));

  Pickle pickle(reinterpret_cast<char *>(shmem->memory()), size);
  PickleIterator iter(pickle);

  uint64 num_scripts = 0;
  CHECK(pickle.ReadUInt64(&iter, &num_scripts));
  for (; num_scripts > 0; --num_scripts) {
    linked_ptr<UserScript> script(new UserScript());
    user_scripts_.push_back(script);

    script->Unpickle(pickle, &iter);
  }
}

UserScriptSlave::UserScriptSlave() {
  content::RenderThread::Get()->AddObserver(this);
}

UserScriptSlave::~UserScriptSlave() {}

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

    if (!script->incognito_enabled() && ProcessObserver::IsOffTheRecord()) {
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
    int id = GetIsolatedWorldID(script->context(), frame);
    frame->executeScriptInIsolatedWorld(id, &source, 1, 0);
  }
}


} // namespace oxide
