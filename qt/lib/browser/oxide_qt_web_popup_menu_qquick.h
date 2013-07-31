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

#ifndef _OXIDE_QT_LIB_BROWSER_WEB_POPUP_MENU_QQUICK_H_
#define _OXIDE_QT_LIB_BROWSER_WEB_POPUP_MENU_QQUICK_H_

#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_web_popup_menu.h"

class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlContext;
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class WebPopupMenuQQuick FINAL : public oxide::WebPopupMenu {
 public:
  WebPopupMenuQQuick(OxideQQuickWebView* view,
                     content::WebContents* web_contents);

  void Show(const gfx::Rect& bounds,
            const std::vector<WebMenuItem>& items,
            int selected_item,
            bool allow_multiple_selection) FINAL;

 private:
  OxideQQuickWebView* view_;
  scoped_ptr<QQmlContext> context_;
  scoped_ptr<QQuickItem> popup_menu_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebPopupMenuQQuick);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_BROWSER_WEB_POPUP_MENU_QQUICK_H_
