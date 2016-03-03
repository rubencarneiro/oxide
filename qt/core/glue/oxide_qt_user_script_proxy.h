// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_USER_SCRIPT_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_USER_SCRIPT_PROXY_H_

#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_proxy_base.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class UserScript;
class UserScriptProxyClient;

class Q_DECL_EXPORT UserScriptProxy : public ProxyBase<UserScript> {
 public:
  static UserScriptProxy* create(UserScriptProxyClient* client,
                                 QObject* handle,
                                 const QUrl& url);
  virtual ~UserScriptProxy();

  virtual bool emulateGreasemonkey() const = 0;
  virtual void setEmulateGreasemonkey(bool emulate) = 0;

  virtual bool matchAllFrames() const = 0;
  virtual void setMatchAllFrames(bool match) = 0;

  virtual bool incognitoEnabled() const = 0;
  virtual void setIncognitoEnabled(bool enabled) = 0;

  virtual QUrl context() const = 0;
  virtual void setContext(const QUrl& context) = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_USER_SCRIPT_PROXY_H_
