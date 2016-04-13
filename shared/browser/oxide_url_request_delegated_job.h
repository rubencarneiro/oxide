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

#ifndef _OXIDE_SHARED_BROWSER_URL_REQUEST_DELEGATED_JOB_H_
#define _OXIDE_SHARED_BROWSER_URL_REQUEST_DELEGATED_JOB_H_

#include <string>

#include "base/compiler_specific.h"
#include "net/base/request_priority.h"
#include "net/http/http_request_headers.h"
#include "net/url_request/url_request_job.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class OXIDE_SHARED_EXPORT URLRequestDelegatedJob : public net::URLRequestJob {
 public:
  virtual ~URLRequestDelegatedJob();

 protected:
  URLRequestDelegatedJob(net::URLRequest* request,
                         net::NetworkDelegate* delegate);

  const net::HttpRequestHeaders& extra_request_headers() const {
    return extra_request_headers_;
  }

  net::RequestPriority priority() const { return priority_; }

  void set_mime_type(const std::string& mime_type) { mime_type_ = mime_type; }

 private:
  // net::URLRequestJob implementation
  void SetExtraRequestHeaders(const net::HttpRequestHeaders& headers) final;
  void SetPriority(net::RequestPriority priority) final;

  void Start() final;

  bool GetMimeType(std::string* mime_type) const final;

  virtual void OnStart() = 0;

  bool started_;

  net::HttpRequestHeaders extra_request_headers_;
  net::RequestPriority priority_;

  std::string mime_type_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_URL_REQUEST_DELEGATED_JOB_H_
