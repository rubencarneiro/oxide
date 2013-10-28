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

#define OXIDE_QT_DECLARE_ADAPTER \
  QObject* _q_func() const { return reinterpret_cast<QObject *>(q_ptr); }

namespace oxide {
namespace qt {

class AdapterBase {
 public:
  virtual ~AdapterBase() {}

  virtual QObject* _q_func() const = 0;
};

} // namespace qt
} // namespace oxide

template <typename T>
inline T* adapterToQObject(oxide::qt::AdapterBase* a) {
  return qobject_cast<T *>(a->_q_func());
}

inline QObject* adapterToQObject(oxide::qt::AdapterBase* a) {
  return qobject_cast<QObject *>(a->_q_func());
}

#endif // _OXIDE_QT_CORE_GLUE_ADAPTER_BASE_H_
