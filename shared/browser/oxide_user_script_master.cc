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

#include "oxide_user_script_master.h"

#include <string>

#include "base/memory/shared_memory.h"
#include "base/memory/singleton.h"
#include "base/pickle.h"
#include "base/process/process.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "content/public/browser/render_process_host.h"
#include "extensions/common/url_pattern.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_user_script.h"

#include "oxide_browser_context.h"

namespace oxide {

namespace {

bool GetValue(const base::StringPiece& line,
              const base::StringPiece& prefix,
              std::string* value) {
  base::StringPiece::size_type index = line.find(prefix);
  if (index == base::StringPiece::npos) {
    return false;
  }

  std::string temp(line.data() + index + prefix.length(),
                   line.length() - index - prefix.length());

  if (temp.empty() || !base::IsUnicodeWhitespace(temp[0])) {
    return false;
  }

  base::TrimWhitespaceASCII(temp, base::TRIM_ALL, value);
  return true;
}

}

class UserScriptMasterFactory : public BrowserContextKeyedServiceFactory {
 public:
  static UserScriptMasterFactory* GetInstance();
  static UserScriptMaster* GetForContext(content::BrowserContext* context);

 private:
  friend struct base::DefaultSingletonTraits<UserScriptMasterFactory>;

  UserScriptMasterFactory();
  ~UserScriptMasterFactory() override;

  // BrowserContextKeyedServiceFactory methods:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(UserScriptMasterFactory);
};

UserScriptMasterFactory::UserScriptMasterFactory()
    : BrowserContextKeyedServiceFactory(
        "UserScriptMaster",
        BrowserContextDependencyManager::GetInstance()) {}

UserScriptMasterFactory::~UserScriptMasterFactory() {}

KeyedService* UserScriptMasterFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new UserScriptMaster(BrowserContext::FromContent(context));
}

content::BrowserContext* UserScriptMasterFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return BrowserContext::FromContent(context)->GetOriginalContext();
}

// static
UserScriptMasterFactory* UserScriptMasterFactory::GetInstance() {
  return base::Singleton<UserScriptMasterFactory>::get();
}

// static
UserScriptMaster* UserScriptMasterFactory::GetForContext(
    content::BrowserContext* context) {
  return static_cast<UserScriptMaster*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}


UserScriptMaster::UserScriptMaster(BrowserContext* context) :
    context_(context) {}

UserScriptMaster::~UserScriptMaster() {}

UserScriptMaster::HostSet UserScriptMaster::GetHostSet() const {
  HostSet hosts;

  for (content::RenderProcessHost::iterator it =
          content::RenderProcessHost::AllHostsIterator();
       !it.IsAtEnd(); it.Advance()) {
    content::RenderProcessHost* host = it.GetCurrentValue();
    BrowserContext* other =
        BrowserContext::FromContent(host->GetBrowserContext());
    if (context_->IsSameContext(other)) {
      hosts.insert(host);
    }
  }

  return hosts;

}

void UserScriptMaster::SendUpdate(content::RenderProcessHost* process) {
  if (!shmem_) {
    return;
  }

  base::ProcessHandle handle = process->GetHandle();
  if (!handle) {
    return;
  }

  base::SharedMemoryHandle handle_for_process;
  if (!shmem_->ShareToProcess(handle, &handle_for_process)) {
    return;
  }

  if (base::SharedMemory::IsHandleValid(handle_for_process)) {
    process->Send(new OxideMsg_UpdateUserScripts(handle_for_process));
  }
}

// static
UserScriptMaster* UserScriptMaster::Get(content::BrowserContext* context) {
  return UserScriptMasterFactory::GetForContext(context);
}

void UserScriptMaster::SerializeUserScriptsAndSendUpdates(
    std::vector<const UserScript *>& scripts) {
  // XXX: Should probably do this off the UI thread
  base::Pickle pickle;
  pickle.WriteUInt64(scripts.size());
  for (size_t i = 0; i < scripts.size(); ++i) {
    const UserScript* script = scripts[i];
    script->Pickle(&pickle);
  }

  shmem_.reset(new base::SharedMemory());
  if (!shmem_->CreateAndMapAnonymous(pickle.size())) {
    shmem_.reset();
    return;
  }

  memcpy(shmem_->memory(), pickle.data(), pickle.size());

  HostSet hosts = GetHostSet();
  for (auto host : hosts) {
    SendUpdate(host);
  }
}

// static
void UserScriptMaster::ParseMetadata(UserScript* script) {
  size_t line_start = 0;
  size_t line_end = line_start;
  bool in_metadata = false;
  base::StringPiece line;

  static const base::StringPiece kUserScriptBegin("// ==UserScript==");
  static const base::StringPiece kUserScriptEnd("// ==/UserScript==");
  static const base::StringPiece kIncludeDeclaration("// @include");
  static const base::StringPiece kExcludeDeclaration("// @exclude");
  static const base::StringPiece kMatchDeclaration("// @match");
  static const base::StringPiece kExcludeMatchDeclaration("// @exclude_match");
  static const base::StringPiece kRunAtDeclaration("// @run-at");
  static const base::StringPiece kRunAtDocumentStartValue("document-start");
  static const base::StringPiece kRunAtDocumentEndValue("document-end");
  static const base::StringPiece kRunAtDocumentIdleValue("document-idle");

  while (line_start < script->content().length()) {
    line_end = script->content().find('\n', line_start);

    line.set(script->content().data() + line_start, line_end - line_start);

    if (!in_metadata) {
      if (line.starts_with(kUserScriptBegin)) {
        in_metadata = true;
      }
    } else {
      if (line.starts_with(kUserScriptEnd)) {
        break;
      }

      std::string value;
      if (GetValue(line, kIncludeDeclaration, &value)) {
        base::ReplaceSubstringsAfterOffset(&value, 0, "\\", "\\\\");
        base::ReplaceSubstringsAfterOffset(&value, 0, "?", "\\?");
        script->add_include_glob(value);
      } else if (GetValue(line, kExcludeDeclaration, &value)) {
        base::ReplaceSubstringsAfterOffset(&value, 0, "\\", "\\\\");
        base::ReplaceSubstringsAfterOffset(&value, 0, "?", "\\?");
        script->add_exclude_glob(value);
      } else if (GetValue(line, kMatchDeclaration, &value)) {
        URLPattern pattern(URLPattern::SCHEME_HTTP | URLPattern::SCHEME_HTTPS);
        if (URLPattern::PARSE_SUCCESS != pattern.Parse(value)) {
          script->add_include_url_pattern(pattern);
        }
      } else if (GetValue(line, kExcludeMatchDeclaration, &value)) {
        URLPattern pattern(URLPattern::SCHEME_HTTP | URLPattern::SCHEME_HTTPS);
        if (URLPattern::PARSE_SUCCESS != pattern.Parse(value)) {
          script->add_exclude_url_pattern(pattern);
        }
      } else if (GetValue(line, kRunAtDeclaration, &value)) {
        if (value == kRunAtDocumentStartValue) {
          script->set_run_location(UserScript::DOCUMENT_START);
        } else if (value == kRunAtDocumentEndValue) {
          script->set_run_location(UserScript::DOCUMENT_END);
        } else if (value == kRunAtDocumentIdleValue) {
          script->set_run_location(UserScript::DOCUMENT_IDLE);
        }
      }
    }

    line_start = line_end + 1;
  }
}

void UserScriptMaster::RenderProcessCreated(
    content::RenderProcessHost* process) {
  SendUpdate(process);
}

} // namespace oxide
