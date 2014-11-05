// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_BROWSER_URL_REQUEST_DELEGATED_JOB_H_
#define _OXIDE_QT_CORE_BROWSER_URL_REQUEST_DELEGATED_JOB_H_

#include <QNetworkReply>
#include <QtGlobal>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

#include "shared/browser/oxide_url_request_delegated_job.h"

QT_BEGIN_NAMESPACE
template <typename T> class QList;
class QSslError;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class CrossThreadDataStream;
class URLRequestDelegatedJobProxy;
class WebContext;

class URLRequestDelegatedJob final
    : public oxide::URLRequestDelegatedJob,
      public base::SupportsWeakPtr<URLRequestDelegatedJob> {
 public:
  URLRequestDelegatedJob(WebContext* context,
                         net::URLRequest* request,
                         net::NetworkDelegate* network_delegate);
  ~URLRequestDelegatedJob();

 private:
  friend class URLRequestDelegatedJobProxy;

  void OnStartError(const net::URLRequestStatus& status);
  void OnDidReceiveResponse(size_t size,
                            const std::string& mime_type);
  void OnDestroyed();
  void OnError(QNetworkReply::NetworkError code);
  void OnSslErrors(const QList<QSslError>& errors);

  void OnDataAvailable();

  // net::URLRequestJob implementation
  void Kill() final;

  bool ReadRawData(net::IOBuffer* buf, int buf_size, int* bytes_read) final;

  // oxide::URLRequestDelegatedJob implementation
  void OnStart() final;

  scoped_refptr<CrossThreadDataStream> stream_;
  scoped_refptr<URLRequestDelegatedJobProxy> proxy_;

  scoped_refptr<net::IOBuffer> read_buf_;
  int read_buf_size_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestDelegatedJob);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_URL_REQUEST_DELEGATED_JOB_H_
