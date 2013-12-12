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

#ifndef _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_CONTEXT_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_CONTEXT_ADAPTER_H_

#include <string>

#include <QList>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

namespace oxide {

class BrowserContext;
class BrowserProcessMain;

namespace qt {

struct LazyInitProperties;
class UserScriptAdapter;
class WebContextAdapter;

class WebContextAdapterPrivate FINAL {
 public:
  static WebContextAdapterPrivate* Create();
  ~WebContextAdapterPrivate();

  std::string GetProduct() const;
  void SetProduct(const std::string& product);

  std::string GetUserAgent() const;
  void SetUserAgent(const std::string& user_agent);

  base::FilePath GetDataPath() const;
  void SetDataPath(const base::FilePath& path);

  base::FilePath GetCachePath() const;
  void SetCachePath(const base::FilePath& path);

  std::string GetAcceptLangs() const;
  void SetAcceptLangs(const std::string& langs);

  QList<UserScriptAdapter *>& user_scripts() {
    return user_scripts_;
  }

  oxide::BrowserContext* context() { return context_.get(); }

  void UpdateUserScripts();

  void CompleteConstruction();

  static WebContextAdapterPrivate* get(WebContextAdapter* adapter);

 private:
  WebContextAdapterPrivate();

  // BrowserProcesMain needs to outlive BrowserContext. The reason
  // that the reference to BrowserProcessMain is here and not in
  // BrowserContext is because BrowserProcessMain has to outlive
  // the destructor for base::SupportsUserData, from which
  // BrowserContext inherits, because some data needs to be deleted
  // on the IO thread
  scoped_refptr<oxide::BrowserProcessMain> process_handle_;
  scoped_ptr<oxide::BrowserContext> context_;
  QList<UserScriptAdapter *> user_scripts_;

  scoped_ptr<LazyInitProperties> lazy_init_props_;

  DISALLOW_COPY_AND_ASSIGN(WebContextAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_CONTEXT_ADAPTER_H_
