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

#include "oxide_file_utils.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/files/file_util_proxy.h"
#include "base/message_loop/message_loop_proxy.h"

namespace oxide {

class AsyncFileJobImpl : public base::RefCounted<AsyncFileJobImpl> {
 public:
  AsyncFileJobImpl(base::TaskRunner* task_runner) :
      task_runner_(task_runner),
      cancelled_(false) {}

  virtual ~AsyncFileJobImpl() {}

  virtual void Run() = 0;
  void Cancel() { cancelled_ = true; }

  base::TaskRunner* task_runner() const { return task_runner_.get(); }
  bool cancelled() const { return cancelled_; }

 private:
  scoped_refptr<base::TaskRunner> task_runner_;
  bool cancelled_;
};

namespace {

class GetFileContentsJobImpl : public AsyncFileJobImpl {
 public:
  GetFileContentsJobImpl(base::TaskRunner* task_runner,
                         const base::FilePath& file_path,
                         const FileUtils::GetFileContentsCallback& callback) :
      AsyncFileJobImpl(task_runner),
      file_path_(file_path),
      callback_(callback),
      file_(base::kInvalidPlatformFileValue) {}

  ~GetFileContentsJobImpl() {
    if (file_ != base::kInvalidPlatformFileValue) {
      base::FileUtilProxy::Close(
          task_runner(), file_,
          base::Bind(&GetFileContentsJobImpl::OnFileClosed));
    }
  }

 private:
  void Run() {
    if (cancelled()) {
      return;
    }

    if (!base::FileUtilProxy::CreateOrOpen(
        task_runner(), file_path_,
        base::File::FLAG_OPEN | base::File::FLAG_READ,
        base::Bind(&GetFileContentsJobImpl::OnFileOpened, this))) {
      callback_.Run(base::File::FILE_ERROR_FAILED, NULL, -1);
    }
  }

  void OnFileOpened(base::File::Error error,
                    base::PassPlatformFile file,
                    bool created) {
    if (cancelled()) {
      return;
    }

    if (error != base::File::FILE_OK) {
      callback_.Run(error, NULL, -1);
      return;
    }

    file_ = file.ReleaseValue();
    if (!base::FileUtilProxy::GetFileInfoFromPlatformFile(
        task_runner(), file_,
        base::Bind(&GetFileContentsJobImpl::OnGotFileInfo, this))) {
      callback_.Run(base::File::FILE_ERROR_FAILED, NULL, -1);
    }
  }

  void OnGotFileInfo(base::File::Error error,
                     const base::File::Info& info) {
    if (cancelled()) {
      return;
    }

    if (error != base::File::FILE_OK) {
      callback_.Run(error, NULL, -1);
      return;
    }

    if (info.is_directory) {
      callback_.Run(base::File::FILE_ERROR_NOT_A_FILE, NULL, -1);
      return;
    }

    // XXX: info.size is int64, but Read() takes an int
    if (!base::FileUtilProxy::Read(
        task_runner(), file_, 0, info.size,
        base::Bind(&GetFileContentsJobImpl::OnGotData, this))) {
      callback_.Run(base::File::FILE_ERROR_FAILED, NULL, -1);
    }
  }

  void OnGotData(base::File::Error error,
                 const char* data,
                 int bytes_read) {
    if (cancelled()) {
      return;
    }

    if (error != base::File::FILE_OK) {
      callback_.Run(error, NULL, -1);
      return;
    }

    callback_.Run(base::File::FILE_OK, data, bytes_read);
  }

  static void OnFileClosed(base::File::Error error) {}

  base::FilePath file_path_;
  FileUtils::GetFileContentsCallback callback_;
  base::PlatformFile file_;
};

class GetFileContentsJob : public AsyncFileJob {
 public:
  GetFileContentsJob(base::TaskRunner* task_runner,
                     const base::FilePath& file_path,
                     const FileUtils::GetFileContentsCallback& callback) :
      AsyncFileJob(new GetFileContentsJobImpl(
        task_runner, file_path, callback)) {}
};

} // namespace

AsyncFileJob::~AsyncFileJob() {
  impl_->Cancel();
}

AsyncFileJob::AsyncFileJob(AsyncFileJobImpl* impl) :
    impl_(impl) {
  impl_->Run();
}

// static
AsyncFileJob* FileUtils::GetFileContents(base::TaskRunner* task_runner,
                                const base::FilePath& file_path,
                                const GetFileContentsCallback& callback) {
  if (callback.is_null()) {
    return NULL;
  }

  return new GetFileContentsJob(task_runner, file_path, callback);
}

} // namespace oxide
