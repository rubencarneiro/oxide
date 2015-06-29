// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_user_agent_override_set.h"

#include "base/logging.h"
#include "url/gurl.h"

namespace oxide {

UserAgentOverrideSet::UserAgentOverrideSet() {}

UserAgentOverrideSet::~UserAgentOverrideSet() {}

std::string UserAgentOverrideSet::GetOverrideForURL(const GURL& url) {
  base::AutoLock lock(lock_);

  for (const auto& entry : overrides_) {
    if (RE2::PartialMatch(url.spec(), *entry.first)) {
      return entry.second;
    }
  }

  return std::string();
}

void UserAgentOverrideSet::SetOverrides(const std::vector<Entry>& overrides) {
  base::AutoLock lock(lock_);

  overrides_.clear();

  RE2::Options options;
  options.set_log_errors(false);
  options.set_max_mem(1000000);

  for (auto& entry : overrides) {
    linked_ptr<RE2> re = make_linked_ptr(new RE2(entry.first, options));
    if (!re->ok()) {
      LOG(WARNING) <<
          "Regular expression \"" << re->pattern() << "\" is invalid. "
          "Error: " << re->error();
      continue;
    }

    overrides_.push_back(std::make_pair(re, entry.second));
  }
}

} // namespace oxide
