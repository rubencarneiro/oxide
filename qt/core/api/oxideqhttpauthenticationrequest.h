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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef OXIDE_Q_HTTP_AUTHENTICATION_REQUEST
#define OXIDE_Q_HTTP_AUTHENTICATION_REQUEST

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

class OxideQHttpAuthenticationRequestPrivate;

namespace oxide {
namespace qt {
class WebView;
}
class ResourceDispatcherHostLoginDelegate;
}

class Q_DECL_EXPORT OxideQHttpAuthenticationRequest : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString host READ host CONSTANT)
  Q_PROPERTY(QString realm READ realm CONSTANT)

  Q_DECLARE_PRIVATE(OxideQHttpAuthenticationRequest)
  Q_DISABLE_COPY(OxideQHttpAuthenticationRequest)

 public:
  ~OxideQHttpAuthenticationRequest();

  QString host() const;
  QString realm() const;

  Q_INVOKABLE void allow(const QString& username, const QString& password);
  Q_INVOKABLE void deny();

 Q_SIGNALS:
  void cancelled() const;

 private:
  Q_DECL_HIDDEN OxideQHttpAuthenticationRequest(
      oxide::ResourceDispatcherHostLoginDelegate* login_delegate);

  QScopedPointer<OxideQHttpAuthenticationRequestPrivate> d_ptr;
};

#endif // OXIDE_Q_HTTP_AUTHENTICATION_REQUEST
