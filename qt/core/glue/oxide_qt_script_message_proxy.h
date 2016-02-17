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

#ifndef _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_PROXY_H_

#include <QString>
#include <QtGlobal>
#include <QUrl>
#include <QVariant>

namespace oxide {
namespace qt {

class ScriptMessage;

class Q_DECL_EXPORT ScriptMessageProxy {
 public:
  virtual ~ScriptMessageProxy() {}

  virtual QObject* frame() const = 0;
  virtual QString msgId() const = 0;
  virtual QUrl context() const = 0;
  virtual QVariant payload() const = 0;
  virtual void reply(const QVariant& payload) = 0;
  virtual void error(const QVariant& payload) = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_PROXY_H_
