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

#include "content/browser/power_save_blocker_impl.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"

namespace content {

class PowerSaveBlockerImpl::Delegate
    : public base::RefCountedThreadSafe<PowerSaveBlockerImpl::Delegate> {
 public:
  Delegate() {}

 private:
  friend class base::RefCountedThreadSafe<Delegate>;
  virtual ~Delegate() {}

  DISALLOW_COPY_AND_ASSIGN(Delegate);
};

PowerSaveBlockerImpl::PowerSaveBlockerImpl(PowerSaveBlockerType type,
                                           const std::string& reason)
    : delegate_(new Delegate()) {
  NOTIMPLEMENTED();
}

PowerSaveBlockerImpl::~PowerSaveBlockerImpl() {
  NOTIMPLEMENTED();
}

}
