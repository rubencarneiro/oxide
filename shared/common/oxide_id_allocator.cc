// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_id_allocator.h"

#include <limits>
#include <utility>

#include "base/logging.h"

namespace oxide {

bool IdAllocator::IsValidId(int id) const {
  return id > kInvalidId && id <= max_id_;
}

IdAllocator::IdAllocator(size_t max_id)
    : max_id_(static_cast<int>(std::min(
                  max_id,
                  static_cast<size_t>(std::numeric_limits<int>::max())))) {
  if (max_id_ == 0) {
    max_id_ = std::numeric_limits<int>::max();
  }
}

IdAllocator::~IdAllocator() {}

int IdAllocator::AllocateId() {
  int id = kInvalidId;
  if (!free_ids_.empty()) {
    // Reuse a free'd ID if we have one
    id = *free_ids_.begin();
  } else if (!used_ids_.empty()) {
    // We have no free ID's. Find the last allocated ID
    int last_id = *used_ids_.rbegin();
    DCHECK(IsValidId(last_id));
    if (last_id == max_id_) {
      // The last allocated ID was our maximum. Search for an ID that
      // has never been allocated
      int x = 0;
      for (std::set<int>::iterator it = used_ids_.begin();
           it != used_ids_.end(); ++it) {
        if (*it != x) {
          id = x;
          break;
        }
        ++x;
      }
    } else {
      // Use the next ID
      id = last_id + 1;
    }
  } else {
    // Use 0 for the first ID
    id = 0;
  }

  if (id == kInvalidId) {
    // There are no more ID's available
    return kInvalidId;
  }

  bool res = MarkAsUsed(id);
  DCHECK(res);

  return id;
}

void IdAllocator::FreeId(int id) {
  if (!IsValidId(id)) {
    return;
  }

  if (used_ids_.erase(id) > 0) {
    free_ids_.insert(id);
  }
}

bool IdAllocator::MarkAsUsed(int id) {
  if (!IsValidId(id)) {
    return false;
  }

  free_ids_.erase(id);
  std::pair<std::set<int>::iterator, bool> res = used_ids_.insert(id);
  return res.second;
}

} // namespace oxide
