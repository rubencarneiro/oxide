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

#include "oxideubuntuwebview.h"
#include "oxideubuntuwebview_p.h"

#include "qt/core/api/oxideqwebcontextmenuparams.h"
#include "qt/core/api/oxideqwebcontextmenuparams_p.h"
#include "qt/uitk/lib/uitk_contents_view.h"

OxideUbuntuWebViewPrivate::OxideUbuntuWebViewPrivate(
    OxideUbuntuWebView* q)
    : OxideQQuickWebViewPrivate(
          q,
          std::unique_ptr<oxide::qquick::ContentsView>(
              new oxide::uitk::ContentsView(q)),
          std::unique_ptr<oxide::qt::AuxiliaryUIFactory>(
              new oxide::uitk::AuxiliaryUIFactory(q, this))) {}

void OxideUbuntuWebViewPrivate::ContextMenuOpening(
    const oxide::qt::WebContextMenuParams& params,
    OxideUbuntuWebContextMenu* menu) {
  Q_Q(OxideUbuntuWebView);

  Q_EMIT q->contextMenuOpening(OxideQWebContextMenuParamsData::Create(params),
                               menu);
}

OxideUbuntuWebView::OxideUbuntuWebView(QQuickItem* parent)
    : OxideQQuickWebView(*new OxideUbuntuWebViewPrivate(this), parent) {}

OxideUbuntuWebView::~OxideUbuntuWebView() = default;
