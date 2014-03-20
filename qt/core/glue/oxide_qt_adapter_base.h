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

#ifndef _OXIDE_QT_CORE_GLUE_ADAPTER_BASE_H_
#define _OXIDE_QT_CORE_GLUE_ADAPTER_BASE_H_

#include <QObject>
#include <QtGlobal>

namespace oxide {
namespace qt {

class AdapterBase {
 public:
  virtual ~AdapterBase() {}

  QObject* api_handle() { return q_ptr; }

 protected:
  AdapterBase(QObject* q) :
      q_ptr(q) {
    Q_ASSERT(q);
  }
  AdapterBase();

  QObject* q_ptr;
};

} // namespace qt
} // namespace oxide

template <typename T>
inline T* adapterToQObject(oxide::qt::AdapterBase* a) {
  if (!a) return NULL;
  return qobject_cast<T *>(a->api_handle());
}

inline QObject* adapterToQObject(oxide::qt::AdapterBase* a) {
  if (!a) return NULL;
  return qobject_cast<QObject *>(a->api_handle());
}

#endif // _OXIDE_QT_CORE_GLUE_ADAPTER_BASE_H_
