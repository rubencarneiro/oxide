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

#include <set>

#include "base/logging.h"
#include "url/gurl.h"

namespace oxide {

namespace {

const size_t kInvalidIndex = static_cast<size_t>(-1);

const uint8_t kUseCountLifetime = 100;

const size_t kMaxCachedEntries = 6;

const int kRegExpMaxMem = 1 * 1024 * 1024;
}

UserAgentOverrideSet::CachedEntry::CachedEntry()
    : index(kInvalidIndex),
      use_count(0) {}

void UserAgentOverrideSet::RecordUsageFor(size_t cache_index) {
  DCHECK_LT(cache_index, cached_overrides_.size());
  DCHECK_LE(history_.size(), kUseCountLifetime);

  CachedEntry& cache_entry = cached_overrides_[cache_index];

  DCHECK_LE(cache_entry.use_count, kUseCountLifetime);

  if (cache_entry.use_count == kUseCountLifetime) {
    return;
  }

  if (history_.size() == kUseCountLifetime) {
    uint8_t i = history_.front();
    history_.pop();
    DCHECK_LT(i, cached_overrides_.size());
    CachedEntry& e = cached_overrides_[i];
    DCHECK_GT(e.use_count, 0U);
    --e.use_count;
  }

  ++cache_entry.use_count;
  history_.push(cache_index);
}

UserAgentOverrideSet::UserAgentOverrideSet() {}

UserAgentOverrideSet::~UserAgentOverrideSet() {}

std::string UserAgentOverrideSet::GetOverrideForURL(const GURL& url) {
  base::AutoLock lock(lock_);

  DCHECK_LE(cached_overrides_.size(), kMaxCachedEntries);

  // Check the cached overrides first
  for (size_t i = 0; i < cached_overrides_.size(); ++i) {
    CachedEntry& entry = cached_overrides_[i];
    if (RE2::PartialMatch(url.spec(), *entry.re)) {
      RecordUsageFor(i);
      return overrides_[entry.index].second;
    }
  }

  std::set<size_t> cached_indexes;
  for (const auto& entry : cached_overrides_) {
    cached_indexes.insert(entry.index);
  }

  RE2::Options options;
  options.set_log_errors(false);
  options.set_max_mem(kRegExpMaxMem);

  linked_ptr<RE2> found_re;
  size_t found_index = kInvalidIndex;
  std::string found_ua;

  for (size_t i = 0; i < overrides_.size(); ++i) {
    if (cached_indexes.size() > 0 && *cached_indexes.begin() == i) {
      // Skip over ones we've just checked above
      cached_indexes.erase(i);
      continue;
    }

    const Entry& entry = overrides_[i];

    linked_ptr<RE2> re = make_linked_ptr(new RE2(entry.first, options));
    if (!re->ok()) {
      LOG(WARNING) <<
          "Regular expression \"" << re->pattern() << "\" is invalid. "
          "Error: " << re->error();
      continue;
    }

    if (RE2::PartialMatch(url.spec(), *re)) {
      found_re = re;
      found_index = i;
      found_ua = entry.second;
      break;
    }
  }

  if (found_index == kInvalidIndex) {
    // No match
    return std::string();
  }

  size_t cache_index = kInvalidIndex;

  if (cached_overrides_.size() == kMaxCachedEntries) {
    // We need to evict a cached entry
    size_t index_to_evict = kInvalidIndex;
    uint8_t lowest_usage = kUseCountLifetime;

    for (size_t i = 0; i < cached_overrides_.size(); ++i) {
      if (cached_overrides_[i].use_count < lowest_usage) {
        lowest_usage = cached_overrides_[i].use_count;
        index_to_evict = i;
      }
    }

    DCHECK_NE(index_to_evict, kInvalidIndex);
    CachedEntry& entry = cached_overrides_[index_to_evict];

    DCHECK(entry.re->pattern() == overrides_[entry.index].first);

    entry.re = found_re;
    entry.index = found_index;

    cache_index = index_to_evict;
  } else {
    // Add a new cached entry
    CachedEntry entry;

    entry.re = found_re;
    entry.index = found_index;

    cached_overrides_.push_back(entry);

    cache_index = cached_overrides_.size() - 1;
  }

  RecordUsageFor(cache_index);

  return found_ua;
}

void UserAgentOverrideSet::SetOverrides(const std::vector<Entry>& overrides) {
  base::AutoLock lock(lock_);

  overrides_ = overrides;
  cached_overrides_.clear();
  while (history_.size() > 0) {
    history_.pop();
  }
}

} // namespace oxide
