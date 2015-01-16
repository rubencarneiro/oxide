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

#ifndef _OXIDE_SHARED_BASE_ID_ALLOCATOR_H_
#define _OXIDE_SHARED_BASE_ID_ALLOCATOR_H_

#include <set>

#include "base/compiler_specific.h"
#include "base/macros.h"

namespace oxide {

static const int kInvalidId = -1;

class IdAllocator final {
 public:
  IdAllocator(size_t max_id);
  ~IdAllocator();

  int AllocateId();
  void FreeId(int id);
  bool MarkAsUsed(int id);

 private:
  bool IsValidId(int id) const;

  int max_id_;
  std::set<int> used_ids_;
  std::set<int> free_ids_;

  DISALLOW_COPY_AND_ASSIGN(IdAllocator);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BASE_ID_ALLOCATOR_H_
