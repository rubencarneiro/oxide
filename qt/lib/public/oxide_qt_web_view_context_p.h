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

#ifndef _OXIDE_QT_LIB_PUBLIC_WEB_VIEW_CONTEXT_P_H_
#define _OXIDE_QT_LIB_PUBLIC_WEB_VIEW_CONTEXT_P_H_

#include <QList>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_browser_process_handle.h"

class OxideQUserScript;

namespace oxide {

class BrowserContext;

namespace qt {

class LazyInitProperties;
class WebViewContext;

class WebViewContextPrivate {
  Q_DECLARE_PUBLIC(WebViewContext)

 public:
  virtual ~WebViewContextPrivate();

  oxide::BrowserContext* context() const {
    return context_.get();
  }

  oxide::BrowserContext* GetContext();

  LazyInitProperties* lazy_init_props() const {
    return lazy_init_props_.get();
  }
  QList<OxideQUserScript *>& user_scripts() {
    return user_scripts_;
  }

  static WebViewContextPrivate* get(WebViewContext* context);

  void updateUserScripts();

 protected:
  WebViewContextPrivate(WebViewContext* q);

  WebViewContext* q_ptr;

 private:
  oxide::BrowserProcessHandle process_handle_;
  scoped_ptr<oxide::BrowserContext> context_;
  QList<OxideQUserScript *> user_scripts_;

  scoped_ptr<LazyInitProperties> lazy_init_props_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebViewContextPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_PUBLIC_WEB_VIEW_CONTEXT_P_H_
