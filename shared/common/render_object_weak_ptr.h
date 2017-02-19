// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_COMMON_RENDER_OBJECT_WEAK_PTR_H_
#define _OXIDE_SHARED_COMMON_RENDER_OBJECT_WEAK_PTR_H_

#include "base/logging.h"

namespace oxide {

namespace internal {
template <class T>
struct RenderObjectWeakPtrTraits {};
}

template <class T>
class RenderObjectWeakPtr {
  using Traits = internal::RenderObjectWeakPtrTraits<T>;
  using ID = typename Traits::ID;

 public:
  RenderObjectWeakPtr() = default;
  ~RenderObjectWeakPtr() = default;

  RenderObjectWeakPtr(T* p)
      : ptr_(p),
        id_(p ? Traits::GetID(p) : ID()) {
    DCHECK(!p || Traits::IsValidID(id_));
  }
  RenderObjectWeakPtr(std::nullptr_t null)
      : ptr_(nullptr) {}

  T* get() const { return Traits::IsValidID(id_) ? ptr_ : nullptr; }

  T* operator->() const { return get(); }

  RenderObjectWeakPtr<T>& operator=(T* p) {
    ptr_ = p;
    id_ = p ? Traits::GetID(p) : ID();
    DCHECK(!p || Traits::IsValidID(id_));
    return *this;
  }
  RenderObjectWeakPtr<T>& operator=(std::nullptr_t null) {
    ptr_ = nullptr;
    id_ = ID();
    return *this;
  }

  explicit operator bool() const { return !!get(); }

  bool operator==(const RenderObjectWeakPtr<T>& other) const {
    return get() == other.get();
  }
  bool operator!=(const RenderObjectWeakPtr<T>& other) const {
    return !(*this == other);
  }

  T* unsafeGet() const { return ptr_; }

 private:
  T* ptr_ = nullptr;
  ID id_;
};

template <typename T, typename U>
bool operator==(const RenderObjectWeakPtr<T>& lhs, U* rhs) {
  return lhs.get() == rhs;
}

template <typename T, typename U>
bool operator==(T* lhs, const RenderObjectWeakPtr<U>& rhs) {
  return lhs == rhs.get();
}

template <typename T, typename U>
bool operator!=(const RenderObjectWeakPtr<T>& lhs, U* rhs) {
  return !(lhs == rhs);
}

template <typename T, typename U>
bool operator!=(T* lhs, const RenderObjectWeakPtr<U>& rhs) {
  return !(lhs == rhs);
}

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_RENDER_OBJECT_WEAK_PTR_H_
