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

#ifndef _OXIDE_SHARED_COMMON_PARAM_TRAITS_H_
#define _OXIDE_SHARED_COMMON_PARAM_TRAITS_H_

#include <string>

#include "ipc/ipc_param_traits.h"

#include "shared/common/oxide_user_agent_override_set.h"

namespace base {
class PickleIterator;
}

namespace oxide {
struct ScriptMessageParams;
}

namespace IPC {

class Message;

template <>
struct ParamTraits<oxide::ScriptMessageParams> {
  typedef oxide::ScriptMessageParams param_type;
  static void Write(Message* m, const param_type& p);
  static bool Read(const Message* m, base::PickleIterator* iter, param_type* r);
  static void Log(const param_type& p, std::string* l);
};

template <>
struct ParamTraits<oxide::UserAgentOverrideSet::Entry> {
  typedef oxide::UserAgentOverrideSet::Entry param_type;
  static void Write(Message* m, const param_type& p);
  static bool Read(const Message* m, base::PickleIterator* iter, param_type* r);
  static void Log(const param_type& p, std::string* l);
};

} // namespace IPC

#endif // _OXIDE_SHARED_COMMON_PARAM_TRAITS_H_
