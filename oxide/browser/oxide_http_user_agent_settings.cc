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

#include "oxide/public/browser/oxide_global_settings.h"

namespace oxide {

std::string HttpUserAgentSettings::GetAcceptLanguage() const {
  std::string new_accept_lang_setting = GlobalSettings::GetAcceptLangs();
  if (new_accept_lang_setting != http_accept_language_setting_) {
    http_accept_language_setting_ = new_accept_lang_setting;
    http_accept_language_ =
        net::HttpUtil::GenerateAcceptLanguageHeader(
          http_accept_language_setting_);
  }

  return http_accept_language_;
}

std::string HttpUserAgentSettings::GetUserAgent(const GURL& url) const {
  return content::GetUserAgent(url);
}

} // namespace oxide
