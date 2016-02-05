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

#include "oxide_param_traits.h"

#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_message_utils.h"

#include "oxide_messages.h"
#include "oxide_script_message_params.h"
#include "oxide_user_agent_override_set.h"

namespace IPC {

// static
void ParamTraits<oxide::ScriptMessageParams>::Write(base::Pickle* m,
                                                    const param_type& p) {
  WriteParam(m, p.context);
  WriteParam(m, p.serial);
  WriteParam(m, p.type);
  WriteParam(m, p.error);
  WriteParam(m, p.msg_id);
  WriteParam(m, p.wrapped_payload);
}

// static
bool ParamTraits<oxide::ScriptMessageParams>::Read(const base::Pickle* m,
                                                   base::PickleIterator* iter,
                                                   param_type* r) {
  if (!ReadParam(m, iter, &r->context)) {
    return false;
  }
  if (!ReadParam(m, iter, &r->serial)) {
    return false;
  }
  if (!ReadParam(m, iter, &r->type)) {
    return false;
  }
  if (!ReadParam(m, iter, &r->error)) {
    return false;
  }
  if (!ReadParam(m, iter, &r->msg_id)) {
    return false;
  }
  if (!ReadParam(m, iter, &r->wrapped_payload)) {
    return false;
  }

  return true;
}

// static
void ParamTraits<oxide::ScriptMessageParams>::Log(const param_type& p,
                                                  std::string* l) {
}

// static
void ParamTraits<oxide::UserAgentOverrideSet::Entry>::Write(
    base::Pickle* m,
    const param_type& p) {
  WriteParam(m, p.first);
  WriteParam(m, p.second);
}

// static
bool ParamTraits<oxide::UserAgentOverrideSet::Entry>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  if (!ReadParam(m, iter, &r->first)) {
    return false;
  }
  if (!ReadParam(m, iter, &r->second)) {
    return false;
  }

  return true;
}

// static
void ParamTraits<oxide::UserAgentOverrideSet::Entry>::Log(const param_type& p,
                                                          std::string* l) {
}

} // namespace IPC
