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

#ifndef _OXIDE_SHARED_COMMON_USER_AGENT_OVERRIDE_SET_H_
#define _OXIDE_SHARED_COMMON_USER_AGENT_OVERRIDE_SET_H_

#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "base/memory/linked_ptr.h"
#include "base/synchronization/lock.h"
#include "third_party/re2/src/re2/re2.h"

class GURL;

namespace oxide {

class UserAgentOverrideSet {
 public:
  typedef std::pair<std::string, std::string> Entry;

  UserAgentOverrideSet();
  ~UserAgentOverrideSet();

  std::string GetOverrideForURL(const GURL& url);

  void SetOverrides(const std::vector<Entry>& overrides);

 private:
  struct CachedEntry {
    CachedEntry();

    linked_ptr<RE2> re;
    size_t index;
    uint8_t use_count;
  };

  void RecordUsageFor(size_t cache_index);

  base::Lock lock_;

  std::vector<Entry> overrides_;

  std::vector<CachedEntry> cached_overrides_;

  std::queue<uint8_t> history_;

  DISALLOW_COPY_AND_ASSIGN(UserAgentOverrideSet);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_USER_AGENT_OVERRIDE_SET_H_
