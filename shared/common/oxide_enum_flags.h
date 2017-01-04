// vim:expandtab:shiftwidth=2:tabstop=2:

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

// This is basically a copy of the mfbt/TypedEnumBits.h from Firefox

#ifndef _OXIDE_SHARED_BASE_ENUM_FLAGS_H_
#define _OXIDE_SHARED_BASE_ENUM_FLAGS_H_

#include <type_traits>

namespace oxide {

template <typename E>
class CastableTypedEnumResult {
 public:
  explicit CastableTypedEnumResult(E value) : value_(value) {}

  operator E() const { return value_; }
  bool operator !() const { return !bool(value_); }

 private:
  E value_;
};

} // namespace oxide

#define OXIDE_CASTABLETYPEDENUMRESULT_BINOP(Op, OtherType, ReturnType) \
  template<typename E> \
  ReturnType operator Op(const OtherType& lhs, \
                         const oxide::CastableTypedEnumResult<E>& rhs) { \
    return ReturnType(lhs Op OtherType(rhs)); \
  } \
  template<typename E> \
  ReturnType operator Op(const oxide::CastableTypedEnumResult<E>& lhs, \
                         const OtherType& rhs) { \
    return ReturnType(OtherType(lhs) Op rhs); \
  } \
  template<typename E> \
  ReturnType operator Op(const oxide::CastableTypedEnumResult<E>& lhs, \
                         const oxide::CastableTypedEnumResult<E>& rhs) { \
    return ReturnType(OtherType(lhs) Op OtherType(rhs)); \
  }

OXIDE_CASTABLETYPEDENUMRESULT_BINOP(|, E, oxide::CastableTypedEnumResult<E>)
OXIDE_CASTABLETYPEDENUMRESULT_BINOP(&, E, oxide::CastableTypedEnumResult<E>)
OXIDE_CASTABLETYPEDENUMRESULT_BINOP(^, E, oxide::CastableTypedEnumResult<E>)
OXIDE_CASTABLETYPEDENUMRESULT_BINOP(==, E, bool)
OXIDE_CASTABLETYPEDENUMRESULT_BINOP(!=, E, bool)
OXIDE_CASTABLETYPEDENUMRESULT_BINOP(||, bool, bool)
OXIDE_CASTABLETYPEDENUMRESULT_BINOP(&&, bool, bool)

template <typename E>
oxide::CastableTypedEnumResult<E>
operator ~(const oxide::CastableTypedEnumResult<E>& a)
{
  return oxide::CastableTypedEnumResult<E>(~(E(a)));
}

#define OXIDE_CASTABLETYPEDENUMRESULT_COMPOUND_ASSIGN_OP(Op) \
  template<typename E> \
  E& operator Op(E& lhs, const oxide::CastableTypedEnumResult<E>& rhs) { \
    return lhs Op E(rhs); \
  }

OXIDE_CASTABLETYPEDENUMRESULT_COMPOUND_ASSIGN_OP(&=)
OXIDE_CASTABLETYPEDENUMRESULT_COMPOUND_ASSIGN_OP(|=)
OXIDE_CASTABLETYPEDENUMRESULT_COMPOUND_ASSIGN_OP(^=)

#undef OXIDE_CASTABLETYPEDENUMRESULT_COMPOUND_ASSIGN_OP
#undef OXIDE_CASTABLETYPEDENUMRESULT_BINOP

#define OXIDE_MAKE_ENUM_BINOP_IMPL(Name, Op) \
  inline ::oxide::CastableTypedEnumResult<Name> operator Op(Name lhs, \
                                                            Name rhs) { \
    typedef std::make_unsigned<Name>::type U; \
    return ::oxide::CastableTypedEnumResult<Name>(Name(U(lhs) Op U(rhs))); \
  } \
  \
  inline Name& operator Op##=(Name& lhs, Name rhs) { \
    return lhs = lhs Op rhs; \
  }

#define OXIDE_MAKE_ENUM_BITWISE_OPERATORS(Name) \
  OXIDE_MAKE_ENUM_BINOP_IMPL(Name, |) \
  OXIDE_MAKE_ENUM_BINOP_IMPL(Name, &) \
  OXIDE_MAKE_ENUM_BINOP_IMPL(Name, ^) \
  inline ::oxide::CastableTypedEnumResult<Name> operator ~(Name a) { \
    typedef std::make_unsigned<Name>::type U; \
    return ::oxide::CastableTypedEnumResult<Name>(Name(~U(a))); \
  }

#endif // _OXIDE_SHARED_BASE_ENUM_FLAGS_H_
