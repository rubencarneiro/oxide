// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_API_USER_SCRIPT_P_P_H_
#define _OXIDE_QT_QUICK_API_USER_SCRIPT_P_P_H_

#include <QObject>
#include <QUrl>

#include "qt/core/glue/oxide_qt_user_script_proxy.h"
#include "qt/core/glue/oxide_qt_user_script_proxy_client.h"

#include "qt/quick/api/oxideqquickuserscript.h"

class OxideQQuickUserScriptPrivate
    : public QObject,
      public oxide::qt::UserScriptProxyHandle,
      public oxide::qt::UserScriptProxyClient {
  Q_OBJECT
  Q_DECLARE_PUBLIC(OxideQQuickUserScript)
  OXIDE_Q_DECL_PROXY_HANDLE_CONVERTER(OxideQQuickUserScript,
                                      oxide::qt::UserScriptProxyHandle);

 public:
  ~OxideQQuickUserScriptPrivate() override;

  static OxideQQuickUserScriptPrivate* get(OxideQQuickUserScript* user_script);

 Q_SIGNALS:
  void willBeDeleted();

 private:
  OxideQQuickUserScriptPrivate(OxideQQuickUserScript* q);

  // oxide::qt::UserScriptProxyClient implementation
  void ScriptLoadFailed() override;
  void ScriptLoaded() override;

  QUrl url_;

  struct ConstructProps;
  QScopedPointer<ConstructProps> construct_props_;

  Q_DISABLE_COPY(OxideQQuickUserScriptPrivate);
};

#endif // _OXIDE_QT_QUICK_API_USER_SCRIPT_P_P_H_
