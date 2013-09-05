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

#ifndef _OXIDE_QT_LIB_API_QWEB_VIEW_CONTEXT_P_H_
#define _OXIDE_QT_LIB_API_QWEB_VIEW_CONTEXT_P_H_

#include <QList>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_browser_process_handle.h"

#include "qt/lib/api/public/oxide_qquick_web_view_context_p.h"

class OxideQUserScript;
class OxideQWebViewContextBase;

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

class QWebViewContextBasePrivate {
  Q_DECLARE_PUBLIC(OxideQWebViewContextBase)

 public:
  virtual ~QWebViewContextBasePrivate();

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

  static QWebViewContextBasePrivate* get(OxideQWebViewContextBase* context);

  void updateUserScripts();

 protected:
  QWebViewContextBasePrivate(OxideQWebViewContextBase* q);

  OxideQWebViewContextBase* q_ptr;

 private:
  oxide::BrowserProcessHandle process_handle_;
  scoped_ptr<oxide::BrowserContext> context_;
  QList<OxideQUserScript *> user_scripts_;

  scoped_ptr<LazyInitProperties> lazy_init_props_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(QWebViewContextBasePrivate);
};

class QQuickWebViewContextPrivate FINAL : public QWebViewContextBasePrivate {
  Q_DECLARE_PUBLIC(OxideQQuickWebViewContext)

 public:
  static QQuickWebViewContextPrivate* Create(OxideQQuickWebViewContext* q);

  ~QQuickWebViewContextPrivate();

  static QQuickWebViewContextPrivate* get(
      OxideQQuickWebViewContext* context);

  static void userScript_append(QQmlListProperty<OxideQUserScript>* prop,
                                OxideQUserScript* user_script);
  static int userScript_count(QQmlListProperty<OxideQUserScript>* prop);
  static OxideQUserScript* userScript_at(
      QQmlListProperty<OxideQUserScript>* prop,
      int index);
  static void userScript_clear(QQmlListProperty<OxideQUserScript>* prop);

 private:
  QQuickWebViewContextPrivate(OxideQQuickWebViewContext* q);

  DISALLOW_COPY_AND_ASSIGN(QQuickWebViewContextPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_API_QWEB_VIEW_CONTEXT_P_H_
