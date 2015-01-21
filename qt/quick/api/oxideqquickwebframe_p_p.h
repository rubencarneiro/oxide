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

#ifndef _OXIDE_QT_QUICK_API_WEB_FRAME_P_P_H_
#define _OXIDE_QT_QUICK_API_WEB_FRAME_P_P_H_

#include <QList>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_web_frame_adapter.h"

class OxideQQuickScriptMessageHandler;
class OxideQQuickWebFrame;

QT_BEGIN_NAMESPACE
template <typename T> class QQmlListProperty;
QT_END_NAMESPACE

class OxideQQuickWebFramePrivate : public oxide::qt::WebFrameAdapter {
  Q_DECLARE_PUBLIC(OxideQQuickWebFrame)

 public:
  static OxideQQuickWebFramePrivate* get(OxideQQuickWebFrame* web_frame);

 private:
  OxideQQuickWebFramePrivate(OxideQQuickWebFrame* q);

  // WebFrameAdapter implementation
  void URLCommitted() override;

  QList<OxideQQuickWebFrame *>& children() {
    return children_;
  }

  static int childFrame_count(QQmlListProperty<OxideQQuickWebFrame>* prop);
  static OxideQQuickWebFrame* childFrame_at(
      QQmlListProperty<OxideQQuickWebFrame>* prop, int index);

  static int messageHandler_count(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop);
  static OxideQQuickScriptMessageHandler* messageHandler_at(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
      int index);

  // We keep this separate to QObject becase we want a way to track child frames quickly
  QList<OxideQQuickWebFrame *> children_;

  Q_DISABLE_COPY(OxideQQuickWebFramePrivate);
};

#endif // _OXIDE_QT_QUICK_API_WEB_FRAME_P_P_H_
