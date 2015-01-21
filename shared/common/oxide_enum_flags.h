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

#ifndef _OXIDE_SHARED_BASE_ENUM_FLAGS_H_
#define _OXIDE_SHARED_BASE_ENUM_FLAGS_H_

namespace oxide {

// This is partly based on Mozilla's CastableTypedEnumResult and
// MOZ_MAKE_ENUM_CLASS_BITWISE_OPERATORS, except we currently don't currently
// use C++11 strongly typed enums and we don't have fake enum classes for
// non-C++11 compilers, which is why there is less code here than there is in
// mfbt/TypedEnumBits.h (from Firefox)

template <typename E>
class CastableEnumResult {
 public:
  explicit CastableEnumResult(E value) : value_(value) {}

  operator E() const { return value_; }
  bool operator !() const { return !bool(value_); }

 private:
  E value_;
};

#define OXIDE_MAKE_ENUM_BINOP_IMPL(Name, Op) \
  inline CastableEnumResult<Name> operator Op(Name lhs, Name rhs) { \
    return CastableEnumResult<Name>(Name(unsigned(lhs) Op unsigned(rhs))); \
  } \
  \
  inline Name& operator Op##=(Name& lhs, Name rhs) { \
    return lhs = lhs Op rhs; \
  }

#define OXIDE_MAKE_ENUM_BITWISE_OPERATORS(Name) \
  OXIDE_MAKE_ENUM_BINOP_IMPL(Name, |) \
  OXIDE_MAKE_ENUM_BINOP_IMPL(Name, &) \
  OXIDE_MAKE_ENUM_BINOP_IMPL(Name, ^) \
  inline CastableEnumResult<Name> operator ~(Name a) { \
    return CastableEnumResult<Name>(Name(~unsigned(a))); \
  }

} // namespace oxide

#endif // _OXIDE_SHARED_BASE_ENUM_FLAGS_H_
