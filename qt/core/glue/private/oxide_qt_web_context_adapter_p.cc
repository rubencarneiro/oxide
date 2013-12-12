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

#include "oxide_qt_web_context_adapter_p.h"

#include <string>
#include <vector>

#include "base/logging.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_user_script_master.h"

#include "qt/core/glue/oxide_qt_web_context_adapter.h"

#include "oxide_qt_user_script_adapter_p.h"

namespace oxide {
namespace qt {

struct LazyInitProperties {
  std::string product;
  std::string user_agent;
  base::FilePath data_path;
  base::FilePath cache_path;
  std::string accept_langs;
};

WebContextAdapterPrivate::WebContextAdapterPrivate() :
    lazy_init_props_(new LazyInitProperties()) {}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::Create() {
  return new WebContextAdapterPrivate();
}

WebContextAdapterPrivate::~WebContextAdapterPrivate() {}

std::string WebContextAdapterPrivate::GetProduct() const {
  if (context_) {
    return context_->GetProduct();
  }

  return lazy_init_props_->product;
}

void WebContextAdapterPrivate::SetProduct(const std::string& product) {
  if (context_) {
    context_->SetProduct(product);
  } else {
    lazy_init_props_->product = product;
  }
}

std::string WebContextAdapterPrivate::GetUserAgent() const {
  if (context_) {
    return context_->GetUserAgent();
  }

  return lazy_init_props_->user_agent;
}

void WebContextAdapterPrivate::SetUserAgent(const std::string& user_agent) {
  if (context_) {
    context_->SetUserAgent(user_agent);
  } else {
    lazy_init_props_->user_agent = user_agent;
  }
}

base::FilePath WebContextAdapterPrivate::GetDataPath() const {
  if (context_) {
    return context_->GetPath();
  }

  return lazy_init_props_->data_path;
}

void WebContextAdapterPrivate::SetDataPath(const base::FilePath& path) {
  DCHECK(!context_);
  lazy_init_props_->data_path = path;
}

base::FilePath WebContextAdapterPrivate::GetCachePath() const {
  if (context_) {
    return context_->GetCachePath();
  }

  return lazy_init_props_->cache_path;
}

void WebContextAdapterPrivate::SetCachePath(const base::FilePath& path) {
  DCHECK(!context_);
  lazy_init_props_->cache_path = path;
}

std::string WebContextAdapterPrivate::GetAcceptLangs() const {
  if (context_) {
    return context_->GetAcceptLangs();
  }

  return lazy_init_props_->accept_langs;
}

void WebContextAdapterPrivate::SetAcceptLangs(const std::string& langs) {
  if (context_) {
    context_->SetAcceptLangs(langs);
  } else {
    lazy_init_props_->accept_langs = langs;
  }
}

void WebContextAdapterPrivate::UpdateUserScripts() {
  if (!context_) {
    return;
  }

  std::vector<oxide::UserScript *> scripts;
  bool wait = false;

  for (int i = 0; i < user_scripts_.size(); ++i) {
    UserScriptAdapterPrivate* script =
        UserScriptAdapterPrivate::get(user_scripts_.at(i));
    if (script->state() == UserScriptAdapter::Loading) {
      wait = true;
    } else if (script->state() == UserScriptAdapter::Deferred) {
      script->StartLoading();
      wait = true;
    } else if (script->state() == UserScriptAdapter::Ready) {
      scripts.push_back(&script->user_script());
    }
  }

  if (!wait) {
    context_->UserScriptManager().SerializeUserScriptsAndSendUpdates(scripts);
  }
}

void WebContextAdapterPrivate::CompleteConstruction() {
  DCHECK(!context_ && !process_handle_);

  // We do this here rather than in the constructor because the first
  // browser context needs to set the shared GL context before anything
  // starts up, in order for compositing to work
  process_handle_ = oxide::BrowserProcessMain::GetInstance();

  context_.reset(oxide::BrowserContext::Create(
      lazy_init_props_->data_path,
      lazy_init_props_->cache_path));

  if (!lazy_init_props_->product.empty()) {
    context_->SetProduct(lazy_init_props_->product);
  }
  if (!lazy_init_props_->user_agent.empty()) {
    context_->SetUserAgent(lazy_init_props_->user_agent);
  }
  if (!lazy_init_props_->accept_langs.empty()) {
    context_->SetAcceptLangs(lazy_init_props_->accept_langs);
  }

  lazy_init_props_.reset();

  UpdateUserScripts();
}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::get(
    WebContextAdapter* adapter) {
  return adapter->priv.data();
}

} // namespace qt
} // namespace oxide
