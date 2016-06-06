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

#include "oxide_qt_url_request_delegated_job.h"

#include <string>

#include <QNetworkAccessManager>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <QVariant>

#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_status.h"
#include "url/gurl.h"

#include "qt/core/browser/oxide_qt_web_context.h"
#include "shared/common/oxide_cross_thread_data_stream.h"

namespace oxide {
namespace qt {

namespace {

static int kBufferSize = 1024 * 512;

QNetworkRequest::Priority CalculateQNetworkRequestPriority(
    net::RequestPriority priority) {
  if (priority <= net::LOW) {
    return QNetworkRequest::LowPriority;
  }

  if (priority == net::MEDIUM) {
    return QNetworkRequest::NormalPriority;
  }

  return QNetworkRequest::HighPriority;
}

int CalculateNetworkError(QNetworkReply::NetworkError code) {
  // TODO: Actually calculate an appropriate error
  return net::ERR_FAILED;
}

}

class CrossThreadDataStream : public oxide::CrossThreadDataStream {
 public:
  CrossThreadDataStream() {}

  int64_t Write(QNetworkReply* reply);

 private:
  virtual ~CrossThreadDataStream() {}
};

int64_t CrossThreadDataStream::Write(QNetworkReply* reply) {
  DCHECK(CalledOnWriteThread());

  int64_t bytes_written = 0;

  while (CanAllocateSpaceForWriting() && reply->bytesAvailable() > 0) {
    int allocated = 0;
    int size = static_cast<int>(reply->bytesAvailable() & 0x7FFFFFFF);
    char* memory = AllocateSpaceForWriting(size, &allocated);
    int64_t res = reply->read(memory, allocated);
    CHECK(static_cast<int>(res) == allocated); // FIXME

    bytes_written += allocated;

    CommitWrite(reply->isFinished() && reply->bytesAvailable() == 0);
  }

  return bytes_written;
}

class URLRequestDelegatedJobProxy
    : public QObject,
      public base::RefCountedThreadSafe<URLRequestDelegatedJobProxy> {
  Q_OBJECT

 public:
  URLRequestDelegatedJobProxy(WebContextGetter* context_getter,
                              URLRequestDelegatedJob* job,
                              CrossThreadDataStream* stream);

  void Start(const std::string& method, const QNetworkRequest& req);
  void Kill();

 private Q_SLOTS:
  void OnDestroyed();
  void OnDataAvailable();
  void OnError(QNetworkReply::NetworkError code);
  void OnFinished();
  void OnSslErrors(const QList<QSslError>& errors);

 private:
  friend class base::RefCountedThreadSafe<URLRequestDelegatedJobProxy>;
  virtual ~URLRequestDelegatedJobProxy();

  void OnDidRead();

  void NotifyStartError(const net::URLRequestStatus& status);

  void DoStart(const std::string& method,
               const QNetworkRequest& req);
  void DoKill();

  void DoNotifyStartError(const net::URLRequestStatus& status);
  void DoNotifyDidReceiveResponse(size_t size,
                                  const std::string& mime_type);

  scoped_refptr<WebContextGetter> context_getter_;
  base::WeakPtr<URLRequestDelegatedJob> job_;

  scoped_refptr<CrossThreadDataStream> stream_;

  QPointer<QNetworkReply> reply_;

  bool did_receive_response_;
};

void URLRequestDelegatedJobProxy::OnDestroyed() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  if (!job_) {
    return;
  }

  job_->OnDestroyed();
}

void URLRequestDelegatedJobProxy::OnDataAvailable() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  if (!did_receive_response_) {
    did_receive_response_ = true;

    QVariant v;

    int64_t size = 0;
    QString mime;

    v = reply_->header(QNetworkRequest::ContentLengthHeader);
    if (v.isValid()) {
      DCHECK(v.canConvert(QVariant::LongLong));
      size = v.toLongLong();
    }

    v = reply_->header(QNetworkRequest::ContentTypeHeader);
    if (v.isValid()) {
      DCHECK(v.canConvert(QVariant::String));
      mime = v.toString();
    }

    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&URLRequestDelegatedJobProxy::DoNotifyDidReceiveResponse,
                   this, size, mime.toStdString()));
  }

  stream_->Write(reply_);
}

void URLRequestDelegatedJobProxy::OnError(QNetworkReply::NetworkError code) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  if (!job_) {
    return;
  }

  job_->OnError(code);
}

void URLRequestDelegatedJobProxy::OnFinished() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  stream_->Write(reply_);
}

void URLRequestDelegatedJobProxy::OnSslErrors(const QList<QSslError>& errors) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  if (!job_) {
    return;
  }

  job_->OnSslErrors(errors);
}

URLRequestDelegatedJobProxy::~URLRequestDelegatedJobProxy() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  stream_->SetDidReadCallback(base::Closure());

  if (reply_) {
    reply_->disconnect(this);
    delete reply_;
  }
}

void URLRequestDelegatedJobProxy::OnDidRead() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  if (!reply_) {
    return;
  }

  stream_->Write(reply_);
}

void URLRequestDelegatedJobProxy::NotifyStartError(
    const net::URLRequestStatus& status) {
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&URLRequestDelegatedJobProxy::DoNotifyStartError,
                 this, status));
}

void URLRequestDelegatedJobProxy::DoStart(const std::string& method,
                                          const QNetworkRequest& req) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  stream_->SetDidReadCallback(
      base::Bind(&URLRequestDelegatedJobProxy::OnDidRead,
                 // Callback is cleared in destructor, and is guaranteed
                 // to not run after that
                 base::Unretained(this)));

  WebContext* context = context_getter_->GetContext();
  if (!context) {
    NotifyStartError(net::URLRequestStatus(net::URLRequestStatus::FAILED,
                                           net::ERR_FAILED));
    return;    
  }

  QNetworkAccessManager* qnam = context->GetCustomNetworkAccessManager();
  if (!qnam) {
    NotifyStartError(net::URLRequestStatus(net::URLRequestStatus::FAILED,
                                           net::ERR_FAILED));
    return;    
  }

  DCHECK(method == "GET");
  reply_ = qnam->get(req);

  if (!reply_) {
    NotifyStartError(net::URLRequestStatus(net::URLRequestStatus::FAILED,
                                           net::ERR_FAILED));
    return;
  }

  connect(reply_, SIGNAL(destroyed()), SLOT(OnDestroyed()));
  connect(reply_, SIGNAL(readyRead()), SLOT(OnDataAvailable()),
          Qt::DirectConnection);
  connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)),
          SLOT(OnError(QNetworkReply::NetworkError)));
  connect(reply_, SIGNAL(finished()), SLOT(OnFinished()),
          Qt::DirectConnection);
  connect(reply_, SIGNAL(sslErrors(const QList<QSslError>&)),
          SLOT(OnSslErrors(const QList<QSslError>&)));
}

void URLRequestDelegatedJobProxy::DoKill() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  if (reply_) {
    reply_->abort();
  }
}

void URLRequestDelegatedJobProxy::DoNotifyStartError(
    const net::URLRequestStatus& status) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  if (!job_) {
    return;
  }

  job_->OnStartError(status);
}

void URLRequestDelegatedJobProxy::DoNotifyDidReceiveResponse(
    size_t size,
    const std::string& mime_type) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  if (!job_) {
    return;
  }

  job_->OnDidReceiveResponse(size, mime_type);
}

URLRequestDelegatedJobProxy::URLRequestDelegatedJobProxy(
    WebContextGetter* context_getter,
    URLRequestDelegatedJob* job,
    CrossThreadDataStream* stream)
    : context_getter_(context_getter),
      job_(job->AsWeakPtr()),
      stream_(stream),
      did_receive_response_(false) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
}

void URLRequestDelegatedJobProxy::Start(const std::string& method,
                                        const QNetworkRequest& req) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&URLRequestDelegatedJobProxy::DoStart,
                 this, method, req));
}

void URLRequestDelegatedJobProxy::Kill() {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&URLRequestDelegatedJobProxy::DoKill,
                 this));
}

void URLRequestDelegatedJob::OnStartError(const net::URLRequestStatus& status) {
  NotifyStartError(status);
}

void URLRequestDelegatedJob::OnDidReceiveResponse(
    size_t size,
    const std::string& mime_type) {
  set_expected_content_size(size);
  set_mime_type(mime_type);
  NotifyHeadersComplete();
}

void URLRequestDelegatedJob::OnDestroyed() {
  NotifyCanceled();
}

void URLRequestDelegatedJob::OnError(QNetworkReply::NetworkError code) {
  if (is_done()) {
    return;
  }

  if (has_response_started()) {
    ReadRawDataComplete(CalculateNetworkError(code));
  } else {
    NotifyStartError(net::URLRequestStatus(net::URLRequestStatus::FAILED,
                                           CalculateNetworkError(code)));
  }
}

void URLRequestDelegatedJob::OnSslErrors(const QList<QSslError>& errors) {
  if (is_done()) {
    return;
  }

  // TODO: Use NotifySSLCertificateError here, although this doesn't matter
  //  too much. As we're not able to allow this error asynchronously with
  //  Qt, all errors are fatal. Therefore, the effect is the same anyway
  NotifyStartError(net::URLRequestStatus(net::URLRequestStatus::FAILED,
                                         net::ERR_FAILED));
}

void URLRequestDelegatedJob::OnDataAvailable() {
  if (!GetStatus().is_io_pending()) {
    return;
  }

  DCHECK(read_buf_.get());
  DCHECK(!stream_->IsEOF());

  int rv = stream_->Read(read_buf_.get(), read_buf_size_);
  DCHECK_GT(rv, 0);

  read_buf_= nullptr;
  read_buf_size_ = 0;

  ReadRawDataComplete(rv);
}

void URLRequestDelegatedJob::Kill() {
  proxy_->Kill();
  net::URLRequestJob::Kill();
}

int URLRequestDelegatedJob::ReadRawData(net::IOBuffer* buf,
                                         int buf_size) {
  DCHECK_GE(buf_size, 0);
  DCHECK(GetStatus().is_success());

  if (stream_->IsEOF()) {
    return 0;
  }

  int rv = stream_->Read(buf, buf_size);
  if (rv > 0) {
    return rv;
  }

  read_buf_ = buf;
  read_buf_size_ = buf_size;

  return net::ERR_IO_PENDING;
}

void URLRequestDelegatedJob::OnStart() {
  if (!stream_->Initialize(kBufferSize)) {
    NotifyStartError(net::URLRequestStatus(net::URLRequestStatus::FAILED,
                                           net::ERR_INSUFFICIENT_RESOURCES));
    return;
  }

  if (request()->method() != "GET") {
    LOG(WARNING) << "Only GET requests are supported for now";
    NotifyStartError(net::URLRequestStatus(net::URLRequestStatus::FAILED,
                                           net::ERR_NOT_IMPLEMENTED));
    return;
  }

  QNetworkRequest req;
  req.setPriority(CalculateQNetworkRequestPriority(priority()));
  req.setUrl(QUrl(QString::fromStdString(request()->url().spec())));

  net::HttpRequestHeaders::Iterator iter(extra_request_headers());
  while (iter.GetNext()) {
    req.setRawHeader(iter.name().c_str(), iter.value().c_str());
  }

  proxy_->Start(request()->method(), req);
}

URLRequestDelegatedJob::URLRequestDelegatedJob(
    WebContextGetter* context_getter,
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate)
    : oxide::URLRequestDelegatedJob(request, network_delegate),
      stream_(new CrossThreadDataStream()),
      proxy_(new URLRequestDelegatedJobProxy(context_getter, this, stream_.get())),
      read_buf_size_(0) {
  stream_->SetDataAvailableCallback(
      base::Bind(&URLRequestDelegatedJob::OnDataAvailable,
                 // Callback is cleared in destructor, and is guaranteed
                 // to not run after that
                 base::Unretained(this)));
}

URLRequestDelegatedJob::~URLRequestDelegatedJob() {
  stream_->SetDataAvailableCallback(base::Closure());

  URLRequestDelegatedJobProxy* proxy = proxy_.get();
  proxy->AddRef();
  proxy_ = nullptr;
  content::BrowserThread::ReleaseSoon(
      content::BrowserThread::UI, FROM_HERE, proxy);
}

} // namespace qt
} // namespace oxide

#if defined(GN_BUILD)
#include "qt/core/browser/oxide_qt_url_request_delegated_job.moc"
#else
#include "oxide_qt_url_request_delegated_job.moc"
#endif
