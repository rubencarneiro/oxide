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

#ifndef _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_P_H_
#define _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_P_H_

#include <QtGlobal>

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"

#include "shared/browser/oxide_browser_process_handle.h"

namespace oxide {
class BrowserContext;
}

struct LazyInitProperties;
class OxideQQuickWebViewContext;

class OxideQQuickWebViewContextPrivate {
  Q_DECLARE_PUBLIC(OxideQQuickWebViewContext)

 public:
  OxideQQuickWebViewContextPrivate(OxideQQuickWebViewContext* q);

  virtual ~OxideQQuickWebViewContextPrivate();

  oxide::BrowserContext* context() const {
    return context_.get();
  }

  oxide::BrowserContext* GetContext();

  LazyInitProperties* lazy_init_props() const {
    return lazy_init_props_.get();
  }

  static OxideQQuickWebViewContextPrivate* get(
      OxideQQuickWebViewContext* context);

 private:
  OxideQQuickWebViewContext* q_ptr;

  oxide::BrowserProcessHandle process_handle_;
  scoped_ptr<oxide::BrowserContext> context_;

  scoped_ptr<LazyInitProperties> lazy_init_props_;
};

#endif // _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_CONTEXT_P_H_
