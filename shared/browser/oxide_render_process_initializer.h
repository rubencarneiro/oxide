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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef _OXIDE_SHARED_BROWSER_RENDER_PROCESS_INITIALIZER_H_
#define _OXIDE_SHARED_BROWSER_RENDER_PROCESS_INITIALIZER_H_

#include "base/macros.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

namespace oxide {

class RenderProcessInitializer : public content::NotificationObserver {
 public:
  RenderProcessInitializer();
  ~RenderProcessInitializer() override;

 private:
  // content::NotificationObserver implementation
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(RenderProcessInitializer);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_PROCESS_INITIALIZER_H_
