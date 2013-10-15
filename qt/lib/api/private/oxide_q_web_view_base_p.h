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

#ifndef _OXIDE_QT_LIB_API_PRIVATE_Q_WEB_VIEW_BASE_P_H_
#define _OXIDE_QT_LIB_API_PRIVATE_Q_WEB_VIEW_BASE_P_H_

#include "shared/browser/oxide_web_view.h"

namespace {
class RenderViewHost;
}

namespace oxide {
namespace qt {

class WebFrameTree;

class QWebViewBasePrivate : public oxide::WebView {
 public:
  virtual WebFrameTree* CreateWebFrameTree(content::RenderViewHost* rvh) = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_API_PRIVATE_Q_WEB_VIEW_BASE_P_H_
