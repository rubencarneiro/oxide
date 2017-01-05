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

#ifndef _OXIDE_QT_UITK_LIB_API_UBUNTU_WEB_VIEW_P_H_
#define _OXIDE_QT_UITK_LIB_API_UBUNTU_WEB_VIEW_P_H_

#include <QtGlobal>

#include "qt/quick/api/oxideqquickwebview_p.h"
#include "qt/uitk/lib/uitk_auxiliary_ui_factory.h"

class OxideUbuntuWebView;

class OxideUbuntuWebViewPrivate
    : public OxideQQuickWebViewPrivate,
      public oxide::uitk::AuxiliaryUIFactory::Delegate {
  Q_DECLARE_PUBLIC(OxideUbuntuWebView)
  Q_DISABLE_COPY(OxideUbuntuWebViewPrivate)

  OxideUbuntuWebViewPrivate(OxideUbuntuWebView* q);

  // oxide::uitk::AuxiliaryUIFactory::Delegate implementation
  void ContextMenuOpening(const oxide::qt::WebContextMenuParams& params,
                          OxideUbuntuWebContextMenu* menu) override;
};

#endif // _OXIDE_QT_UITK_LIB_API_UBUNTU_WEB_VIEW_P_H_
