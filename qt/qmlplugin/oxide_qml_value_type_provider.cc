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

#include "oxide_qml_value_type_provider.h"

#include "qt/core/api/oxideqdownloadrequest.h"
#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqsslcertificate.h"

#include "oxide_qml_download_request.h"
#include "oxide_qml_load_event.h"
#include "oxide_qml_ssl_certificate.h"

namespace oxide {
namespace qmlplugin {

bool ValueTypeProvider::create(int type, QQmlValueType*& v) {
  if (type == qMetaTypeId<OxideQLoadEvent>()) {
    return create<LoadEvent>(v);
  } else if (type == qMetaTypeId<OxideQDownloadRequest>()) {
    return create<DownloadRequest>(v);
  } else if (type == qMetaTypeId<OxideQSslCertificate>()) {
    return create<SslCertificate>(v);
  }

  return false;
}

bool ValueTypeProvider::init(int type, void* data, size_t data_size) {
  if (type == qMetaTypeId<OxideQLoadEvent>()) {
    return init<OxideQLoadEvent>(data, data_size);
  } else if (type == qMetaTypeId<OxideQDownloadRequest>()) {
    return init<OxideQDownloadRequest>(data, data_size);
  } else if (type == qMetaTypeId<OxideQSslCertificate>()) {
    return init<OxideQSslCertificate>(data, data_size);
  }

  return false;
}

bool ValueTypeProvider::destroy(int type, void* data, size_t data_size) {
  if (type == qMetaTypeId<OxideQLoadEvent>()) {
    return destroy<OxideQLoadEvent>(data, data_size);
  } else if (type == qMetaTypeId<OxideQDownloadRequest>()) {
    return destroy<OxideQDownloadRequest>(data, data_size);
  } else if (type == qMetaTypeId<OxideQSslCertificate>()) {
    return destroy<OxideQSslCertificate>(data, data_size);
  }

  return false;
}

bool ValueTypeProvider::copy(int type,
                             const void* src,
                             void* dst,
                             size_t dst_size) {
  if (type == qMetaTypeId<OxideQLoadEvent>()) {
    return copy<OxideQLoadEvent>(src, dst, dst_size);
  } else if (type == qMetaTypeId<OxideQDownloadRequest>()) {
    return copy<OxideQDownloadRequest>(src, dst, dst_size);
  } else if (type == qMetaTypeId<OxideQSslCertificate>()) {
    return copy<OxideQSslCertificate>(src, dst, dst_size);
  }

  return false;
}

bool ValueTypeProvider::create(int type,
                               int argc,
                               const void* argv[],
                               QVariant* v) {
  return false;
}

bool ValueTypeProvider::createFromString(int type,
                                         const QString& s,
                                         void* data,
                                         size_t data_size) {
  return false;
}

bool ValueTypeProvider::createStringFrom(int type,
                                         const void* data,
                                         QString* s) {
  return false;
}

bool ValueTypeProvider::variantFromString(const QString& s, QVariant* v) {
  return false;
}

bool ValueTypeProvider::variantFromString(int type,
                                          const QString& s,
                                          QVariant* v) {
  return false;
}

bool ValueTypeProvider::variantFromJsObject(int type,
                                            QQmlV4Handle h,
                                            QV8Engine* e,
                                            QVariant* v) {
  return false;
}

bool ValueTypeProvider::equal(int type,
                              const void* lhs,
                              const void* rhs,
                              size_t rhs_size) {
  if (type == qMetaTypeId<OxideQLoadEvent>()) {
    return equal<OxideQLoadEvent>(lhs, rhs);
  } else if (type == qMetaTypeId<OxideQDownloadRequest>()) {
    return equal<OxideQDownloadRequest>(lhs, rhs);
  } else if (type == qMetaTypeId<OxideQSslCertificate>()) {
    return equal<OxideQSslCertificate>(lhs, rhs);
  }

  return false;
}

bool ValueTypeProvider::store(int type,
                              const void* src,
                              void* dst,
                              size_t dst_size) {
  if (type == qMetaTypeId<OxideQLoadEvent>()) {
    return store<OxideQLoadEvent>(src, dst, dst_size);
  } else if (type == qMetaTypeId<OxideQDownloadRequest>()) {
    return store<OxideQDownloadRequest>(src, dst, dst_size);
  } else if (type == qMetaTypeId<OxideQSslCertificate>()) {
    return store<OxideQSslCertificate>(src, dst, dst_size);
  }

  return false;
}

bool ValueTypeProvider::read(int src_type,
                             const void* src,
                             size_t src_size,
                             int dst_type,
                             void* dst) {
  if (src_type == qMetaTypeId<OxideQLoadEvent>()) {
    return read<OxideQLoadEvent>(src_type, src, src_size, dst_type, dst);
  } else if (src_type == qMetaTypeId<OxideQDownloadRequest>()) {
    return read<OxideQDownloadRequest>(src_type, src, src_size, dst_type, dst);
  } else if (src_type == qMetaTypeId<OxideQSslCertificate>()) {
    return read<OxideQSslCertificate>(src_type, src, src_size, dst_type, dst);
  }

  return false;
}

bool ValueTypeProvider::write(int type,
                              const void* src,
                              void* dst,
                              size_t dst_size) {
  if (type == qMetaTypeId<OxideQLoadEvent>()) {
    return write<OxideQLoadEvent>(src, dst, dst_size);
  } else if (type == qMetaTypeId<OxideQDownloadRequest>()) {
    return write<OxideQDownloadRequest>(src, dst, dst_size);
  } else if (type == qMetaTypeId<OxideQSslCertificate>()) {
    return write<OxideQSslCertificate>(src, dst, dst_size);
  }

  return false;
}

ValueTypeProvider::ValueTypeProvider() {}

ValueTypeProvider::~ValueTypeProvider() {}

} // namespace qmlplugin
} // namespace oxide
