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

#ifndef _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_ADAPTER_H_

#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>
#include <QVariant>

#include "qt/core/glue/oxide_qt_adapter_base.h"
#include "qt/core/glue/oxide_qt_proxy_handle.h"

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ScriptMessage;
class WebFrameProxy;

OXIDE_Q_DECL_PROXY_HANDLE(WebFrameProxy);

class Q_DECL_EXPORT ScriptMessageAdapter : public AdapterBase {
 public:
  virtual ~ScriptMessageAdapter();

  WebFrameProxyHandle* frame() const;
  QString msgId() const;
  QUrl context() const;
  QVariant args() const;
  void reply(const QVariant& args);
  void error(const QString& msg);

 protected:
  ScriptMessageAdapter(QObject* q);

 private:
  friend class ScriptMessage;

  mutable QVariant args_;
  QScopedPointer<ScriptMessage> message_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_ADAPTER_H_
