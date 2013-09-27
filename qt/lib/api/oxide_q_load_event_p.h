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

#ifndef _OXIDE_QT_LIB_API_Q_LOAD_EVENT_P_H_
#define _OXIDE_QT_LIB_API_Q_LOAD_EVENT_P_H_

#include <QString>
#include <QUrl>

#include "base/basictypes.h"
#include "base/compiler_specific.h"

#include "qt/lib/api/public/oxide_q_load_event.h"

namespace oxide {
namespace qt {

class QLoadEventPrivate FINAL {
 public:
  static QLoadEventPrivate* Create(const QUrl& url,
                                   OxideQLoadEvent::Type type,
                                   int error_code,
                                   const QString& error_description);

  QUrl url() const;
  OxideQLoadEvent::Type type() const;
  OxideQLoadEvent::ErrorCode error() const;
  QString errorString() const;

 private:
  QLoadEventPrivate(const QUrl& url,
                     OxideQLoadEvent::Type type,
                     OxideQLoadEvent::ErrorCode error,
                     const QString& error_description);

  static OxideQLoadEvent::ErrorCode ChromeErrorCodeToOxideErrorCode(
      int error_code);

  QUrl url_;
  OxideQLoadEvent::Type type_;
  OxideQLoadEvent::ErrorCode error_;
  QString error_string_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(QLoadEventPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_API_Q_LOAD_EVENT_P_H_
