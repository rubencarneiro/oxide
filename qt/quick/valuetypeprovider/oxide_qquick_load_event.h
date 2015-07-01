// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef _OXIDE_QQUICK_VALUETYPEPROVIDER_LOAD_EVENT_H_
#define _OXIDE_QQUICK_VALUETYPEPROVIDER_LOAD_EVENT_H_

#include <QtGlobal>
#include <QtQml/private/qqmlvaluetype_p.h>

#include "qt/core/api/oxideqloadevent.h"

namespace oxide {
namespace qquick {

class Q_DECL_EXPORT LoadEvent : public QQmlValueTypeBase<OxideQLoadEvent> {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(Type type READ type CONSTANT)
  Q_PROPERTY(ErrorDomain errorDomain READ errorDomain CONSTANT)
  Q_PROPERTY(QString errorString READ errorString CONSTANT)
  Q_PROPERTY(int errorCode READ errorCode CONSTANT)
  Q_PROPERTY(int httpStatusCode READ httpStatusCode CONSTANT REVISION 2)
  Q_PROPERTY(QUrl originalUrl READ originalUrl CONSTANT)
  Q_PROPERTY(bool isError READ isError CONSTANT REVISION 1)

  Q_ENUMS(Type)
  Q_ENUMS(ErrorDomain)

 public:
  LoadEvent(QObject* parent = nullptr);
  ~LoadEvent() override;

  enum Type {
    TypeStarted = OxideQLoadEvent::TypeStarted,
    TypeStopped = OxideQLoadEvent::TypeStopped,
    TypeSucceeded = OxideQLoadEvent::TypeSucceeded,
    TypeFailed = OxideQLoadEvent::TypeFailed,
    TypeCommitted = OxideQLoadEvent::TypeCommitted,
    TypeRedirected = OxideQLoadEvent::TypeRedirected
  };

  enum ErrorDomain {
    ErrorDomainNone = OxideQLoadEvent::ErrorDomainNone,
    ErrorDomainInternal = OxideQLoadEvent::ErrorDomainInternal,
    ErrorDomainConnection = OxideQLoadEvent::ErrorDomainConnection,
    ErrorDomainCertificate = OxideQLoadEvent::ErrorDomainCertificate,
    ErrorDomainHTTP = OxideQLoadEvent::ErrorDomainHTTP,
    ErrorDomainCache = OxideQLoadEvent::ErrorDomainCache,
    ErrorDomainFTP = OxideQLoadEvent::ErrorDomainFTP,
    ErrorDomainDNS = OxideQLoadEvent::ErrorDomainDNS
  };

  QUrl url() const;
  Type type() const;
  ErrorDomain errorDomain() const;
  QString errorString() const;
  int errorCode() const;
  int httpStatusCode() const;
  QUrl originalUrl() const;
  bool isError() const;

  // QQmlValueType implementation
  QString toString() const;
  bool isEqual(const QVariant& other) const;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QQUICK_VALUETYPEPROVIDER_LOAD_EVENT_H_
