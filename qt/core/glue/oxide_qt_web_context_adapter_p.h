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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_ADAPTER_P_H_
#define _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_ADAPTER_P_H_

#include <QList>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

namespace oxide {

class BrowserContext;

namespace qt {

struct ConstructProperties;
class UserScriptAdapter;
class WebContextAdapter;

class WebContextAdapterPrivate FINAL {
 public:
  WebContextAdapterPrivate();

  void Init();

  static WebContextAdapterPrivate* get(WebContextAdapter* adapter);

  oxide::BrowserContext* context() { return context_.get(); }
  ConstructProperties* construct_props() { return construct_props_.get(); }

  QList<UserScriptAdapter *> user_scripts;

 private:
  scoped_ptr<oxide::BrowserContext> context_;
  scoped_ptr<ConstructProperties> construct_props_;

  DISALLOW_COPY_AND_ASSIGN(WebContextAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_CONTEXT_ADAPTER_P_H_
