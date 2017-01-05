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

#ifndef _OXIDE_QT_CORE_API_WEB_CONTEXT_MENU_PARAMS_P_H_
#define _OXIDE_QT_CORE_API_WEB_CONTEXT_MENU_PARAMS_P_H_

#include <QSharedData>
#include <QString>
#include <QUrl>

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/api/oxideqwebcontextmenuparams.h"

namespace oxide {
namespace qt {
struct WebContextMenuParams;
}
}

class OXIDE_QTCORE_EXPORT OxideQWebContextMenuParamsData : public QSharedData {
 public:
  static OxideQWebContextMenuParams Create(
      const oxide::qt::WebContextMenuParams& params);

  OxideQWebContextMenuParamsData();
  ~OxideQWebContextMenuParamsData();

  QUrl page_url;
  QUrl frame_url;

  bool is_link = false;
  bool is_editable = false;
  bool is_selection = false;
  OxideQWebContextMenuParams::MediaType media_type =
      OxideQWebContextMenuParams::MediaTypeNone;

  QUrl link_url;
  QString link_text;
  QString title_text;
  QUrl src_url;
};

#endif // _OXIDE_QT_CORE_API_WEB_CONTEXT_MENU_PARAMS_P_H_
