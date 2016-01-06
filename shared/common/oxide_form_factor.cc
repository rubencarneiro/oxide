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

#include "oxide_form_factor.h"

#include "base/command_line.h"

#include "oxide_constants.h"

namespace oxide {

FormFactor GetFormFactorHint() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  std::string form_factor =
      command_line->GetSwitchValueASCII(switches::kFormFactor);
  if (form_factor == switches::kFormFactorDesktop) {
    return FORM_FACTOR_DESKTOP;
  } else if (form_factor == switches::kFormFactorTablet) {
    return FORM_FACTOR_TABLET;
  } else if (form_factor == switches::kFormFactorPhone) {
    return FORM_FACTOR_PHONE;
  }

  return FORM_FACTOR_DESKTOP;
}

} // namespace oxide
