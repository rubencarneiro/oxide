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

#ifndef _OXIDE_QT_LIB_PUBLIC_Q_WEB_FRAME_BASE_H_
#define _OXIDE_QT_LIB_PUBLIC_Q_WEB_FRAME_BASE_H_

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

class OxideQMessageHandlerBase;

namespace oxide {
namespace qt {

class QWebFrameBasePrivate;
class WebFrame;

}
}

class Q_DECL_EXPORT OxideQWebFrameBase : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)

  Q_DECLARE_PRIVATE(oxide::qt::QWebFrameBase)

 public:
  virtual ~OxideQWebFrameBase();

  QUrl url() const;

  Q_INVOKABLE void addMessageHandler(OxideQMessageHandlerBase* handler);
  Q_INVOKABLE void removeMessageHandler(OxideQMessageHandlerBase* handler);

 Q_SIGNALS:
  void urlChanged();
  void messageHandlersChanged();

 protected:
  OxideQWebFrameBase(oxide::qt::QWebFrameBasePrivate& dd);

  QScopedPointer<oxide::qt::QWebFrameBasePrivate> d_ptr;
};

#endif // _OXIDE_QT_LIB_PUBLIC_Q_WEB_FRAME_BASE_H_
