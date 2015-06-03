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

#ifndef OXIDE_Q_BASIC_AUTHENTICATION_REQUEST
#define OXIDE_Q_BASIC_AUTHENTICATION_REQUEST

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

class OxideQBasicAuthenticationRequestPrivate;

namespace oxide {
  class ResourceDispatcherHostLoginDelegate;
}

class Q_DECL_EXPORT OxideQBasicAuthenticationRequest : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString realm READ realm NOTIFY realmChanged)

    Q_DECLARE_PRIVATE(OxideQBasicAuthenticationRequest)
    Q_DISABLE_COPY(OxideQBasicAuthenticationRequest)

   public:
    OxideQBasicAuthenticationRequest(oxide::ResourceDispatcherHostLoginDelegate*
                                     login_delegate);
    ~OxideQBasicAuthenticationRequest();

    QString realm() const;

    Q_INVOKABLE void allow(const QString& username, const QString& password);
    Q_INVOKABLE void deny();

   Q_SIGNALS:
    void realmChanged() const;
    void cancelled();

   private:
    QScopedPointer<OxideQBasicAuthenticationRequestPrivate> d_ptr;
};

#endif // OXIDE_Q_BASIC_AUTHENTICATION_REQUEST
