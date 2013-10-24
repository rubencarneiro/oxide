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

#ifndef _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_P_H_
#define _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_P_H_

#include <QList>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_browser_process_handle.h"

class OxideQQuickWebContext;
class OxideQQuickUserScript;

QT_BEGIN_NAMESPACE
template <typename T> class QQmlListProperty;
QT_END_NAMESPACE

namespace oxide {
class BrowserContext;

namespace qt {
struct LazyInitProperties {
  std::string product;
  std::string user_agent;
  base::FilePath data_path;
  base::FilePath cache_path;
  std::string accept_langs;
};

class QQuickWebContextPrivate FINAL {
  Q_DECLARE_PUBLIC(OxideQQuickWebContext)

 public:
  static QQuickWebContextPrivate* Create(OxideQQuickWebContext* q);

  oxide::BrowserContext* context() const {
    return context_.get();
  }

  oxide::BrowserContext* GetContext();

  LazyInitProperties* lazy_init_props() const {
    return lazy_init_props_.get();
  }
  QList<OxideQQuickUserScript *>& user_scripts() {
    return user_scripts_;
  }

  static QQuickWebContextPrivate* get(OxideQQuickWebContext* context);

  void updateUserScripts();

  static void userScript_append(QQmlListProperty<OxideQQuickUserScript>* prop,
                                OxideQQuickUserScript* user_script);
  static int userScript_count(QQmlListProperty<OxideQQuickUserScript>* prop);
  static OxideQQuickUserScript* userScript_at(
      QQmlListProperty<OxideQQuickUserScript>* prop,
      int index);
  static void userScript_clear(QQmlListProperty<OxideQQuickUserScript>* prop);

 private:
  QQuickWebContextPrivate(OxideQQuickWebContext* q);

  oxide::BrowserProcessHandle process_handle_;
  scoped_ptr<oxide::BrowserContext> context_;
  QList<OxideQQuickUserScript *> user_scripts_;

  scoped_ptr<LazyInitProperties> lazy_init_props_;

  OxideQQuickWebContext* q_ptr;

  DISALLOW_COPY_AND_ASSIGN(QQuickWebContextPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_QUICK_API_WEB_CONTEXT_P_P_H_
