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

#ifndef _OXIDE_QMLPLUGIN_VALUE_TYPE_PROVIDER_H_
#define _OXIDE_QMLPLUGIN_VALUE_TYPE_PROVIDER_H_

#include <QtGlobal>
#include <QtQml/private/qqmlglobal_p.h>

namespace oxide {
namespace qmlplugin {

class ValueTypeProvider : public QQmlValueTypeProvider {
 public:
  ValueTypeProvider();
  ~ValueTypeProvider() override;

 private:

  template<typename T>
  bool create(QQmlValueType*& v) {
    v = new T;
    return true;
  }

  template<typename T>
  bool init(void* data, size_t data_size) {
    Q_ASSERT(data_size >= sizeof(T));
    T* t = reinterpret_cast<T*>(data);
    new (t) T();
    return true;
  }

  template<typename T>
  bool destroy(void* data, size_t data_size) {
    Q_ASSERT(data_size >= sizeof(T));
    T* t = reinterpret_cast<T*>(data);
    t->~T();
    return true;
  }

  template<typename T>
  bool copy(const void* src, void* dst, size_t dst_size) {
    Q_ASSERT(dst_size >= sizeof(T));
    const T* srcT = reinterpret_cast<const T*>(src);
    T* dstT = reinterpret_cast<T*>(dst);
    new (dstT) T(*srcT);
    return true;
  }

  template<typename T>
  bool equal(const void* lhs, const void* rhs) {
    *reinterpret_cast<const T*>(lhs) == *reinterpret_cast<const T*>(rhs);
  }

  template<typename T>
  bool store(const void* src, void* dst, size_t dst_size) {
    Q_ASSERT(dst_size >= sizeof(T));
    const T* srcT = reinterpret_cast<const T*>(src);
    T* dstT = reinterpret_cast<T*>(dst);
    new (dstT) T(*srcT);
    return true;
  }

  template<typename T>
  bool read(int src_type,
            const void* src,
            size_t src_size,
            int dst_type,
            void* dst) {
    T* dstT = reinterpret_cast<T*>(dst);
    if (src_type == dst_type) {
      Q_ASSERT(src_size >= sizeof(T));
      const T* srcT = reinterpret_cast<const T*>(src);
      *dstT = *srcT;
    } else {
      *dstT = T();
    }
    return true;
  }

  template<typename T>
  bool write(const void* src, void* dst, size_t dst_size) {
    Q_ASSERT(dst_size >= sizeof(T));
    const T* srcT = reinterpret_cast<const T*>(src);
    T* dstT = reinterpret_cast<T*>(dst);
    if (*dstT != *srcT) {
      *dstT = *srcT;
      return true;
    }
    return false;
  }

  // QQmlValueTypeProvider implementation
  bool create(int type, QQmlValueType*& v) override;
  bool init(int type, void* data, size_t data_size) override;
  bool destroy(int type, void* data, size_t data_size) override;
  bool copy(int type, const void* src, void* dst, size_t dst_size) override;

  bool create(int type, int argc, const void* argv[], QVariant* v) override;
  bool createFromString(int type,
                        const QString& s,
                        void* data,
                        size_t data_size) override;
  bool createStringFrom(int type, const void* data, QString* s) override;
  bool variantFromString(const QString& s, QVariant* v) override;
  bool variantFromString(int type, const QString& s, QVariant* v) override;
  bool variantFromJsObject(int type,
                           QQmlV4Handle h,
                           QV8Engine* e,
                           QVariant* v) override;
  bool equal(int type,
             const void* lhs,
             const void* rhs,
             size_t rhs_size) override;
  bool store(int type, const void* src, void* dst, size_t dst_size) override;
  bool read(int src_type,
            const void* src,
            size_t src_size,
            int dst_type,
            void* dst) override;
  bool write(int type, const void* src, void* dst, size_t dst_size) override;

  Q_DISABLE_COPY(ValueTypeProvider)
};

} // namespace qmlplugin
} // namespace oxide

#endif // _OXIDE_QMLPLUGIN_VALUE_TYPE_PROVIDER_H_
