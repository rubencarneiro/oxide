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

#ifndef _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_HANDLER_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_HANDLER_ADAPTER_H_

#include <string>

#include <QList>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_adapter_base.h"

class OxideQScriptMessage;

namespace oxide {
namespace qt {

class ScriptMessageHandlerAdapterPrivate;
class WebFrameAdapter;

class Q_DECL_EXPORT ScriptMessageHandlerAdapter : public AdapterBase {
 public:
  virtual ~ScriptMessageHandlerAdapter();

  QString msgId() const;
  void setMsgId(const QString& id);

  QList<QUrl> contexts() const;
  void setContexts(const QList<QUrl>& contexts);

  void attachHandler();
  void detachHandler();

 protected:
  ScriptMessageHandlerAdapter(QObject* q);

 private:
  friend class ScriptMessageHandlerAdapterPrivate;

  virtual bool OnReceiveMessage(OxideQScriptMessage* message,
                                WebFrameAdapter* frame,
                                QString& error) = 0;

  QScopedPointer<ScriptMessageHandlerAdapterPrivate> priv;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_HANDLER_ADAPTER_H_
