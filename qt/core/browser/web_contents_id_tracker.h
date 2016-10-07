// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_CONTENTS_ID_TRACKER_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_CONTENTS_ID_TRACKER_H_

#include <set>

#include "base/macros.h"

#include "qt/core/common/oxide_qt_export.h"
#include "qt/core/glue/web_contents_id.h"

namespace base {
template<typename Type> struct DefaultSingletonTraits;
}

namespace content {
class WebContents;
}

namespace oxide {
namespace qt {

// Allows safely passing an opaque identifier for a WebContents outside of the
// core library
class OXIDE_QT_EXPORT WebContentsIDTracker {
 public:
  static WebContentsIDTracker* GetInstance();

  WebContentsID GetIDForWebContents(content::WebContents* contents);

  content::WebContents* GetWebContentsFromID(WebContentsID id);

 private:
  friend struct base::DefaultSingletonTraits<WebContentsIDTracker>;
  WebContentsIDTracker();

  void AddWebContents(content::WebContents* contents);

  class WebContentsObserver;
  void RemoveWebContents(content::WebContents* contents);

  std::set<WebContentsID> web_contents_set_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsIDTracker);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_CONTENTS_ID_TRACKER_H_
