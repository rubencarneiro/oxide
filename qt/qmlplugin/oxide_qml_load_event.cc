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

#include "oxide_qml_load_event.h"

#include <QMetaType>

namespace oxide {
namespace qmlplugin {

LoadEvent::LoadEvent(QObject* parent)
    : QQmlValueTypeBase<OxideQLoadEvent>(qMetaTypeId<OxideQLoadEvent>(),
                                         parent) {}

LoadEvent::~LoadEvent() {}

QUrl LoadEvent::url() const {
  return v.url();
}

LoadEvent::Type LoadEvent::type() const {
  return static_cast<Type>(v.type());
}

LoadEvent::ErrorDomain LoadEvent::errorDomain() const {
  return static_cast<ErrorDomain>(v.errorDomain());
}

QString LoadEvent::errorString() const {
  return v.errorString();
}

int LoadEvent::errorCode() const {
  return v.errorCode();
}

int LoadEvent::httpStatusCode() const {
  return v.httpStatusCode();
}

QUrl LoadEvent::originalUrl() const {
  return v.originalUrl();
}

bool LoadEvent::isError() const {
  return v.isError();
}

QString LoadEvent::toString() const {
  return QString();
}

bool LoadEvent::isEqual(const QVariant& other) const {
  if (other.userType() != qMetaTypeId<OxideQLoadEvent>()) {
    return false;
  }

  return v == other.value<OxideQLoadEvent>();
}

} // namespace qmlplugin
} // namespace oxide
