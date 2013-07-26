// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef _OXIDE_PUBLIC_BROWSER_BROWSER_PROCESS_HANDLE_H_
#define _OXIDE_PUBLIC_BROWSER_BROWSER_PROCESS_HANDLE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"

#include "shared/browser/oxide_browser_process_main.h"

namespace oxide {

// Creating the first instance of this class will start up the main
// browser process components. These will exist until all instances
// of this class have been destroyed
class BrowserProcessHandle {
 public:
  BrowserProcessHandle();
  bool Available() const { return !!handle_; }

 private:
  scoped_refptr<BrowserProcessMain> handle_;

  DISALLOW_COPY_AND_ASSIGN(BrowserProcessHandle);
};

} // namespace oxide

#endif // _OXIDE_PUBLIC_BROWSER_BROWSER_PROCESS_HANDLE_H_
