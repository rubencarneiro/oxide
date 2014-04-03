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

#ifndef _OXIDE_QT_CORE_GLUE_USER_SCRIPT_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_USER_SCRIPT_ADAPTER_H_

#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_adapter_base.h"

namespace oxide {
namespace qt {

class UserScriptAdapterPrivate;

class Q_DECL_EXPORT UserScriptAdapter : public AdapterBase {
 public:
  virtual ~UserScriptAdapter();

  QUrl url() const;
  void setUrl(const QUrl& url);

  bool emulateGreasemonkey() const;
  void setEmulateGreasemonkey(bool emulate);

  bool matchAllFrames() const;
  void setMatchAllFrames(bool match);

  bool incognitoEnabled() const;
  void setIncognitoEnabled(bool enabled);

  QUrl context() const;
  void setContext(const QUrl& context);

  void init();

 protected:
  UserScriptAdapter(QObject* q);

 private:
  friend class UserScriptAdapterPrivate;

  virtual void OnScriptLoadFailed() = 0;
  virtual void OnScriptLoaded() = 0;

  QScopedPointer<UserScriptAdapterPrivate> priv;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_USER_SCRIPT_ADAPTER_H_
