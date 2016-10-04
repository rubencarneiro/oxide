// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_API_NEW_VIEW_REQUEST_P_H_
#define _OXIDE_QT_CORE_API_NEW_VIEW_REQUEST_P_H_

#include <QRect>
#include <QtGlobal>

#include "base/memory/weak_ptr.h"

#include "qt/core/api/oxideqnewviewrequest.h"
#include "shared/browser/web_contents_unique_ptr.h"

namespace content {
class WebContents;
}

namespace oxide {
class WebView;
}

class OxideQNewViewRequestPrivate final {
 public:
  ~OxideQNewViewRequestPrivate();

  static OxideQNewViewRequestPrivate* get(OxideQNewViewRequest* q);

  oxide::WebContentsUniquePtr contents;
  base::WeakPtr<oxide::WebView> view;

 private:
  friend class OxideQNewViewRequest;

  OxideQNewViewRequestPrivate(const QRect& position,
                              OxideQNewViewRequest::Disposition disposition);

  QRect position_;
  OxideQNewViewRequest::Disposition disposition_;
};

#endif // _OXIDE_QT_CORE_API_NEW_VIEW_REQUEST_P_H_
