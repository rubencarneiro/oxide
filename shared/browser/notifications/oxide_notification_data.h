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

#ifndef _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DATA_H_
#define _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DATA_H_

#include "base/strings/string16.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "url/gurl.h"

namespace oxide {

struct NotificationData {
  NotificationData(const base::string16& title,
                   const base::string16& body,
                   const SkBitmap& icon)
      : title(title),
        body(body),
        icon(icon) {}

  base::string16 title;
  base::string16 body;
  SkBitmap icon;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DATA_H_
