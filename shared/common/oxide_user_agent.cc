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

#include "oxide_user_agent.h"

#include "base/lazy_instance.h"

namespace oxide {

namespace {
base::LazyInstance<std::string>::Leaky g_user_agent = LAZY_INSTANCE_INITIALIZER;
}

std::string GetUserAgent() {
  return g_user_agent.Get();
}

void SetUserAgent(const std::string& user_agent) {
  g_user_agent.Get() = user_agent;
}

} // namespace oxide
