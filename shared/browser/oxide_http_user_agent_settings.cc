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

#include "oxide_http_user_agent_settings.h"

#include "content/public/common/content_client.h"
#include "net/http/http_util.h"

#include "oxide_browser_context.h"

namespace oxide {

HttpUserAgentSettings::HttpUserAgentSettings(BrowserContextIOData* context) :
    context_(context) {}

std::string HttpUserAgentSettings::GetAcceptLanguage() const {
  std::string new_accept_lang_setting = context_->GetAcceptLangs();
  if (new_accept_lang_setting != http_accept_language_setting_) {
    http_accept_language_setting_ = new_accept_lang_setting;
    http_accept_language_ =
        net::HttpUtil::GenerateAcceptLanguageHeader(
          http_accept_language_setting_);
  }

  return http_accept_language_;
}

std::string HttpUserAgentSettings::GetUserAgent() const {
  // TODO: Add support for overrides
  // XXX: We use this for a per-context user agent setting - however,
  //      this does not work. URLRequestHttpJob only uses this value if the
  //      request doesn't have a User-Agent header already. All
  //      requests coming from Blink have a User-Agent header, which is either
  //      the one set by WebContents::SetUserAgentOverride() or the one
  //      returned by ContentClient::GetUserAgent(), so this value is ignored.
  //      We could maybe change blink to not set the header if the override
  //      setting hasn't been set. Or we could change URLRequestHttpJob to
  //      set the User-Agent header to the value returned from here
  //      unconditionally (but that would break per-webview overrides)
  //      For now, we actually set the per context user agent setting
  //      via WebContents::SetUserAgentOverride for each webview
  //      See https://launchpad.net/bugs/1279900
  return context_->GetUserAgent();
}

} // namespace oxide
